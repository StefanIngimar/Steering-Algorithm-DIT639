#pragma once

#include "ml_model_runtime.hpp"

class CvDnnRuntime : public MlModelRuntime {
public:
    CvDnnRuntime(const std::string& model_path);
    ~CvDnnRuntime() override = default;

    void load() override;
    std::vector<cv::Mat> predict(const cv::Mat& image) const override;

private:
    const std::string& m_model_path;
}