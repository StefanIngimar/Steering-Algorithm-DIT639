#include "hsv_object_detector.hpp"

#include <stdexcept>

HsvObjectDetector::HsvObjectDetector(Config& config)
  : M_LOWER_BOUND((config.height - 1.0f) * 1.0f / 2.0f),
    M_UPPER_BOUND((config.height - 1.0f) * 3.0f / 4.0f) {
  m_is_verbose = config.is_verbose;
}

ColorClassifiedCones HsvObjectDetector::detect(const cv::Mat& frame) const {
  cv::Mat hsv_frame, yellow_mask, blue_mask, combined_mask;

  // Stage (1) === Preprocessing ===

  // ImageProcessor class performs BGRA2BGR conversion

  // Create CLAHE object for contrast enhancement
  cv::Ptr<cv::CLAHE> clahe{cv::createCLAHE(2.0, cv::Size(8, 8))};

  // Contrast adjustment: appply CLAHE on the Y channel
  cv::Mat ycrcb;
  cv::cvtColor(frame, ycrcb, cv::COLOR_BGR2YCrCb);
  std::vector<cv::Mat> channels;
  cv::split(ycrcb, channels);
  clahe->apply(channels[0], channels[0]);
  cv::merge(channels, ycrcb);
  cv::Mat contrast_enhanced;
  cv::cvtColor(ycrcb, contrast_enhanced, cv::COLOR_YCrCb2BGR);

  // Region of interest (ROI) mask
  cv::Mat roi_mask{cv::Mat::zeros(contrast_enhanced.size(), contrast_enhanced.type())};
  // Define ROI polygon
  std::vector<cv::Point> roi_polygon{
      cv::Point(0, 380),    // Bottom left
      cv::Point(640, 380),  // Bottom right
      cv::Point(640, 200),  // Top right
      cv::Point(0, 200)     // Top left
  };
  // Fill polygon white on black mask
  std::vector<std::vector<cv::Point>> polygons{roi_polygon};
  cv::fillPoly(roi_mask, polygons, cv::Scalar(255, 255, 255));

  if (m_is_verbose) {
    // Draw source points for visualization
    for (uint16_t i = 0; i < 4; i++) {
        cv::circle(frame, roi_polygon[i], 5, cv::Scalar(0, 255, 0), cv::FILLED);
    }
  }

  // Apply mask to extract ROI
  cv::Mat roi_frame;
  cv::bitwise_and(contrast_enhanced, roi_mask, roi_frame);

  // Convert frame color model
  cv::cvtColor(roi_frame, hsv_frame, cv::COLOR_BGR2HSV);

  // HSV color segmentation

  // Apply threshold values to get binary mask
  cv::inRange(hsv_frame, cv::Scalar(15, 80, 30), cv::Scalar(40, 255, 255),
              yellow_mask);
  cv::inRange(hsv_frame, cv::Scalar(15, 120, 30), cv::Scalar(160, 255, 255),
              blue_mask);

  // Morphological operations (to clean up the mask)
  // Kernel sizes: 3x3, 5x5, and 7x7
  // Use larger kernel for more noisy frames
  // Use smaller kernel for clean frames
  cv::Mat kernel{cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5))};
  cv::morphologyEx(yellow_mask, yellow_mask, cv::MORPH_OPEN, kernel);
  cv::morphologyEx(yellow_mask, yellow_mask, cv::MORPH_CLOSE, kernel);
  cv::morphologyEx(blue_mask, blue_mask, cv::MORPH_OPEN, kernel);
  cv::morphologyEx(blue_mask, blue_mask, cv::MORPH_CLOSE, kernel);

  // Combine both masks for contour detection
  cv::bitwise_or(yellow_mask, blue_mask, combined_mask);

  // Stage (2) === Detection  ===

  // Contour detection and labeling on original frame

  // Find potential cones
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(combined_mask, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);

  std::vector<cv::Rect> blue_boxes;
  std::vector<cv::Rect> yellow_boxes;

  // Filter contours based on area
  // Classify contours based on color
  // Draw bounding boxes and add labels
  for (const auto& contour : contours) {
    float obj_area{static_cast<float>(cv::contourArea(contour))};
    // Filter out very small or very large objects. Adjust these values as
    // needed
    if (obj_area < M_SMALLEST_CONE || obj_area > M_BIGGEST_CONE) continue;

    // Bounding box
    cv::Rect b_box{cv::boundingRect(contour)};
    cv::Point2f cone_btm_center{b_box.x + b_box.width / 2.0f, 
                                static_cast<float>(b_box.y + b_box.height)};

    // Get amount of yellow and blue within bounding box
    int32_t yellow_amount{cv::countNonZero(yellow_mask(b_box))};
    int32_t blue_amount{cv::countNonZero(blue_mask(b_box))};

    std::string label;    // bounding box label
    cv::Scalar bb_color;  // bounding box color

    // Focus ahead: consider cones just below bottom half of frame but not
    // too close to the vehicle. Adjust range as needed
    if ((cone_btm_center.y > M_LOWER_BOUND) && (cone_btm_center.y < M_UPPER_BOUND)) {
      // Contour classification based on color
      if (yellow_amount > blue_amount) {
        label = "yellow";
        bb_color = cv::Scalar(0, 255, 255);  // Yellow
        yellow_boxes.push_back(b_box);
      } else if (blue_amount > yellow_amount) {
        label = "blue";
        bb_color = cv::Scalar(255, 0, 0);  // Blue
        blue_boxes.push_back(b_box);
      } else {
        continue;  // uncertain, skip
      }
    }

    if (m_is_verbose) {
      // Contour visualization: bounding boxes and classification labels
      cv::rectangle(frame, b_box, bb_color, 2);
      cv::circle(frame, cone_btm_center, 4, bb_color, -1);
      cv::putText(frame, label, b_box.tl() - cv::Point(0, 10),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, bb_color, 2);
    }
  }

  return {blue_boxes, yellow_boxes};
}
