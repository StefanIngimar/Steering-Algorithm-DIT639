#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

class MlModelRuntime {
public:
    virtual ~MlModelRuntime() = default;
    virtual void load() = 0;
    virtual std::vector<cv::Mat> predict(const cv::Mat& image) = 0;

    virtual int get_trained_frame_width() const = 0;
    virtual int get_trained_frame_height() const = 0;
};
