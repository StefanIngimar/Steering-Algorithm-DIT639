#pragma once

#include <string>
#include <vector>

#include <onnxruntime_cxx_api.h>

#include "ml_model_runtime.hpp"

class OnnxRuntime : public MlModelRuntime {
public:
    OnnxRuntime(const std::string& model_path, int trained_frame_width, int trained_frame_height);
    ~OnnxRuntime() override = default;

    void load() override;
    std::vector<cv::Mat> predict(const cv::Mat& image) override;

    int get_trained_frame_width() const override;
    int get_trained_frame_height() const override;

private:
    std::string m_model_path;
    int m_trained_frame_width;
    int m_trained_frame_height;

    Ort::Env m_env;
    Ort::Session m_session;
    std::vector<std::string> m_input_names;
    std::vector<std::string> m_output_names;
    std::vector<int64_t> m_input_shape;
};
