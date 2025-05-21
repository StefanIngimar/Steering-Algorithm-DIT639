  
#include "focal_x_path_finder.hpp"
#include "logger.hpp"
#include "config.hpp"

FocalXPathFinder::FocalXPathFinder(Config& config) {
  m_frameWidth = config.width;
  m_frameHeight = config.height;
  m_is_verbose = config.is_verbose;
  m_focal_x = FOC_LEN / SENSOR_WIDTH * static_cast<float>(m_frameWidth);
  m_high_row_idx = static_cast<float>(m_frameHeight - 1);
  m_high_col_idx = static_cast<float>(m_frameWidth - 1);
  m_y_penalty = m_focal_x / m_high_row_idx;
  m_side_mapping_known = false;
};

cv::Point2f FocalXPathFinder::find_midpoint(
    const ColorClassifiedCones& detection_result, const cv::Mat& frame) {
  std::vector<cv::Rect> blue_cones = detection_result.blue_cones;
  std::vector<cv::Rect> yellow_cones = detection_result.yellow_cones;
  cv::Point2f midpoint{0};
  bool found_cone{false};
  bool found_pair{false};
  cv::Point2f left_cone{0};
  cv::Point2f right_cone{0};

  if (blue_cones.empty() && yellow_cones.empty())
    found_cone = false;
  else 
    found_cone = true;

  auto blue_btm_centers{get_btm_centers(blue_cones)};
  auto yellow_btm_centers{get_btm_centers(yellow_cones)};

  float best_score{1e9}; // init. with a large enough value 
  cv::Point2f best_blue, best_yellow;

  // Find best blue-yellow pair
  for (const auto& blue_cone : blue_btm_centers) {
    for (const auto& yellow_cone : yellow_btm_centers) {
      float y_diff{blue_cone.y - yellow_cone.y};
      // Filter cone pairs that are not well aligned vertically
      if (std::abs(y_diff) > MAX_Y_DIFF) 
        continue; 

      float x_diff{blue_cone.x - yellow_cone.x};
      // Filter cone pairs that are too close or too far apart horizontally
      if (std::abs(x_diff) < MIN_LANE_WIDTH || std::abs(x_diff) > MAX_LANE_WIDTH) 
        continue;
      
      // Single weighted score: add a penalty weight to the normal distance based on 
      // how vertically misaligned the cones are (lowest score preferred)
      float score = (x_diff * x_diff + y_diff * y_diff) + m_y_penalty * std::abs(y_diff);
      
      if (score < best_score) {
        best_score = score;
        best_blue = blue_cone;
        best_yellow = yellow_cone;
        found_pair = true;
      }
    }
  }

  if (found_pair) {
    // Determine which cone is on the left and which is on the right
    if (best_blue.x < best_yellow.x) {
      left_cone = best_blue;
      right_cone = best_yellow;
      m_last_known_left_color = "blue";
      m_last_known_right_color = "yellow";
      m_side_mapping_known = true;
    } 
    else {
      left_cone = best_yellow;
      right_cone = best_blue;
      m_last_known_left_color = "yellow";
      m_last_known_right_color = "blue";
      m_side_mapping_known = true;
    }
    
    midpoint = (left_cone + right_cone) / 2;
  }    
  else if (found_cone && m_side_mapping_known) {
    // Compute virtual midpoint if only one cone is visible in frame
    cv::Point2f actual_cone;
    std::string cone_color;

    if (blue_cones.empty()) {
      cone_color = "yellow";
      actual_cone = yellow_btm_centers.front();
    }
    else {
      cone_color = "blue";
      actual_cone = blue_btm_centers.front();
    }

    // Create virtual cone 
    // If the setup of the track changes in the same program instance, 
    // side mapping will auto-fix on the next detected cone pair
    cv::Point2f virtual_cone;
    if (cone_color == m_last_known_left_color) {
      left_cone = actual_cone;
      float tmp_x{actual_cone.x + MAX_LANE_WIDTH};
      float v_x{(tmp_x) > m_high_col_idx ? m_high_col_idx : tmp_x};
      virtual_cone = cv::Point2f(v_x, actual_cone.y); // mirror to right
      right_cone = virtual_cone;
    }
    else if (cone_color == m_last_known_right_color) {
      right_cone = actual_cone;
      float tmp_x{actual_cone.x - MAX_LANE_WIDTH};
      float v_x{(tmp_x) < 0 ? 0 : tmp_x};
      virtual_cone = cv::Point2f(v_x, actual_cone.y); // mirror to left
      left_cone = virtual_cone;
    }

    midpoint = (left_cone + right_cone) / 2;
  }
  else {
    midpoint = cv::Point2f(m_high_col_idx / 2, m_high_row_idx); // fallback straight
  }

  if (m_is_verbose && found_cone && m_side_mapping_known) {
    cv::Point2f frame_bottom_mid{m_high_col_idx / 2.0f, m_high_row_idx};
    cv::circle(frame, midpoint, 5, cv::Scalar(0, 0, 255), cv::FILLED); // red circle
    cv::line(frame, left_cone, right_cone, cv::Scalar(0, 0, 255), 2); // red line 
    cv::line(frame, frame_bottom_mid, midpoint, cv::Scalar(0, 255, 0), 2); // green line
  }

  return midpoint;
}

float FocalXPathFinder::calculate_steering_angle(const cv::Point2f& midpoint,
                                                   const cv::Mat& frame) {
  if (midpoint.y >= m_high_row_idx || frame.cols == 0) {
    return 0.0f;
  }

  // image center x, zero-indexed coordinates
  const float IMG_CENTER_X = m_high_col_idx / 2.0f; 

  // Offset of midpoint from image center
  float x_offset{midpoint.x - IMG_CENTER_X};

  // Compute the angle between the camera's optical axis and the midpoint
  // NEGATE x_offset to match vehicle convention
  float offset_angle_rad = std::atan2(-x_offset, m_focal_x);

  // Angle correction 
  // K is the calibration of gain (tune as needed)
  const float K{0.4468f};
  float corrected_angle_rad = K * offset_angle_rad;

  // Cap the steering angle to prevent erratic movement:
  const float MAX_ANGLE{0.3f};
  corrected_angle_rad = std::clamp(corrected_angle_rad, -MAX_ANGLE, MAX_ANGLE);  

  return corrected_angle_rad;
}
 
/**
 * Get bottom center of detected objects in relation to the car (bottom of the
 * frame).
 */
std::vector<cv::Point2f> FocalXPathFinder::get_btm_centers(
    const std::vector<cv::Rect>& objects) {
  std::vector<cv::Point2f> btm_centers;
 
  for (const auto& rec_box : objects) {
    cv::Point2f cone_btm_center{rec_box.x + rec_box.width / 2.0f, 
                              static_cast<float>(rec_box.y + rec_box.height)};
    btm_centers.push_back(cone_btm_center);
  }

  return btm_centers;
}
