#include "average_x_path_finder.hpp"
#include "logger.hpp"

cv::Point2f AverageXPathFinder::find_midpoint(const ColorClassifiedCones& detection_result, const cv::Mat& frame) {
    auto logger = Logger::get_instance().get_logger();

    auto blue_cones = detection_result.blue_cones;
    auto yellow_cones = detection_result.yellow_cones;
    sort_by_y_desc(blue_cones);
    sort_by_y_desc(yellow_cones);

    filter_out_objects_outside_threshold(blue_cones, frame);
    filter_out_objects_outside_threshold(yellow_cones, frame);

    std::vector<cv::Rect> closest_blue_cones;
    std::vector<cv::Rect> closest_yellow_cones;
    find_closest_objects(blue_cones, closest_blue_cones);
    find_closest_objects(yellow_cones, closest_yellow_cones);

    std::vector<cv::Point2f> closest_blue_cone_centers;
    std::vector<cv::Point2f> closest_yellow_cone_centers;
    find_object_centers(closest_blue_cones, closest_blue_cone_centers);
    find_object_centers(closest_yellow_cones, closest_yellow_cone_centers);

    logger->info(
        "Found cones: left {}, right {}; Closest cones: left {}, right {}", 
        blue_cones.size(), 
        yellow_cones.size(),
        closest_blue_cones.size(),
        closest_yellow_cones.size()
    );

    cv::Point2f midpoint(0.0f, 0.0f);
    if (!closest_blue_cone_centers.empty() && !closest_yellow_cone_centers.empty()) {
        cv::Point2f left_pos_avg(0.0f, 0.0f);
        for (const auto& clc : closest_blue_cone_centers) {
            left_pos_avg += clc;
        }
        left_pos_avg /= static_cast<float>(closest_blue_cone_centers.size());

        cv::Point2f right_pos_avg(0.0f, 0.0f);
        for (const auto& crc : closest_yellow_cone_centers) {
            right_pos_avg += crc;
        }
        right_pos_avg /= static_cast<float>(closest_yellow_cone_centers.size());

        midpoint.x = (left_pos_avg.x + right_pos_avg.x) / 2;
        midpoint.y = (left_pos_avg.y + right_pos_avg.y) / 2;
    }

    return midpoint;
}

float AverageXPathFinder::calculate_steering_angle(const cv::Point2f& midpoint, const cv::Mat& frame) {
    if (std::abs(midpoint.x) < 1e-5f && std::abs(midpoint.y) < 1e-5f) {
        return 0.0f;
    }

    if (frame.cols == 0) {
        auto logger = Logger::get_instance().get_logger();
        logger->warn("Calculate steering angle received empty frame");
        return 0.0f;
    }

    float image_center_x = frame.cols / 2.0f;
    float deviation = (midpoint.x - image_center_x) / (frame.cols / 2.0f);

    float scaled_deviation = deviation * m_sensitivity;
    
    // since going left is positive, the multiplication by -1.0f is necessary to 'mirror' the calculation
    float steering_angle = scaled_deviation * m_max_steering_angle * -1.0f;

    return std::max(-m_max_steering_angle, std::min(m_max_steering_angle, steering_angle));
}

/*
 * Select only the specified number of objects and use those objects to popule the 'closest_objects' vector.
 * */
void AverageXPathFinder::find_closest_objects(
    const std::vector<cv::Rect>& all_objects, std::vector<cv::Rect>& closest_objects, int to_find
) {
    closest_objects.assign(
        all_objects.begin(), all_objects.begin() + std::min<size_t>(to_find, all_objects.size())
    );
}

void AverageXPathFinder::find_object_centers(const std::vector<cv::Rect>& all_objects, std::vector<cv::Point2f>& object_centers) {
    for (const auto& obj : all_objects) {
        cv::Point2f center_point(obj.x + obj.width / 2.0f, obj.y + obj.height / 2.0f);
        object_centers.emplace_back(center_point);
    }
}

/*
 * Sort detected objects vector to put detected objects that are closest to the car at the beginning of the vector.
 * */
void AverageXPathFinder::sort_by_y_desc(std::vector<cv::Rect>& objects) {
    std::sort(objects.begin(), objects.end(), [](const cv::Rect& a, const cv::Rect& b) {
        return a.y > b.y;
    });
}

/**
 * Remove objects from the array that are outside the defined threshold.
 * 
 * The valid threshold is described as the area between car's bumper and the middle of the frame.
 */
void AverageXPathFinder::filter_out_objects_outside_threshold(std::vector<cv::Rect>& objects, const cv::Mat& frame) {
    int bottom_threshold = static_cast<int>(frame.rows * 0.5f);
    int upper_threshold = static_cast<int>(frame.rows - 125);

    for (auto obj = objects.begin(); obj != objects.end();) {
        if (obj->y + obj->height < bottom_threshold || obj->y + obj->height > upper_threshold) {
            obj = objects.erase(obj);
        } else {
            obj += 1;
        }
    }
}
