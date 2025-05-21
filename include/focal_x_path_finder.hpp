#pragma once

#include "object_detector.hpp"
#include "path_finder.hpp"
#include "config.hpp"

class FocalXPathFinder : public PathFinder {
 public:
  FocalXPathFinder(Config& config);
  ~FocalXPathFinder() = default;

  cv::Point2f find_midpoint(const ColorClassifiedCones& detection_result,
                            const cv::Mat& frame) override;
  float calculate_steering_angle(const cv::Point2f& midpoint,
                                 const cv::Mat& frame) override;

 private:
  uint32_t m_frameWidth;
  uint32_t m_frameHeight;
  bool m_is_verbose;
  float m_high_col_idx;
  float m_high_row_idx; // zero-indexed rows (top-down)
  float m_focal_x;
  float m_y_penalty; 
  bool m_side_mapping_known;
  std::string m_last_known_left_color;
  std::string m_last_known_right_color;
  const float MAX_Y_DIFF{25.0f};  
  // known average lane width is 338px 
  const float MIN_LANE_WIDTH{200.0f};
  const float MAX_LANE_WIDTH{450.0f}; 
  // Raspberry Pi Module 2 camera:
  const float FOC_LEN{3.04f};
  const float SENSOR_WIDTH{3.68f}; 

  std::vector<cv::Point2f> get_btm_centers(
      const std::vector<cv::Rect>& objects);
};
