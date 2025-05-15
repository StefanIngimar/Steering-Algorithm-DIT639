#pragma once

#include "ml_model_runtime.hpp"
#include "object_detector.hpp"

class MlObjectDetector : public ObjectDetector {
public:
    // Describes the data layout of the model trained to detect and classify cones based on their colors
    enum class DetectionAttribute {
        CenterX = 0,
        CenterY = 1,
        Width = 2,
        Height = 3,
        Objectness = 4,
        BlueConeProbability = 5,
        YellowConeProbability = 6,
        RedConeProbability = 7,
    };

    MlObjectDetector(std::shared_ptr<MlModelRuntime> model_runtime, float confidence_threshold = 0.5f, float nms_threshold = 0.4f);
    ~MlObjectDetector() = default;

    ColorClassifiedCones detect(const cv::Mat& frame) const override;
private:
    struct DetectionResult {
        std::vector<cv::Rect> boxes;
        std::vector<float> confidences;
        std::vector<int> indices;
        std::vector<int> class_ids;
        std::vector<std::vector<float>> class_probabilties;
    };

    float m_confidence_threshold;
    float m_nms_threshold;

    std::shared_ptr<MlModelRuntime> m_model_runtime;

    ColorClassifiedCones classify_detected_cones(const DetectionResult& detected_objects) const;
    void annotate_frame_with_detected_objects(const cv::Mat& frame, const DetectionResult& detected_objects) const;
    std::string class_id_to_string(const int class_id) const;
};
