#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class MlModelRuntime {
public:
    virtual ~MlModelRuntime() = default;
    virtual void load() = 0;
    virtual std::vector<cv::Mat> predict(const cv::Mat& image) const = 0;
};