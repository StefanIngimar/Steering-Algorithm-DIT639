#include <stdexcept>

#include "ml_object_detector.hpp"
#include "object_detector.hpp"

MlObjectDetector::MlObjectDetector(std::shared_ptr<MlModelRuntime> model_runtime, const float confidence_threshold, const float nms_threshold)
    : m_confidence_threshold(confidence_threshold), m_nms_threshold(nms_threshold), m_model_runtime(model_runtime) {
    if (!m_model_runtime) {
        throw std::invalid_argument("[ObjectDetector] Model pointer cannot be null");
    }
    m_model_runtime->load();
}

ColorClassifiedCones MlObjectDetector::detect(const cv::Mat& frame) const {
    const std::vector<cv::Mat> outputs = m_model_runtime->predict(frame);

    const cv::Mat& output = outputs[0];
    const int rows = output.size[1];
    const int cols = output.size[2];
    const auto* data = reinterpret_cast<float*>(output.data);

    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;
    std::vector<int> class_ids;
    std::vector<std::vector<float>> class_probabilities;

    for (int i = 0; i < rows; i++) {
        float objectness = data[static_cast<int>(DetectionAttribute::Objectness)];

        std::vector<float> probabilities = {
            data[static_cast<int>(DetectionAttribute::BlueConeProbability)],
            data[static_cast<int>(DetectionAttribute::YellowConeProbability)],
            data[static_cast<int>(DetectionAttribute::RedConeProbability)],
        };

        auto highest_probability_item = std::max_element(probabilities.begin(), probabilities.end());
        float highest_probability = *highest_probability_item;
        int class_id = std::distance(probabilities.begin(), highest_probability_item);

        float combined_confidence = objectness * highest_probability;
        if (combined_confidence >= m_confidence_threshold) {
            const float scale_x = static_cast<float>(frame.cols) / m_model_runtime->get_trained_frame_width();
            const float scale_y = static_cast<float>(frame.rows) / m_model_runtime->get_trained_frame_height();

            const float x = data[static_cast<int>(DetectionAttribute::CenterX)] * scale_x;
            const float y = data[static_cast<int>(DetectionAttribute::CenterY)] * scale_y;
            const float w = data[static_cast<int>(DetectionAttribute::Width)] * scale_x;
            const float h = data[static_cast<int>(DetectionAttribute::Height)] * scale_y;

            const int left = static_cast<int>(x - w / 2);
            const int top = static_cast<int>(y - h / 2);

            boxes.emplace_back(left, top, static_cast<int>(w), static_cast<int>(h));
            confidences.emplace_back(combined_confidence);
            class_ids.emplace_back(class_id);
            class_probabilities.emplace_back(probabilities);
        }

        data += cols;
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, m_confidence_threshold, m_nms_threshold, indices);
    
    const auto detection_result = DetectionResult{
        boxes,
        confidences,
        indices,
        class_ids,
        class_probabilities
    };

    annotate_frame_with_detected_objects(frame, detection_result);

    auto color_classified_cones = classify_detected_cones(detection_result);

    return color_classified_cones;
}

void MlObjectDetector::annotate_frame_with_detected_objects(const cv::Mat& frame, const DetectionResult& detected_objects) const {
    for (const int index : detected_objects.indices) {
        const auto& box = detected_objects.boxes[index];
        const auto class_id = detected_objects.class_ids[index];

        std::string class_name = class_id_to_string(class_id);

        int text_x = box.x;
        int text_y = box.y - 5;

        cv::rectangle(frame, box, cv::Scalar(0, 255, 255), 1);

        cv::putText(
            frame, 
            class_name, 
            cv::Point(text_x, text_y), 
            cv::FONT_HERSHEY_PLAIN, 
            1.0, 
            cv::Scalar(255, 255, 255), 
            1
        );
    }
}

ColorClassifiedCones MlObjectDetector::classify_detected_cones(const DetectionResult& detected_objects) const {
    std::vector<cv::Rect> blue_boxes;
    std::vector<cv::Rect> yellow_boxes;

    for (const int index : detected_objects.indices) {
        const auto& box = detected_objects.boxes[index];
        const int class_id = detected_objects.class_ids[index];

        if (class_id == 0) {
            blue_boxes.emplace_back(box);
        } else if (class_id == 1) {
            yellow_boxes.emplace_back(box);
        }
    }

    return {blue_boxes, yellow_boxes};
}

std::string MlObjectDetector::class_id_to_string(const int class_id) const {
    std::string class_name;
    switch (class_id) {
        case 0: class_name = "Blue"; break;
        case 1: class_name = "Yellow"; break;
        case 2: class_name = "Red"; break;
        default: class_name = "Unknown"; break;
    }

    return class_name;
}
