/**
 * @file Application.cpp
 * @brief Implementation of the main Application class
 * @author AI Detection System
 * @date 2025-09-28
 */

#include "Application.h"
#include <iostream>
#include <chrono>
#include <thread>

Application::Application() 
    : running_(false), frame_count_(0), detection_count_(0) {
    
    config_manager_ = std::make_unique<ConfigManager>();
    yolo_detector_ = std::make_unique<YoloDetector>();
    rtsp_streamer_ = std::make_unique<RtspStreamer>();
    metadata_publisher_ = std::make_unique<MetadataPublisher>();
}

Application::~Application() {
    stop();
}

bool Application::initialize(const std::string& config_file) {
    std::cout << "=== Initializing AI Detection System ===" << std::endl;
    
    // Load configuration
    if (!config_manager_->loadConfig(config_file)) {
        std::cerr << "Failed to load configuration" << std::endl;
        return false;
    }
    
    config_manager_->printConfig();
    
    // Initialize all components
    if (!initializeComponents()) {
        std::cerr << "Failed to initialize components" << std::endl;
        return false;
    }
    
    // Initialize camera
    if (!initializeCamera()) {
        std::cerr << "Failed to initialize camera" << std::endl;
        return false;
    }
    
    std::cout << "=== System Initialized Successfully ===" << std::endl;
    return true;
}

bool Application::initializeComponents() {
    const auto& config = config_manager_->getConfig();
    
    // Initialize YOLO detector
    std::cout << "Loading YOLO model..." << std::endl;
    if (yolo_detector_->load(config.model_path, config.use_gpu) != 0) {
        std::cerr << "Failed to load YOLO model: " << config.model_path << std::endl;
        return false;
    }
    std::cout << "YOLO model loaded successfully" << std::endl;
    
    // Initialize RTSP server
    std::cout << "Initializing RTSP server..." << std::endl;
    if (!rtsp_streamer_->initialize("rtsp://localhost:8554/stream", config.frame_width, config.frame_height, config.frame_fps, config.rtsp_port)) {
        std::cerr << "Failed to initialize RTSP server" << std::endl;
        return false;
    }
    
    if (!rtsp_streamer_->start()) {
        std::cerr << "Failed to start RTSP streamer" << std::endl;
        return false;
    }
    
    // Initialize metadata publisher
    std::cout << "Initializing metadata publisher..." << std::endl;
    if (!metadata_publisher_->initialize(config.metadata_host, config.metadata_port, 
                                        config.metadata_endpoint, config.metadata_publish_interval_ms)) {
        std::cerr << "Failed to initialize metadata publisher" << std::endl;
        return false;
    }
    
    if (!metadata_publisher_->start()) {
        std::cerr << "Failed to start metadata publisher" << std::endl;
        return false;
    }
    
    return true;
}

