#include "cv_dnn_runtime.hpp"
#include "logger.hpp"

CvDnnRuntime::CvDnnRuntime(const std::string& model_path, int trained_frame_width, int trained_frame_height) 
    : m_model_path(model_path), m_trained_frame_width(trained_frame_width), m_trained_frame_height(trained_frame_height) {
}

void CvDnnRuntime::load() {
    auto logger = Logger::get_instance().get_logger();
    logger->info("CvDnnRuntime: Loading model using at '{}'", m_model_path);

    try {
        m_net = cv::dnn::readNetFromONNX(m_model_path);
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    } catch (const std::exception& e) {
        logger->error("CvDnnRuntime: Error while loading model using CV DNN Runtime: {}", e.what());
        throw;
    } catch (...) {
        logger->error("CvDnnRuntime: Unknown error while loading model using CV DNN Runtime");
        throw;
    }

    logger->info("CvDnnRuntime: Model loaded");
}

std::vector<cv::Mat> CvDnnRuntime::predict(const cv::Mat& image) {
    if (image.empty()) {
        return {};
    }

    cv::Mat blob;
    cv::dnn::blobFromImage(
        image, blob, 1.0 / 255.0,
        cv::Size(m_trained_frame_width, m_trained_frame_height),
        cv::Scalar(), true, false
    );
    m_net.setInput(blob);

    std::vector<cv::Mat> outputs;
    m_net.forward(outputs, m_net.getUnconnectedOutLayersNames());

    return outputs;
}

int CvDnnRuntime::get_trained_frame_width() const {
    return m_trained_frame_width;
}

int CvDnnRuntime::get_trained_frame_height() const {
    return m_trained_frame_height;
}
