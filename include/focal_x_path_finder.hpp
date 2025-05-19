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
  float highColIdx{639.0f};
  float highRowIdx{479.0f}; 
  float focalX{};
  bool sideMappingKnown{false};
  std::string leftColor{};
  std::string rightColor{};

  std::vector<cv::Point2f> getBtmCenters(
      const std::vector<cv::Rect>& objects);
};