bool Application::initializeCamera() {
    const auto& config = config_manager_->getConfig();
    
    std::cout << "Initializing camera " << config.camera_id << "..." << std::endl;
    
    camera_ = std::make_unique<cv::VideoCapture>(config.camera_id, cv::CAP_V4L2);
    if (!camera_->isOpened()) {
        std::cout << "Failed with V4L2, trying default backend..." << std::endl;
        camera_->open(config.camera_id);
        if (!camera_->isOpened()) {
            std::cerr << "Failed to open camera " << config.camera_id << std::endl;
            return false;
        }
    }
    
    // Set camera properties for stability
    camera_->set(cv::CAP_PROP_BUFFERSIZE, 1); // Minimal buffer to avoid delays
    camera_->set(cv::CAP_PROP_FRAME_WIDTH, config.frame_width);
    camera_->set(cv::CAP_PROP_FRAME_HEIGHT, config.frame_height);
    camera_->set(cv::CAP_PROP_FPS, config.frame_fps);
    
    // Additional properties for V4L2 stability
    camera_->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    camera_->set(cv::CAP_PROP_AUTO_EXPOSURE, 0.25); // Manual exposure
    
    // Test camera with timeout
    cv::Mat test_frame;
    bool camera_ready = false;
    for (int i = 0; i < 20; i++) { // Increased attempts
        *camera_ >> test_frame;
        if (!test_frame.empty()) {
            std::cout << "Camera initialized successfully" << std::endl;
            std::cout << "Actual frame size: " << test_frame.cols << "x" << test_frame.rows << std::endl;
            camera_ready = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (!camera_ready) {
        std::cerr << "Camera test failed - no frames received" << std::endl;
        return false;
    }
    
    return true;
}

int Application::run() {
    const auto& config = config_manager_->getConfig();
    
    std::cout << "=== Starting AI Detection System ===" << std::endl;
    std::cout << "Press 'q' to quit, 's' for statistics, 'c' to show config" << std::endl;
    
    running_ = true;
    start_time_ = std::chrono::steady_clock::now();
    last_metadata_time_ = start_time_;
    
    cv::Mat frame;
    while (running_) {
        // Capture frame with timeout protection
        bool frame_captured = false;
        for (int retry = 0; retry < 3; retry++) {
            *camera_ >> frame;
            if (!frame.empty()) {
                frame_captured = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (!frame_captured) {
            std::cerr << "Failed to capture frame after retries" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        frame_count_++;
        
        // Process frame
        processFrame(frame);
        
        // Handle keyboard input
        if (config.show_display) {
            try {
                char key = cv::waitKey(1);
                if (key != -1) {
                    handleKeyInput(key);
                }
            } catch (...) {
                std::cerr << "Display error, switching to headless mode" << std::endl;
                // Force headless mode if display fails
                break;
            }
        } else {
            // Small delay when not showing display and check for interruption
            std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30fps timing
        }
        
        // Additional check for running_ flag (helps with signal handling)
        if (!running_) {
            break;
        }
    }
    
    std::cout << "Application stopped" << std::endl;
    return 0;
}

void Application::processFrame(const cv::Mat& frame) {
    const auto& config = config_manager_->getConfig();
    
    // Detect objects
    std::vector<Object> objects;
    yolo_detector_->detect(frame, objects, config.detection_threshold, config.nms_threshold);
    
    if (!objects.empty()) {
        detection_count_ += objects.size();
    }
    
    // Send frame to RTSP stream
    cv::Mat display_frame = frame.clone();
    if (config.draw_detections && !objects.empty()) {
        YoloDetector::draw_objects(display_frame, objects);
    }
    
    rtsp_streamer_->pushFrame(display_frame);
    
    // Publish metadata at configured interval
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_metadata_time_);
    
    if (elapsed.count() >= config.metadata_publish_interval_ms) {
        metadata_publisher_->publishDetections(objects, frame.cols, frame.rows, "camera_" + std::to_string(config.camera_id));
        last_metadata_time_ = now;
    }
    
    // Show display if enabled
    if (config.show_display) {
        // Add status overlay
        std::string status = "Frame: " + std::to_string(frame_count_) + 
                           " | Detections: " + std::to_string(objects.size()) +
                           " | Queue: " + std::to_string(metadata_publisher_->getQueueSize());
        
        cv::putText(display_frame, status, cv::Point(10, 30), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        
        cv::imshow("AI Detection System", display_frame);
    }
}

void Application::handleKeyInput(char key) {
    switch (key) {
        case 'q':
        case 27: // ESC
            std::cout << "Quit requested" << std::endl;
            stop();
            break;
            
        case 's':
            printStatistics();
            break;
            
        case 'c':
            config_manager_->printConfig();
            break;
            
        case 'r':
            // Reset statistics
            frame_count_ = 0;
            detection_count_ = 0;
            start_time_ = std::chrono::steady_clock::now();
            std::cout << "Statistics reset" << std::endl;
            break;
            
        default:
            break;
    }
}

void Application::stop() {
    if (!running_) return;
    
    std::cout << "Stopping application..." << std::endl;
    running_ = false;
    
    // Print final statistics
    printStatistics();
    
    // Cleanup
    if (camera_) {
        camera_->release();
    }
    
    cv::destroyAllWindows();
    
    std::cout << "Application stopped successfully" << std::endl;
}

void Application::printStatistics() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    
    std::cout << "=== Statistics ===" << std::endl;
    std::cout << "Runtime: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Frames processed: " << frame_count_ << std::endl;
    std::cout << "Total detections: " << detection_count_ << std::endl;
    std::cout << "Metadata queue size: " << metadata_publisher_->getQueueSize() << std::endl;
    std::cout << "Metadata published: " << metadata_publisher_->getPublishedCount() << std::endl;
    std::cout << "RTSP streaming: " << (rtsp_streamer_->isRunning() ? "Active" : "Inactive") << std::endl;
    std::cout << "Metadata publisher: " << (metadata_publisher_->isRunning() ? "Active" : "Inactive") << std::endl;
    
    if (elapsed.count() > 0) {
        std::cout << "Average FPS: " << (frame_count_ / elapsed.count()) << std::endl;
        std::cout << "Detections per second: " << (detection_count_ / elapsed.count()) << std::endl;
    }
    
    std::cout << "==================" << std::endl;
}