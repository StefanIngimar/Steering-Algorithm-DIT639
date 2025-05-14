#pragma once

#include "path_finder.hpp"

class AverageXPathFinder : public PathFinder {
public:
    cv::Point2f find_midpoint(const std::vector<cv::Rect> detection_result, const cv::Mat& frame) override;
    float calculate_steering_angle(const cv::Point2f& midpoint, const cv::Mat& frame) override;

private:
    const float m_max_steering_angle = 0.3f;
    const float m_sensitivity = 0.5f;

    void separate_detected_objects(
        const cv::Mat& frame, const std::vector<cv::Rect>& objects, std::vector<cv::Point2f>& left, std::vector<cv::Point2f>& right
    );
    void sort_by_y_desc(std::vector<cv::Point2f>& objects);
    void find_closest_objects(const std::vector<cv::Point2f>& all_objects, std::vector<cv::Point2f>& closest_objects, int to_find = 1);
};
