/**
 * @file YoloDetector.h
 * @brief YOLOv4-tiny object detection implementation using NCNN
 * @author AI Detection System
 * @date 2025-09-28
 */

#ifndef YOLO_DETECTOR_H
#define YOLO_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <ncnn/net.h>
#include <ncnn/mat.h>
#include <vector>
#include <algorithm>

/**
 * @struct Object
 * @brief Represents a detected object with bounding box and classification
 */
struct Object
{
    cv::Rect_<float> rect;  ///< Bounding box rectangle
    int label;              ///< Class label ID
    float prob;             ///< Detection confidence probability
};

/**
 * @class YoloDetector
 * @brief YOLOv4-tiny object detection engine using NCNN framework
 * 
 * This class implements object detection using YOLOv4-tiny model with NCNN
 * framework for efficient inference on CPU/GPU.
 */
class YoloDetector
{
public:
    YoloDetector();
    ~YoloDetector();
    
    int load(const std::string& modelpath, bool use_gpu = false);
    int detect(const cv::Mat& rgb, std::vector<Object>& objects, float prob_threshold = 0.25f, float nms_threshold = 0.45f);
    
    // Utility methods for drawing
    static void draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects);
    static const char* getClassName(int label);
    
private:
    ncnn::Net yolov4;
    int target_size = 416;
    float mean_vals[3] = {0.f, 0.f, 0.f};
    float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
    
    static inline float intersection_area(const Object& a, const Object& b)
    {
        cv::Rect_<float> inter = a.rect & b.rect;
        return inter.area();
    }
    
    static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right);
    static void qsort_descent_inplace(std::vector<Object>& objects);
    static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold);
    
    // Class names
    static const char* class_names_[];
};

#endif // YOLO_DETECTOR_H