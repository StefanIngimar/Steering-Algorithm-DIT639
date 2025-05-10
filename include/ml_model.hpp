#pragma once

#include <opencv2/dnn/dnn.hpp>

class MlModel {
public:
    MlModel(int trained_frame_width, int trained_frame_height);
    virtual ~MlModel() = default;

    virtual void load() = 0;
    virtual std::vector<cv::Mat> predict(const cv::Mat& image) = 0;

    int get_trained_frame_width() const;
    int get_trained_frame_height() const;

private:
    const int m_trained_frame_width;
    const int m_trained_frame_height;
};

