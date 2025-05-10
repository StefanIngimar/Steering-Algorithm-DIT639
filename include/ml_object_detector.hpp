#pragma once

#include "ml_model.hpp"
#include "object_detector.hpp"

class MlObjectDetector : public ObjectDetector {
public:
    // NOTE(sw): this takes into account only a single class - in our case a cone detection.
    // The confidence describes how confident the model is about 'finding' the cone.
    enum class DetectionAttribute {
        CenterX = 0,
        CenterY = 1,
        Width = 2,
        Height = 3,
        Objectness = 4,
        Confidence = 5
    };

    MlObjectDetector(std::shared_ptr<MlModel> model, float confidence_threshold = 0.5f, float nms_threshold = 0.4f);
    ~MlObjectDetector() = default;

    std::vector<cv::Rect> detect(const cv::Mat& frame) const override;
private:
    float m_confidence_threshold;
    float m_nms_threshold;

    std::shared_ptr<MlModel> m_model;
};


