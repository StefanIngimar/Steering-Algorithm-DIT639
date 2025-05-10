#pragma once

class ObjectDetector {
public:
    virtual std::vector<cv::Rect> detect(const cv::Mat& frame) const = 0;
};