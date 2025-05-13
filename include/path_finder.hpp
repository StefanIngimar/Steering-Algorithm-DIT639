#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

class PathFinder {
public:
    virtual cv::Point2f find_midpoint(const std::vector<cv::Rect> detection_result, const cv::Mat& frame) = 0;
    virtual float calculate_steering_angle(const cv::Point2f& midpoint, const cv::Mat& frame) = 0;
};
