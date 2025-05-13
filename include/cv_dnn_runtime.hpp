#pragma once

#include "ml_model_runtime.hpp"

class CvDnnRuntime : public MlModelRuntime {
public:
    CvDnnRuntime(const std::string& model_path, int trained_frame_width, int trained_frame_height);
    ~CvDnnRuntime() override = default;

    void load() override;
    std::vector<cv::Mat> predict(const cv::Mat& image) override;
    
    int get_trained_frame_width() const override;
    int get_trained_frame_height() const override;
private:
    std::string m_model_path;
    int m_trained_frame_width;
    int m_trained_frame_height;

    cv::dnn::Net m_net;
};
