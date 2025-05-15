#include "average_x_path_finder.hpp"
#include "logger.hpp"

cv::Point2f AverageXPathFinder::find_midpoint(const ColorClassifiedCones& detection_result, const cv::Mat& frame) {
    auto logger = Logger::get_instance().get_logger();

    std::vector<cv::Rect> blue_cones = detection_result.blue_cones;
    std::vector<cv::Rect> yellow_cones = detection_result.yellow_cones;

    sort_and_filter(blue_cones, frame);
    sort_and_filter(yellow_cones, frame);

    auto blue_centers = get_closest_centers(blue_cones);
    auto yellow_centers = get_closest_centers(yellow_cones);

    logger->info(
        "Found cones: left {}, right {}; Closest cones: left {}, right {}", 
        blue_cones.size(), 
        yellow_cones.size(),
        blue_centers.size(),
        yellow_centers.size()
    );

    cv::Point2f midpoint(0.0f, 0.0f);
    if (!blue_centers.empty() && !yellow_centers.empty()) {
        cv::Point2f left_pos_avg(0.0f, 0.0f);
        for (const auto& clc : blue_centers) {
            left_pos_avg += clc;
        }
        left_pos_avg /= static_cast<float>(blue_centers.size());

        cv::Point2f right_pos_avg(0.0f, 0.0f);
        for (const auto& crc : yellow_centers) {
            right_pos_avg += crc;
        }
        right_pos_avg /= static_cast<float>(yellow_centers.size());

        midpoint.x = (left_pos_avg.x + right_pos_avg.x) / 2;
        midpoint.y = (left_pos_avg.y + right_pos_avg.y) / 2;
    }

    return midpoint;
}

float AverageXPathFinder::calculate_steering_angle(const cv::Point2f& midpoint, const cv::Mat& frame) {
    if (std::abs(midpoint.x) < 1e-5f) {
        return 0.0f;
    }

    if (frame.cols == 0) {
        auto logger = Logger::get_instance().get_logger();
        logger->warn("Calculate steering angle received empty frame");
        return 0.0f;
    }

    float image_center_x = frame.cols / 2.0f;
    float deviation = (midpoint.x - image_center_x) / image_center_x;

    float scaled_deviation = deviation * m_sensitivity;
    
    // since going left is positive, the multiplication by -1.0f is necessary to 'mirror' the calculation
    constexpr float STEERING_MIRROR = -1.0f;
    float steering_angle = scaled_deviation * m_max_steering_angle * STEERING_MIRROR;

    return std::clamp(steering_angle, -m_max_steering_angle, m_max_steering_angle);
}

/**
 * Get closest centers of detected objects in relation to the car (bottom of the frame).
 */
std::vector<cv::Point2f> AverageXPathFinder::get_closest_centers(const std::vector<cv::Rect>& objects, int to_find) {
    std::vector<cv::Point2f> centers;
    auto end = objects.begin() + std::min<int>(to_find, objects.size());
    for (auto iter = objects.begin(); iter != objects.end(); iter += 1) {
        cv::Point2f center(iter->x + iter->width / 2.0f, iter->y + iter->height / 2.0f);
        centers.emplace_back(center);
    }

    return centers;
}

/*
 * Remove objects from the array that are outside the defined threshold.
 * The valid threshold is described as the area between car's bumper and the middle of the frame.
 * 
 * After filering, sort detected objects vector to put detected objects that are closest to the car at the beginning of the vector.
 * */
void AverageXPathFinder::sort_and_filter(std::vector<cv::Rect>& objects, const cv::Mat& frame) {
    int bottom_threshold = static_cast<int>(frame.rows * 0.5f);
    int upper_threshold = static_cast<int>(frame.rows - 125);

    objects.erase(
        std::remove_if(objects.begin(), objects.end(), [bottom_threshold, upper_threshold](const cv::Rect& obj) {
            return obj.y + obj.height < bottom_threshold || obj.y + obj.height > upper_threshold;
        }), objects.end()
    );

    std::sort(objects.begin(), objects.end(), [](const cv::Rect& a, const cv::Rect& b) {
        return a.y > b.y;
    });
}