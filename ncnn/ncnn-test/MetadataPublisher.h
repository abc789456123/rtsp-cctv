/**
 * @file MetadataPublisher.h
 * @brief JSON metadata publishing system for detection results
 * @author AI Detection System
 * @date 2025-09-28
 */

#ifndef METADATA_PUBLISHER_H
#define METADATA_PUBLISHER_H

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <chrono>
#include <opencv2/opencv.hpp>

// Forward declaration for Object struct
struct Object;

/**
 * @struct DetectionMetadata
 * @brief Container for detection metadata with timestamp and camera info
 */
struct DetectionMetadata {
    std::chrono::system_clock::time_point timestamp; ///< Detection timestamp
    std::vector<Object> objects;                     ///< Detected objects list
    int frame_width;                                 ///< Frame width in pixels
    int frame_height;                                ///< Frame height in pixels
    std::string camera_id;                           ///< Camera identifier
};

/**
 * @class MetadataPublisher
 * @brief Publishes detection metadata in JSON format via HTTP POST
 * 
 * This class manages a background thread that periodically sends detection
 * results as JSON metadata to a configured HTTP endpoint.
 */
class MetadataPublisher {
public:
    /**
     * @brief Default constructor
     */
    MetadataPublisher();
    
    /**
     * @brief Destructor
     */
    ~MetadataPublisher();
    
    /**
     * @brief Initialize metadata publisher with network settings
     * @param host Target HTTP server hostname
     * @param port Target HTTP server port
     * @param endpoint HTTP endpoint path for metadata
     * @param publish_interval_ms Publishing interval in milliseconds
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& host, int port, const std::string& endpoint, int publish_interval_ms);
    
    /**
     * @brief Start the metadata publishing thread
     * @return true if started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the metadata publishing thread
     */
    void stop();
    
    /**
     * @brief Check if publisher is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_; }
    
    /**
     * @brief Add detection data to publishing queue
     * @param objects Vector of detected objects
     * @param frame_width Frame width in pixels
     * @param frame_height Frame height in pixels
     * @param camera_id Camera identifier string
     */
    void publishDetections(const std::vector<Object>& objects, int frame_width, int frame_height, const std::string& camera_id = "camera_0");
    
    /**
     * @brief Get current queue size
     * @return Number of items in publishing queue
     */
    int getQueueSize() const;
    
    /**
     * @brief Get total published count
     * @return Number of successfully published metadata items
     */
    int getPublishedCount() const { return published_count_; }

private:
    std::string host_;
    int port_;
    std::string endpoint_;
    int publish_interval_ms_;
    
    std::atomic<bool> running_;
    std::atomic<bool> initialized_;
    std::atomic<int> published_count_;
    
    std::thread publisher_thread_;
    std::queue<DetectionMetadata> metadata_queue_;
    mutable std::mutex queue_mutex_;
    
    // Publishing methods
    void publishingLoop();
    std::string createJsonMetadata(const DetectionMetadata& metadata);
    bool sendHttpPost(const std::string& json_data);
    
    // Utility methods
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);
    std::string getClassNameForLabel(int label);
    
    // Class names (same as in main.cpp)
    static const char* class_names_[];
};

#endif // METADATA_PUBLISHER_H