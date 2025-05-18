#pragma once

#include "object_detector.hpp"
#include "path_finder.hpp"

class AverageXPathFinder : public PathFinder {
 public:
  AverageXPathFinder();

  cv::Point2f find_midpoint(const ColorClassifiedCones& detection_result,
                            const cv::Mat& frame) override;
  float calculate_steering_angle(const cv::Point2f& midpoint,
                                 const cv::Mat& frame) override;

 private:
  const float m_max_steering_angle = 0.3f;
  const float m_sensitivity = 0.5f;

  uint32_t m_road_width_measurements;
  float m_average_road_width;

  std::vector<cv::Point2f> get_closest_centers(
      const std::vector<cv::Rect>& objects, int to_find = 1);
  void sort_and_filter(std::vector<cv::Rect>& objects, const cv::Mat& frame);
  void update_road_width_running_average(const float road_width);
};
