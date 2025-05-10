#include <stdexcept>

#include "ml_object_detector.hpp"

MlObjectDetector::MlObjectDetector(std::shared_ptr<MlModel> model, const float confidence_threshold, const float nms_threshold)
    : m_confidence_threshold(confidence_threshold), m_nms_threshold(nms_threshold), m_model(model) {
    if (!m_model) {
        throw std::invalid_argument("[ObjectDetector] Model pointer cannot be null");
    }
    m_model->load();
}

std::vector<cv::Rect> MlObjectDetector::detect(const cv::Mat& frame) const {
    const std::vector<cv::Mat> outputs = m_model->predict(frame);

    const cv::Mat& output = outputs[0];
    const int rows = output.size[1];
    const int cols = output.size[2];
    const auto* data = reinterpret_cast<float*>(output.data);

    // Those variables will be used for the Non-Maximum Suppression (NMS) to remove all the overlapping
    // boxes/rectangles for the same object.
    std::vector<cv::Rect> boxes;
    std::vector<float> confidences;

    for (int i = 0; i < rows; i++) {
        float objectness = data[static_cast<int>(DetectionAttribute::Objectness)];
        float confidence = data[static_cast<int>(DetectionAttribute::Confidence)];
        float combined_confidence = objectness * confidence;

        if (combined_confidence >= m_confidence_threshold) {
            const float scale_x = static_cast<float>(frame.cols) / m_model->get_trained_frame_width();
            const float scale_y = static_cast<float>(frame.rows) / m_model->get_trained_frame_height();

            const float x = data[static_cast<int>(DetectionAttribute::CenterX)] * scale_x;
            const float y = data[static_cast<int>(DetectionAttribute::CenterY)] * scale_y;
            const float w = data[static_cast<int>(DetectionAttribute::Width)] * scale_x;
            const float h = data[static_cast<int>(DetectionAttribute::Height)] * scale_y;

            const int left = static_cast<int>(x - w / 2);
            const int top = static_cast<int>(y - h / 2);

            boxes.emplace_back(left, top, static_cast<int>(w), static_cast<int>(h));
            confidences.emplace_back(confidence);
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
