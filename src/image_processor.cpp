#include "image_processor.hpp"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sstream>

#include "config.hpp"
#include "ground_steering_message_handler.hpp"
#include "logger.hpp"
#include "object_detector.hpp"

ImageProcessor::ImageProcessor(
    const Config &config, std::shared_ptr<cluon::OD4Session> od4,
    std::unique_ptr<ObjectDetector> detector,
    std::unique_ptr<PathFinder> path_finder,
    std::shared_ptr<GroundSteeringMessageHandler> gs_handler)
    : m_config(config),
      m_od4(od4),
      m_detector(std::move(detector)),
      m_shared_memory(
          std::make_unique<cluon::SharedMemory>(config.shared_memory_name)),
      m_path_finder(std::move(path_finder)),
      m_gs_handler(gs_handler),
      m_processed_frames(0),
      m_correctly_calculated_steering_angle(0) {
  auto logger = Logger::get_instance().get_logger();
  if (!m_detector) {
    logger->warn(
        "[ImageProcessor] No object detector was provided - object "
        "detection will be omitted");
  }
};

void ImageProcessor::run() {
  auto logger = Logger::get_instance().get_logger();

  logger->info("ImageProcessor: Starting image processing");
  while (m_od4->isRunning()) {
    try {
      process_frame();
      m_processed_frames += 1;
    } catch (const std::exception &e) {
      logger->error("ImageProcessor: Raised exception: {}", e.what());
      throw;
    } catch (...) {
      logger->error("ImageProcessor: Raised unknown exception");
      throw;
    }
  }

  logger->info("Processed frames: {}", m_processed_frames);
  logger->info("Correctly calculated steering in: {}",
               m_correctly_calculated_steering_angle);
  logger->info(
      "Incorrectly calculated steering in: {}",
      m_evaluated_frames -
          m_correctly_calculated_steering_angle);  // evaluated frames are the
                                                   // non-zero values
  // check whether we got data we can calculate
  if (m_evaluated_frames > 0) {
    double correctness = (m_correctly_calculated_steering_angle /
                          static_cast<double>(m_evaluated_frames)) *
                         100;
    logger->info("Evaluated frames: {}", m_evaluated_frames);
    logger->info("Correctness score: {:.2f}%", correctness);
  } else {
    logger->info("No non-zero steering values to calculate correctness");
  }

  logger->info("ImageProcessor: Closing image processing");
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
    // i had to change this back for the timestamps to be correct
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
    if (m_gs_handler) {
      actual_steering = m_gs_handler->get_actual_steering_angle();
    }

    if (m_detector) {
      ColorClassifiedCones detected_objects = m_detector->detect(image);

      cv::Point2f midpoint =
          m_path_finder->find_midpoint(detected_objects, image);
      cv::circle(image, midpoint, 3, cv::Scalar(255, 255, 255), cv::FILLED);

      steering_angle = m_path_finder->calculate_steering_angle(midpoint, image);
    }

    std::cout << "group_04;" << sample_time_point << ";" << steering_angle << std::endl;

    if (m_config.should_generate_plot) {
      log_steering(sample_time_point, actual_steering, steering_angle);
    }

    // filter out actual steering angles where the value is 0
    if (std::abs(actual_steering) >= 1e-6) {
      m_evaluated_frames += 1;
      if (std::abs(steering_angle - actual_steering) <= 0.09f) {
        m_correctly_calculated_steering_angle += 1;
      }
    }

    if (m_config.is_verbose) {
      annotate_image(image, sample_time_point, actual_steering, steering_angle);
      cv::imshow(m_config.shared_memory_name, image);
      cv::waitKey(1);
    }
  }
}

// Added logging in the image_processor since all the variables needed were here
// already
void ImageProcessor::log_steering(int64_t timestamp, float actual,
                                  float predicted) {
  auto logger = Logger::get_instance().get_logger();

  static bool written = false;

  static std::string filename = []() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y%m%d_%H%M%S");
    std::string datetime_str = oss.str();

    std::string dir = "/data/csv";
    std::filesystem::create_directories(dir);

    std::string fullpath = dir + "/steeringAngles_" + datetime_str + ".csv";
    std::cerr << "Writing log to: " << fullpath << std::endl;
    return fullpath;
  }();
  static std::ofstream outputFile(filename, std::ios::out | std::ios::trunc);
  if (!outputFile.is_open()) {
    logger->error("Failed to open .csv file at '{}'", filename);
    return;
  }
  if (!written) {
    outputFile << "Timestamp;PredictedSteeringAngle;ActualSteeringAngle\n";
    written = true;
  }
  outputFile << timestamp << ";" << predicted << ";" << actual << "\n";
}

void ImageProcessor::annotate_image(cv::Mat &image, int64_t timestamp,
                                    float actual_steering,
                                    float steering_angle) const {
  const float steering_angle_difference =
      std::abs(steering_angle - actual_steering);
  std::array<std::string, 5> words = {
      "TS: " + std::to_string(timestamp),
      "Calculated: " + std::to_string(steering_angle),
      "Actual:  " + std::to_string(actual_steering),
      "Difference: " + std::to_string(steering_angle_difference),
      std::string("Is Valid: ") +
          (steering_angle_difference <= 0.09f ? "Yes" : "No")};

  cv::rectangle(image, cv::Point(0, 0),
                cv::Point(175, M_BASE_Y * (words.size() + 1)),
                cv::Scalar(0, 0, 0), cv::FILLED);

  for (std::size_t i = 0; i < words.size(); i += 1) {
    cv::putText(image, words[i], cv::Point(10, M_BASE_Y + i * M_LINE_HEIGHT),
                M_FONT, M_FONT_SCALE, M_TEXT_COLOR, M_TEXT_THICKNESS);
  }
}
