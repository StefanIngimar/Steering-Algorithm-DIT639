#pragma once

#include <opencv2/opencv.hpp>

class PathFinder {
public:
    virtual ~PathFinder() = default;

    virtual cv::Point2f find_midpoint(const ColorClassifiedCones& detection_result, const cv::Mat& frame) = 0;
    virtual float calculate_steering_angle(const cv::Point2f& midpoint, const cv::Mat& frame) = 0;
};
