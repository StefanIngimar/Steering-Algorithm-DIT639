#include <exception>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "config.hpp"
#include "logger.hpp"
#include "image_processor.hpp"
#include "object_detector.hpp"

ImageProcessor::ImageProcessor(
    const Config& config, std::shared_ptr<cluon::OD4Session> od4, std::unique_ptr<ObjectDetector> detector
)
    : m_config(config),
      m_od4(od4),
      m_detector(std::move(detector)),
      m_shared_memory(std::make_unique<cluon::SharedMemory>(config.shared_memory_name)),
      m_message_handlers() {

    auto logger = Logger::get_instance().get_logger();
    if (!m_detector) {
        logger->warn("[ImageProcessor] No object detector was provided - object detection will be omitted");
    }
};

void ImageProcessor::add_message_handler(std::shared_ptr<MessageHandler> message_handler) {
    m_message_handlers.push_back(message_handler);
}

void ImageProcessor::run() {
    auto logger = Logger::get_instance().get_logger();
    
    logger->info("ImageProcessor: Setting up message handlers");
    setup_message_handlers();

    logger->info("ImageProcessor: Starting image processing");
    while (m_od4->isRunning()) {
        try {
            process_frame();
        } catch (const std::exception& e) {
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
    for (const auto& handler : m_message_handlers) {
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

        cv::Mat wrapped(m_config.height, m_config.width, CV_8UC4, m_shared_memory->data());
        image = wrapped.clone();
        auto sample_time_point = cluon::time::toMicroseconds(m_shared_memory->getTimeStamp().second);
        m_shared_memory->unlock();

        if (image.empty()) {
            logger->error("ImageProcessor: Cloned image is empty");
            return;
        }

        if (image.channels() == 4) {
            cv::cvtColor(image, image, cv::COLOR_BGRA2BGR);
        }

        if (m_detector) {
            std::vector<cv::Rect> detected_objects = m_detector->detect(image);
            for (auto object : detected_objects) {
                cv::rectangle(image, object, cv::Scalar(0, 255, 255), 1);
            }
        }

        annotate_image(image, sample_time_point);
    }

    if (m_config.is_verbose) {
        cv::imshow(m_config.shared_memory_name, image);
        cv::waitKey(1);
    }
}

void ImageProcessor::annotate_image(cv::Mat& image, int sample_time_point) const {
        int font = cv::FONT_HERSHEY_COMPLEX;
        double font_scale = 0.6;
        cv::Scalar text_color(255, 255, 255);
        int text_thickness = 1;

        // display current UTC time
        auto now = cluon::time::now();
        std::time_t seconds = now.seconds();
        std::tm* utc_time = std::gmtime(&seconds);
        std::ostringstream time_stream;
        time_stream << "Now: " << std::put_time(utc_time, "%Y-%m-%dT%H:%M:%SZ");
        cv::putText(image, time_stream.str(), cv::Point(10, 20), font,
                    font_scale, text_color, text_thickness);

        // display frame time stamp
        std::ostringstream ts_stream;
        ts_stream << "TS: " << sample_time_point;
        cv::putText(image, ts_stream.str(), cv::Point(10, 37), font, font_scale,
                    text_color, text_thickness);
}
