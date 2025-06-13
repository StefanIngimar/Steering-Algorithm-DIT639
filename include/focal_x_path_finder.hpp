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
  bool m_is_verbose;
  bool m_side_mapping_known;
  std::string m_last_known_left_color;
  std::string m_last_known_right_color;
  const float M_HIGH_ROW_IDX; // zero-indexed rows (top-down)
  const float M_HIGH_COL_IDX;
  const float M_FOCAL_X;
  const float M_Y_PENALTY; 
  const float M_IMG_CENTER_X; 
  static constexpr float M_MAX_Y_DIFF{25.0f};  
  // known average lane width is 338px 
  static constexpr float M_MIN_LANE_WIDTH{200.0f};
  static constexpr float M_MAX_LANE_WIDTH{450.0f}; 
  // Raspberry Pi Module 2 camera:
  static constexpr float M_FOC_LEN{3.04f};
  static constexpr float M_SENSOR_WIDTH{3.68f}; 
  // Calibration of gain (correction factor)
  static constexpr float M_K{0.4468f};
  static constexpr float M_MAX_ANGLE{0.3f};

  std::vector<cv::Point2f> get_btm_centers(
      const std::vector<cv::Rect>& objects);
};
