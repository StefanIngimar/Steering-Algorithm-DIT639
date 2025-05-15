#pragma once

#include "object_detector.hpp"
#include "path_finder.hpp"

class AverageXPathFinder : public PathFinder {
public:
    cv::Point2f find_midpoint(const ColorClassifiedCones& detection_result, const cv::Mat& frame) override;
    float calculate_steering_angle(const cv::Point2f& midpoint, const cv::Mat& frame) override;

private:
    const float m_max_steering_angle = 0.3f;
    const float m_sensitivity = 0.5f;

    void sort_by_y_desc(std::vector<cv::Rect>& objects);
    void filter_out_objects_outside_threshold(std::vector<cv::Rect>& objects, const cv::Mat& frame);
    void find_closest_objects(const std::vector<cv::Rect>& all_objects, std::vector<cv::Rect>& closest_objects, int to_find = 1);
    void find_object_centers(const std::vector<cv::Rect>& all_objects, std::vector<cv::Point2f>& object_centerse);
};
