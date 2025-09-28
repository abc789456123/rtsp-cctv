/**
 * @file Application.h
 * @brief Main application class that integrates all components
 * @author AI Detection System
 * @date 2025-09-28
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <opencv2/opencv.hpp>
#include <memory>
#include <atomic>
#include <chrono>

#include "ConfigManager.h"
#include "YoloDetector.h"
#include "RtspStreamer.h"
#include "MetadataPublisher.h"

/**
 * @class Application
 * @brief Main application class that manages the entire AI detection system
 * 
 * This class integrates all components including object detection, RTSP streaming,
 * and metadata publishing. It handles the main processing loop and coordinates
 * between different modules.
 */
class Application {
public:
    /**
     * @brief Default constructor
     */
    Application();
    
    /**
     * @brief Destructor
     */
    ~Application();
    
    /**
     * @brief Initialize the application with configuration
     * @param config_file Path to the JSON configuration file
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& config_file = "config.json");
    
    /**
     * @brief Run the main application loop
     * @return Exit code (0 for success)
     */
    int run();
    
    /**
     * @brief Stop the application gracefully
     */
    void stop();
    
private:
    // Core components
    std::unique_ptr<ConfigManager> config_manager_;        ///< Configuration manager instance
    std::unique_ptr<YoloDetector> yolo_detector_;          ///< YOLO object detector instance
    std::unique_ptr<RtspStreamer> rtsp_streamer_;          ///< RTSP video streamer instance
    std::unique_ptr<MetadataPublisher> metadata_publisher_; ///< Metadata publisher instance
    
    // Camera and processing
    std::unique_ptr<cv::VideoCapture> camera_;             ///< Camera capture instance
    std::atomic<bool> running_;                            ///< Application running state flag
    
    // Timing for metadata publishing
    std::chrono::steady_clock::time_point last_metadata_time_;
    
    // Private methods
    bool initializeCamera();
    bool initializeComponents();
    void processFrame(const cv::Mat& frame);
    void handleKeyInput(char key);
    void printStatistics();
    
    // Statistics
    int frame_count_;
    int detection_count_;
    std::chrono::steady_clock::time_point start_time_;
};

#endif // APPLICATION_H