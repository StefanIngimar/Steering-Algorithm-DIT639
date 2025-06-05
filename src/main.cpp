// Include the GUI and image processing header files from OpenCV
#include <ctime>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Include the single-file, header-only middleware libcluon to create
// high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are
// usually exchanged for automotive or robotic applications
#include <chrono>
#include <thread>

#include "average_x_path_finder.hpp"
#include "config.hpp"
#include "cv_dnn_runtime.hpp"
#include "focal_x_path_finder.hpp"
#include "ground_steering_message_handler.hpp"
#include "hsv_object_detector.hpp"
#include "image_processor.hpp"
#include "logger.hpp"
#include "ml_model_runtime.hpp"
#include "ml_object_detector.hpp"
#include "opendlv-standard-message-set.hpp"

void attach_to_shared_memory(const Config& config) {
    auto logger = Logger::get_instance().get_logger();
    std::unique_ptr<cluon::SharedMemory> shared_memory;

    const int max_attempts = 300;
    const float backoff_factor = 1.5f;
    const int max_sleep_ms = 5000;

    int sleep_ms = 500;
    
    logger->info("Attempting to attach to shared memory: '{}'", config.shared_memory_name);
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        shared_memory = std::make_unique<cluon::SharedMemory>(config.shared_memory_name);
        if (shared_memory && shared_memory->valid()) {
            logger->info("Successfully attached to shared memory after {} attempts", attempt);
            break;
        }

        logger->info("Could not attach to shared memory, retrying after {} ms", sleep_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        sleep_ms = std::min(static_cast<int>(sleep_ms * backoff_factor), max_sleep_ms);
    }

    if (!shared_memory || !shared_memory->valid()) {
      throw std::runtime_error("Failed to attach to shared memory after waiting: " + config.shared_memory_name);
    }

    logger->info("Attached to shared memory '{}' ({} bytes).", shared_memory->name(), shared_memory->size());
}

int main(int argc, char **argv) {
  auto logger = Logger::get_instance().get_logger();
  auto config = Config::parse_config(argc, argv);

  try {
    // std::shared_ptr<MlModelRuntime> model_runtime =
    //     std::make_shared<CvDnnRuntime>("res/ml_models/colored_cones_nano.onnx",
    //                                    320, 320);
    // auto detector = std::make_unique<MlObjectDetector>(model_runtime);
    auto detector = std::make_unique<HsvObjectDetector>();
    // auto path_finder = std::make_unique<AverageXPathFinder>();
    auto path_finder = std::make_unique<FocalXPathFinder>();
    
    attach_to_shared_memory(config);

    auto gs_msg_handler = std::make_shared<GroundSteeringMessageHandler>();
    std::shared_ptr<cluon::OD4Session> od4 = std::make_shared<cluon::OD4Session>(config.cid);
    gs_msg_handler->setup(*od4);

    ImageProcessor processor(config, od4, std::move(detector), std::move(path_finder), gs_msg_handler);
    processor.run();
  } catch (const std::exception &e) {
    logger->error("Main: Raised exception: {}", e.what());
    return 1;
  } catch (...) {
    logger->error("Main: Raised unknown exception");
    return 1;
  }

  return 0;
}
