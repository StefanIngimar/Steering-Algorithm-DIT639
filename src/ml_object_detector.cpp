#include <stdexcept>

#include "ml_object_detector.hpp"

MlObjectDetector::MlObjectDetector(std::shared_ptr<MlModelRuntime> model_runtime, const float confidence_threshold, const float nms_threshold)
    : m_confidence_threshold(confidence_threshold), m_nms_threshold(nms_threshold), m_model_runtime(model_runtime) {
    if (!m_model_runtime) {
        throw std::invalid_argument("[ObjectDetector] Model pointer cannot be null");
    }
    m_model_runtime->load();
}

std::vector<cv::Rect> MlObjectDetector::detect(const cv::Mat& frame) const {
    const std::vector<cv::Mat> outputs = m_model_runtime->predict(frame);

    const cv::Mat& output = outputs[0];
    const int rows = output.size[1];
    const int cols = output.size[2];
    const auto* data = reinterpret_cast<float*>(output.data);

    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;

    for (int i = 0; i < rows; i++) {
        float objectness = data[static_cast<int>(DetectionAttribute::Objectness)];

        std::vector<float> probabilities = {
            data[static_cast<int>(DetectionAttribute::BlueConeProbability)],
            data[static_cast<int>(DetectionAttribute::YellowConeProbability)],
            data[static_cast<int>(DetectionAttribute::RedConeProbability)],
        };

        auto highest_probability_item = std::max_element(probabilities.begin(), probabilities.end());
        float highest_probability = *highest_probability_item;

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
        }

        data += cols;
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, m_confidence_threshold, m_nms_threshold, indices);

    std::vector<cv::Rect> detected_objects;
    for (const int index: indices) {
        detected_objects.push_back(boxes[index]);
    }

    return detected_objects;
}
