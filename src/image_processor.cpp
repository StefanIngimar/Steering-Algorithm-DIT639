#include <cerrno>
#include <cstring>
#include <exception>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>

#include "config.hpp"
#include "ground_steering_message_handler.hpp"
#include "image_processor.hpp"
#include "logger.hpp"
#include "object_detector.hpp"

ImageProcessor::ImageProcessor(
    const Config &config, std::shared_ptr<cluon::OD4Session> od4,
    std::unique_ptr<ObjectDetector> detector,
    std::unique_ptr<PathFinder> path_finder,
    std::shared_ptr<GroundSteeringMessageHandler> gs_handler)
    : m_config(config), m_od4(od4), m_detector(std::move(detector)),
      m_shared_memory(
          std::make_unique<cluon::SharedMemory>(config.shared_memory_name)),
      m_message_handlers(), m_path_finder(std::move(path_finder)),
      m_gs_handler(gs_handler) {

  auto logger = Logger::get_instance().get_logger();
  if (!m_detector) {
    logger->warn("[ImageProcessor] No object detector was provided - object "
                 "detection will be omitted");
  }
};

void ImageProcessor::add_message_handler(
    std::shared_ptr<MessageHandler> message_handler) {
  m_message_handlers.push_back(message_handler);

  auto casted =
      std::dynamic_pointer_cast<GroundSteeringMessageHandler>(message_handler);
  if (casted) {
    m_gs_handler = casted;
  }
}

void ImageProcessor::run() {
  auto logger = Logger::get_instance().get_logger();

  logger->info("ImageProcessor: Setting up message handlers");
  setup_message_handlers();

  logger->info("ImageProcessor: Starting image processing");
  while (m_od4->isRunning()) {
    try {
      process_frame();
    } catch (const std::exception &e) {
      logger->error("ImageProcessor: Raised exception: {}", e.what());
      throw;
    } catch (...) {
      logger->error("ImageProcessor: Raised unknown exception");
      throw;
    }
  }

  logger->info("ImageProcessor: Closing image processing");
}

void ImageProcessor::setup_message_handlers() {
  for (const auto &handler : m_message_handlers) {
    handler->setup(*m_od4);
  }
}

void ImageProcessor::process_frame() {
  auto logger = Logger::get_instance().get_logger();

  m_shared_memory->wait();
  cv::Mat image;

  {
    m_shared_memory->lock();
    if (!m_shared_memory->valid() || !m_shared_memory->data()) {
      logger->error("ImageProcessor: Invalid shared memory data");
      m_shared_memory->unlock();
      return;
    }

    cv::Mat wrapped(m_config.height, m_config.width, CV_8UC4,
                    m_shared_memory->data());
    image = wrapped.clone();
    auto sample_time_point =
        cluon::time::toMicroseconds(m_shared_memory->getTimeStamp().second);
    m_shared_memory->unlock();

    if (image.empty()) {
      logger->error("ImageProcessor: Cloned image is empty");
      return;
    }

    if (image.channels() == 4) {
      cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
    }

    float actual_steering = 0.0f;
    float steering_angle = 0.0f;

    if (m_detector) {
      ColorClassifiedCones detected_objects = m_detector->detect(image);

      cv::Point2f midpoint =
          m_path_finder->find_midpoint(detected_objects, image);
      cv::circle(image, midpoint, 3, cv::Scalar(255, 255, 255), cv::FILLED);

      steering_angle = m_path_finder->calculate_steering_angle(midpoint, image);
      logger->info("Calculated steering angle: {}", steering_angle);
    }

    if (m_gs_handler) {
      actual_steering = m_gs_handler->get_actual_steering_angle();
    }

    annotate_image(image, sample_time_point, actual_steering, steering_angle);
    log_steering(sample_time_point, actual_steering, steering_angle);

    if (m_config.is_verbose) {
      cv::imshow(m_config.shared_memory_name, image);
      cv::waitKey(1);
    }
  }
}

// Added logging in the image_processor since all the variables needed were here
// already
void ImageProcessor::log_steering(int64_t timestamp, float actual,
                                  float predicted) {
  static bool written = false;
  static std::ofstream outputFile(
      "/usr/bin/res/steering_data/steeringAngles.csv",
      std::ios::out | std::ios::trunc);
  if (!outputFile.is_open()) {
    std::cerr << "failed to open .csv file" << std::endl;
  }
  if (!written) {
    outputFile << "Timestamp;PredictedSteeringAngle;ActualSteeringAngle\n";
    written = true;
  }
  outputFile << timestamp << ";" << predicted << ";" << actual << "\n";
}

void ImageProcessor::annotate_image(cv::Mat &image, int sample_time_point,
                                    float actual_steering,
                                    float steering_angle) const {
  int font = cv::FONT_HERSHEY_COMPLEX;
  double font_scale = 0.6;
  cv::Scalar text_color(255, 255, 255);
  int text_thickness = 1;

  // display current UTC time
  auto now = cluon::time::now();
  std::time_t seconds = now.seconds();
  std::tm *utc_time = std::gmtime(&seconds);
  std::ostringstream time_stream;
  time_stream << "Now: " << std::put_time(utc_time, "%Y-%m-%dT%H:%M:%SZ");

  // display frame time stamp
  std::ostringstream ts_stream;
  ts_stream << "TS: " << sample_time_point;

  // display steering
  std::ostringstream steering_line1;
  std::ostringstream steering_line2;
  steering_line1 << "Current steering: " << steering_angle;
  steering_line2 << "Actual steering: " << actual_steering;

  int line_height = 20;
  int base_y = 20;

  cv::putText(image, time_stream.str(), cv::Point(10, base_y), font, font_scale,
              text_color, text_thickness);
  cv::putText(image, ts_stream.str(), cv::Point(10, base_y + line_height), font,
              font_scale, text_color, text_thickness);
  cv::putText(image, steering_line1.str(),
              cv::Point(10, base_y + 2 * line_height), font, font_scale,
              text_color, text_thickness);
  cv::putText(image, steering_line2.str(),
              cv::Point(10, base_y + 3 * line_height), font, font_scale,
              text_color, text_thickness);
}
