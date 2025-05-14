#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

class ObjectDetector {
public:
    virtual std::vector<cv::Rect> detect(const cv::Mat& frame) const = 0;
    virtual ~ObjectDetector() = default;
};
