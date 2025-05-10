#include "nano_model.hpp"
#include "ml_model.hpp"

NanoModel::NanoModel(const std::string& model_path, const int trained_frame_width, const int trained_frame_height)
    : MlModel(trained_frame_width, trained_frame_height), m_model_path(model_path) {
}

void NanoModel::load() {
    m_net = cv::dnn::readNetFromONNX(m_model_path);
    m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
}

std::vector<cv::Mat> NanoModel::predict(const cv::Mat& image) {
    if (image.empty()) {
        return {};
    }

    cv::Mat blob;
    cv::dnn::blobFromImage(
        image, blob, 1.0 / 255.0,
        cv::Size(get_trained_frame_width(),get_trained_frame_height()),
        cv::Scalar(), true, false
    );
    m_net.setInput(blob);

    std::vector<cv::Mat> outputs;
    m_net.forward(outputs, m_net.getUnconnectedOutLayersNames());

    return outputs;
}

