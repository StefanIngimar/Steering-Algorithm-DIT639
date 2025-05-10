// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <ctime>

// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications 
#include "opendlv-standard-message-set.hpp"

#include "logger.hpp"
#include "config.hpp"

#include "ml_model.hpp"
#include "nano_model.hpp"

#include "ml_object_detector.hpp"

#include "image_processor.hpp"
#include "ground_steering_message_handler.hpp"

int main(int argc, char** argv) {
    auto logger = Logger::get_instance().get_logger();
    auto config = Config::parse_config(argc, argv);

    std::shared_ptr<MlModel> model = std::make_shared<NanoModel>("res/ml_models/nano.onnx", 320, 320);
    auto detector = std::make_unique<MlObjectDetector>(model);

    std::unique_ptr<cluon::SharedMemory> shared_memory(new cluon::SharedMemory{config.shared_memory_name});
    if (!shared_memory || !shared_memory->valid()) {
        throw std::runtime_error("Failed to attach to shared memory: " + config.shared_memory_name);
    }

    logger->info("{}: Attached to shared memory '{}' ({} bytes).", argv[0], shared_memory->name(), shared_memory->size());

    std::shared_ptr<cluon::OD4Session> od4 = std::make_shared<cluon::OD4Session>(config.cid);
    ImageProcessor processor(config, od4, std::move(detector));

    auto gs_msg_handler = std::make_shared<GroundSteeringMessageHandler>();
    processor.add_message_handler(gs_msg_handler);

    processor.run();

    return 0;
}
