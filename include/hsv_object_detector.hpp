#pragma once

#include "object_detector.hpp"

class HsvObjectDetector : public ObjectDetector {
public:
    HsvObjectDetector();
    ~HsvObjectDetector() = default;

    ColorClassifiedCones detect(const cv::Mat& frame) const override;
private:
};


