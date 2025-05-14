#pragma once

#include "config.hpp"
#include "message_handler.hpp"
#include "object_detector.hpp"
#include "path_finder.hpp"

class ImageProcessor {
public:
    ImageProcessor(
        const Config& config, 
        std::shared_ptr<cluon::OD4Session> od4, 
        std::unique_ptr<ObjectDetector> detector,
        std::unique_ptr<PathFinder> path_finder
    );

    void add_message_handler(std::shared_ptr<MessageHandler> message_handler);
    void run();
private:
    const Config& m_config;
    std::shared_ptr<cluon::OD4Session> m_od4;
    std::unique_ptr<ObjectDetector> m_detector;

    std::unique_ptr<cluon::SharedMemory> m_shared_memory;

    std::vector<std::shared_ptr<MessageHandler>> m_message_handlers;
    std::unique_ptr<PathFinder> m_path_finder;
    
    void setup_message_handlers();
    void process_frame();
    void annotate_image(cv::Mat& image, int sample_time_point) const;
};
