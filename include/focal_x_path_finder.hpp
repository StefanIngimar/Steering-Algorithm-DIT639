#pragma once

#include "object_detector.hpp"
#include "path_finder.hpp"

class FocalXPathFinder : public PathFinder {
 public:
  FocalXPathFinder();
  ~FocalXPathFinder() = default;

  cv::Point2f find_midpoint(const ColorClassifiedCones& detection_result,
                            const cv::Mat& frame) override;
  float calculate_steering_angle(const cv::Point2f& midpoint,
                                 const cv::Mat& frame) override;

 private:
  // zero-indexed rows (top-down)
  float m_high_col_idx;
  float m_high_row_idx; 
  float m_focal_x;
  bool m_side_mapping_known;
  std::string m_left_color;
  std::string m_right_color;

  std::vector<cv::Point2f> get_btm_centers(
      const std::vector<cv::Rect>& objects);
};
