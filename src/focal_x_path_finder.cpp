  
#include "focal_x_path_finder.hpp"

#include "logger.hpp"

FocalXPathFinder::FocalXPathFinder() {};

cv::Point2f FocalXPathFinder::find_midpoint(
    const ColorClassifiedCones& detectionResult, const cv::Mat& frame) {
  std::vector<cv::Rect> blueCones = detectionResult.blue_cones;
  std::vector<cv::Rect> yellowCones = detectionResult.yellow_cones;
  cv::Point2f midpoint{};
  bool foundCone{false};
  bool foundPair{false};
  cv::Point2f leftCone{};
  cv::Point2f rightCone{};

  highRowIdx = static_cast<float>(frame.rows - 1);
  highColIdx = static_cast<float>(frame.cols - 1);

  if (blueCones.empty() && yellowCones.empty())
    foundCone = false;
  else 
    foundCone = true;

  auto blueBtmCenters{getBtmCenters(blueCones)};
  auto yellowBtmCenters{getBtmCenters(yellowCones)};

  const float MAX_Y_DIFF{25.0f};  
  // known average lane width is 338px 
  const float MIN_LANE_WIDTH{200.0f};
  const float MAX_LANE_WIDTH{450.0f}; 
  // Raspberry Pi Module 2 camera:
  const float FOC_LEN{3.04f};
  const float SENSOR_WIDTH{3.68f}; 
  focalX = FOC_LEN / SENSOR_WIDTH * static_cast<float>(frame.cols);
  const float Y_PENALTY{focalX/ highRowIdx}; 
  float bestScore{1e9}; // init. with a large enough value 
  cv::Point2f bestBlue, bestYellow;

  // Find best blue-yellow pair
  for (const auto& blueCone : blueBtmCenters) {
    for (const auto& yellowCone : yellowBtmCenters) {
      float yDiff{blueCone.y - yellowCone.y};
      // Filter cone pairs that are not well aligned vertically
      if (std::abs(yDiff) > MAX_Y_DIFF) 
        continue; 

      float xDiff{blueCone.x - yellowCone.x};
      // Filter cone pairs that are too close or too far apart horizontally
      if (std::abs(xDiff) < MIN_LANE_WIDTH || std::abs(xDiff) > MAX_LANE_WIDTH) 
        continue;
      
      // Single weighted score: add a penalty weight to the normal distance based on 
      // how vertically misaligned the cones are (lowest score preferred)
      float score = (xDiff * xDiff + yDiff * yDiff) + Y_PENALTY * std::abs(yDiff);
      
      if (score < bestScore) {
        bestScore = score;
        bestBlue = blueCone;
        bestYellow = yellowCone;
        foundPair = true;
      }
    }
  }

  if (foundPair) {
    // Determine which cone is on the left and which is on the right
    if (bestBlue.x < bestYellow.x) {
      leftCone = bestBlue;
      rightCone = bestYellow;
      leftColor = "blue";
      rightColor = "yellow";
      sideMappingKnown = true;
    } 
    else {
      leftCone = bestYellow;
      rightCone = bestBlue;
      leftColor = "yellow";
      rightColor = "blue";
      sideMappingKnown = true;
    }
    
    midpoint = (leftCone + rightCone) / 2;
  }    
  else if (foundCone && sideMappingKnown) {
    // Compute virtual midpoint if only one cone is visible in frame
    cv::Point2f actualCone;
    std::string coneColor;

    if (blueCones.empty()) {
      coneColor = "yellow";
      actualCone = yellowBtmCenters.front();
    }
    else {
      coneColor = "blue";
      actualCone = blueBtmCenters.front();
    }

    // Create virtual cone 
    cv::Point2f virtualCone;
    if (coneColor == leftColor) {
      leftCone = actualCone;
      float tmpX{actualCone.x + MAX_LANE_WIDTH};
      float vX{(tmpX) > highColIdx ? highColIdx : tmpX};
      virtualCone = cv::Point2f(vX, actualCone.y); // mirror to right
      rightCone = virtualCone;
    }
    else if (coneColor == rightColor) {
      rightCone = actualCone;
      float tmpX{actualCone.x - MAX_LANE_WIDTH};
      float vX{(tmpX) < 0 ? 0 : tmpX};
      virtualCone = cv::Point2f(vX, actualCone.y); // mirror to left
      leftCone = virtualCone;
    }

    midpoint = (leftCone + rightCone) / 2;
  }
  else {
    midpoint = cv::Point2f(highColIdx / 2, highRowIdx); // fallback straight
  }

  if (foundCone && sideMappingKnown) {
    cv::Point2f frameBottomMid{highColIdx / 2.0f, highRowIdx};
    cv::circle(frame, midpoint, 5, cv::Scalar(0, 0, 255), cv::FILLED); // red circle
    cv::line(frame, leftCone, rightCone, cv::Scalar(0, 0, 255), 2); // red line 
    cv::line(frame, frameBottomMid, midpoint, cv::Scalar(0, 255, 0), 2); // green line
  }

  return midpoint;
}

float FocalXPathFinder::calculate_steering_angle(const cv::Point2f& midpoint,
                                                   const cv::Mat& frame) {
  if (midpoint.y >= highRowIdx || frame.cols == 0) {
    return 0.0f;
  }

  // image center x, zero-indexed coordinates
  const float IMG_CENTER_X = highColIdx / 2.0f; 

  // Offset of midpoint from image center
  float xOffset{midpoint.x - IMG_CENTER_X};

  // Compute the angle between the camera's optical axis and the midpoint
  // NEGATE xOffset to match vehicle convention
  float offsetAngleRad = std::atan2(-xOffset, focalX);

  // Angle correction 
  // K is the calibration of gain (tune as needed)
  const float K{0.4468f};
  float correctedAngleRad = K * offsetAngleRad;

  // Cap the steering angle to prevent erratic movement:
  const float MAX_ANGLE{0.3f};
  correctedAngleRad = std::clamp(correctedAngleRad, -MAX_ANGLE, MAX_ANGLE);  

  return correctedAngleRad;
}
 
/**
 * Get bottom center of detected objects in relation to the car (bottom of the
 * frame).
 */
std::vector<cv::Point2f> FocalXPathFinder::getBtmCenters(
    const std::vector<cv::Rect>& objects) {
  std::vector<cv::Point2f> btmCenters;
 
  for (const auto& recBox : objects) {
    cv::Point2f coneBtmCenter{recBox.x + recBox.width / 2.0f, 
                              static_cast<float>(recBox.y + recBox.height)};
    btmCenters.emplace_back(coneBtmCenter);
  }

  return btmCenters;
}