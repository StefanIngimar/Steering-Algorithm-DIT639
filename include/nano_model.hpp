#pragma once

#include "ml_model.hpp"

class NanoModel final : public MlModel {
public:
    NanoModel(const std::string& model_path, int trained_frame_width, int trained_frame_height);

    void load() override;
    std::vector<cv::Mat> predict(const cv::Mat& image) override;

private:
    std::string m_model_path;
    cv::dnn::Net m_net;
};

