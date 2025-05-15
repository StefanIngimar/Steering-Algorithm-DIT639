#pragma once

#include "object_detector.hpp"

class HsvObjectDetector : public ObjectDetector {
public:
    // public members: constructors, getters, setters
    HsvObjectDetector();
    ~HsvObjectDetector() = default;

    ColorClassifiedCones detect(const cv::Mat& frame) const override;
private:
    // private members
};


