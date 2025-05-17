#pragma once

#include <memory>

#include "config.hpp"
#include "ground_steering_message_handler.hpp"
#include "object_detector.hpp"
#include "path_finder.hpp"

class ImageProcessor {
 public:
  ImageProcessor(const Config &config, std::shared_ptr<cluon::OD4Session> od4,
                 std::unique_ptr<ObjectDetector> detector,
                 std::unique_ptr<PathFinder> path_finder,
                 std::shared_ptr<GroundSteeringMessageHandler> gs_handler);

  void run();

 private:
  static constexpr int M_FONT = cv::FONT_HERSHEY_PLAIN;
  static constexpr double M_FONT_SCALE = 0.8;
  static inline const cv::Scalar M_TEXT_COLOR{255, 255, 255};
  static constexpr int M_TEXT_THICKNESS = 1;
  static constexpr int M_LINE_HEIGHT = 20;
  static constexpr int M_BASE_Y = 20;

  const Config &m_config;
  std::shared_ptr<cluon::OD4Session> m_od4;
  std::unique_ptr<ObjectDetector> m_detector;

  std::unique_ptr<cluon::SharedMemory> m_shared_memory;
  std::unique_ptr<PathFinder> m_path_finder;
  std::shared_ptr<GroundSteeringMessageHandler> m_gs_handler;

  uint64_t m_processed_frames;
  uint64_t m_correctly_calculated_steering_angle;

  void process_frame();
  void log_steering(int64_t timestamp, float predicted, float actual);
  void annotate_image(cv::Mat &image, int sample_time_point,
                      float actual_steering, float steering_angle) const;
};
