#include "ml_model.hpp"

MlModel::MlModel(int trained_frame_width, int trained_frame_height)
    : m_trained_frame_width(trained_frame_width), m_trained_frame_height(trained_frame_height) {
}

int MlModel::get_trained_frame_width() const {
    return m_trained_frame_width;
}

int MlModel::get_trained_frame_height() const {
    return m_trained_frame_height;
}

