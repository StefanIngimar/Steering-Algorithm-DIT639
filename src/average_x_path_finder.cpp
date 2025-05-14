#include "average_x_path_finder.hpp"
#include "logger.hpp"

cv::Point2f AverageXPathFinder::find_midpoint(const std::vector<cv::Rect> detection_result, const cv::Mat& frame) {
    auto logger = Logger::get_instance().get_logger();
    
    std::vector<cv::Point2f> left_object_centers;
    std::vector<cv::Point2f> right_object_centers;
    separate_detected_objects(frame, detection_result, left_object_centers, right_object_centers);

    sort_by_y_desc(left_object_centers);
    sort_by_y_desc(right_object_centers);

    std::vector<cv::Point2f> closest_left_centers;
    std::vector<cv::Point2f> closest_right_centers;
    find_closest_objects(left_object_centers, closest_left_centers);
    find_closest_objects(right_object_centers, closest_right_centers);

    logger->info(
        "Detected cones: left {}, right {}; Closest cones: left {}, right {}", 
        left_object_centers.size(), 
        right_object_centers.size(),
        closest_left_centers.size(),
        closest_right_centers.size()
    );

    cv::Point2f midpoint(0.0f, 0.0f);
    if (!closest_left_centers.empty() && !closest_right_centers.empty()) {
        cv::Point2f left_pos_avg(0.0f, 0.0f);
        for (const auto& clc : closest_left_centers) {
            left_pos_avg += clc;
        }
        left_pos_avg /= static_cast<float>(closest_left_centers.size());

        cv::Point2f right_pos_avg(0.0f, 0.0f);
        for (const auto& crc : closest_right_centers) {
            right_pos_avg += crc;
        }
        right_pos_avg /= static_cast<float>(closest_right_centers.size());

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
    const std::vector<cv::Point2f>& all_objects, std::vector<cv::Point2f>& closest_objects, int to_find
) {
    closest_objects.assign(
        all_objects.begin(), all_objects.begin() + std::min<size_t>(to_find, all_objects.size())
    );
}

/*
 * Separate detected objects in place by comparing the object position with the middle of the frame.
 * Only objects within a given threshold (lower bottom of the frame to the bumper of the car) will be taken
 * into consideration.
 * */
void AverageXPathFinder::separate_detected_objects(
    const cv::Mat& frame, const std::vector<cv::Rect>& objects, std::vector<cv::Point2f>& left, std::vector<cv::Point2f>& right
) {
    left.clear();
    right.clear();

    // only the detected cones that are between the thresholds will be taken into
    // consideration while finding the midpoint
    int bottom_threshold = static_cast<int>(frame.rows * 0.5);
    int upper_threshold = frame.rows - 125;

    float image_center_x = frame.cols / 2.0f;

    for (const auto& obj : objects) {
        cv::Point2f obj_center(obj.x + obj.width / 2.0f, obj.y + obj.height / 2.0f);

        if (obj.y + obj.height < bottom_threshold || obj.y + obj.height > upper_threshold) {
            continue;
        }

        if (obj_center.x < image_center_x) {
            left.push_back(obj_center);
        } else if (obj_center.x > image_center_x) {
            right.push_back(obj_center);
        }
    }
}

/*
 * Sort detected objects vector to put detected objects that are closest to the car at the beginning of the vector.
 * */
void AverageXPathFinder::sort_by_y_desc(std::vector<cv::Point2f>& objects) {
    std::sort(objects.begin(), objects.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.y > b.y;
    });
}
