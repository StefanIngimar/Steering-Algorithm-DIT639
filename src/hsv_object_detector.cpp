#include <stdexcept>

#include "hsv_object_detector.hpp"

HsvObjectDetector::HsvObjectDetector() {}

ColorClassifiedCones HsvObjectDetector::detect(const cv::Mat& frame) const {
    cv::Mat HSVFrame, yellowMask, blueMask, combinedMask;

    // Image coordinates are zero-indexed in OpenCV
    float frameRows = frame.rows - 1.0f;   

    // Stage (1) === Preprocessing ===

    // ImageProcessor class performs BGRA2BGR conversion

    // Create CLAHE object for contrast enhancement
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));

    // Contrast adjustment: appply CLAHE on the Y channel
    cv::Mat ycrcb;
    cv::cvtColor(frame, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);
    clahe->apply(channels[0], channels[0]);
    cv::merge(channels, ycrcb);
    cv::Mat contrastEnhanced;
    cv::cvtColor(ycrcb, contrastEnhanced, cv::COLOR_YCrCb2BGR);

    // Region of interest (ROI) mask
    cv::Mat roiMask = cv::Mat::zeros(contrastEnhanced.size(), contrastEnhanced.type());
    // Define ROI polygon
    std::vector<cv::Point> roiPolygon = {
        cv::Point(0, 380),      // Bottom left
        cv::Point(640, 380),    // Bottom right
        cv::Point(640, 200),    // Top right
        cv::Point(0, 200)       // Top left 
    };
    // Fill polygon white on black mask
    std::vector<std::vector<cv::Point>> polygons = {roiPolygon};
    cv::fillPoly(roiMask, polygons, cv::Scalar(255, 255, 255));

    // Optional: draw source points for visualization
    for (uint16_t i = 0; i < 4; i++) {
        cv::circle(frame, roiPolygon[i], 5, cv::Scalar(0, 255, 0), cv::FILLED);
    }

    // Apply mask to extract ROI   
    cv::Mat roiFrame;
    cv::bitwise_and(contrastEnhanced, roiMask, roiFrame);

    // Convert frame color model 
    cv::cvtColor(roiFrame, HSVFrame, cv::COLOR_BGR2HSV);

    // HSV color segmentation

    // Apply threshold values to get binary mask
    cv::inRange(HSVFrame, cv::Scalar(15, 80, 30),
                cv::Scalar(40, 255, 255), yellowMask);
    cv::inRange(HSVFrame, cv::Scalar(15, 120, 30),
                cv::Scalar(160, 255, 255), blueMask);

    // Morphological operations (to clean up the mask)
    // Kernel sizes: 3x3, 5x5, and 7x7
    // Use larger kernel for more noisy frames
    // Use smaller kernel for clean frames
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(yellowMask, yellowMask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(yellowMask, yellowMask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(blueMask, blueMask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(blueMask, blueMask, cv::MORPH_CLOSE, kernel);

    // Combine both masks for contour detection
    cv::bitwise_or(yellowMask, blueMask, combinedMask);

    // Stage (2) === Detection  ===

    // Contour detection and labeling on original frame

    // Find potential cones
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combinedMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // Vertical range of interest for cones 
    const float LOWER_BOUND = frameRows * 1.0f/2.0f; 
    const float UPPER_BOUND = frameRows * 3.0f/4.0f; 

    // Object/cone area thresholds
    const float SMALLEST_CONE = 50.0f;
    const float BIGGEST_CONE = 400.0f;

    std::vector<cv::Rect> blueBoxes;
    std::vector<cv::Rect> yellowBoxes;

    // Filter contours based on area
    // Classify contours based on color 
    // Draw bounding boxes and add labels 
    for (const auto& contour : contours) {
        float objArea = static_cast<float>(cv::contourArea(contour));
        // Filter out very small or very large objects. Adjust these values as needed
        if (objArea < SMALLEST_CONE || objArea > BIGGEST_CONE) 
            continue; 

        // Bounding box 
        cv::Rect bbox = cv::boundingRect(contour);
        cv::Point2f coneBottomCenter(bbox.x + bbox.width / 2, bbox.y + bbox.height);

        // Get amount of yellow and blue within bounding box
        int32_t yellowAmount = cv::countNonZero(yellowMask(bbox));
        int32_t blueAmount = cv::countNonZero(blueMask(bbox));

        std::string label;  // bounding box label
        cv::Scalar bbColor; // bounding box color

        // Focus ahead: consider cones just below bottom half of frame but not
        // too close to the vehicle. Adjust range as needed 
        if ((coneBottomCenter.y > LOWER_BOUND) && (coneBottomCenter.y < UPPER_BOUND)) {
            // Contour classification based on color 
            if (yellowAmount > blueAmount) {
                label = "yellow";
                bbColor = cv::Scalar(0, 255, 255);  // Yellow
                yellowBoxes.push_back(bbox);
            } else if (blueAmount > yellowAmount) {
                label = "blue";
                bbColor = cv::Scalar(255, 0, 0);  // Blue
                blueBoxes.push_back(bbox);
            } else {
                continue; // uncertain, skip
            }
        }

        // Contour visualization: bounding boxes and classification labels
        cv::rectangle(frame, bbox, bbColor, 2);
        cv::circle(frame, coneBottomCenter, 4, bbColor, -1);
        cv::putText(frame, label, bbox.tl() - cv::Point(0, 10), 
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, bbColor, 2);
    }

    return {blueBoxes, yellowBoxes};
}
