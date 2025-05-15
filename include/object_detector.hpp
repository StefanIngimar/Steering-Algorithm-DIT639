#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

struct ColorClassifiedCones {
    std::vector<cv::Rect> blue_cones;
    std::vector<cv::Rect> yellow_cones;
};

class ObjectDetector {
public:
    virtual ~ObjectDetector() = default;

    virtual ColorClassifiedCones detect(const cv::Mat& frame) const = 0;
};
