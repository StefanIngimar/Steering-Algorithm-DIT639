#pragma once

#include "object_detector.hpp"
#include "config.hpp"

class HsvObjectDetector : public ObjectDetector {
public:
  HsvObjectDetector(Config& config);
  ~HsvObjectDetector() = default;

    ColorClassifiedCones detect(const cv::Mat& frame) const override;
private:
  bool m_is_verbose;
  // Vertical range of interest for cones
  const float M_LOWER_BOUND;
  const float M_UPPER_BOUND;
  // Cone area thresholds
  static constexpr float M_SMALLEST_CONE{50.0f};
  static constexpr float M_BIGGEST_CONE{400.0f};
};
