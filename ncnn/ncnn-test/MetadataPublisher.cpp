/**
 * @file MetadataPublisher.cpp
 * @brief Implementation of JSON metadata publishing system
 * @author AI Detection System
 * @date 2025-09-28
 */

#include "MetadataPublisher.h"
#include "YoloDetector.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <curl/curl.h>

/**
 * @brief COCO dataset class names for object classification
 */
const char* MetadataPublisher::class_names_[] = {
    "background", "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", 
    "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", 
    "cow", "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", 
    "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

/**
 * @brief Callback function for libcurl to handle HTTP response data
 * @param contents Pointer to received data
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param userp User-defined pointer (string to store response)
 * @return Total number of bytes processed
 */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

MetadataPublisher::MetadataPublisher() 
    : running_(false), initialized_(false), published_count_(0) {
}

MetadataPublisher::~MetadataPublisher() {
    stop();
}

bool MetadataPublisher::initialize(const std::string& host, int port, const std::string& endpoint, int publish_interval_ms) {
    host_ = host;
    port_ = port;
    endpoint_ = endpoint;
    publish_interval_ms_ = publish_interval_ms;
    
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    initialized_ = true;
    std::cout << "Metadata Publisher initialized: " << host_ << ":" << port_ << endpoint_ << std::endl;
    std::cout << "Publish interval: " << publish_interval_ms_ << "ms" << std::endl;
    
    return true;
}

bool MetadataPublisher::start() {
    if (!initialized_) {
        std::cerr << "Metadata Publisher not initialized" << std::endl;
        return false;
    }
    
    if (running_) {
        std::cout << "Metadata Publisher already running" << std::endl;
        return true;
    }
    
    running_ = true;
    publisher_thread_ = std::thread(&MetadataPublisher::publishingLoop, this);
    
    std::cout << "Metadata Publisher started" << std::endl;
    return true;
}

void MetadataPublisher::stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (publisher_thread_.joinable()) {
        publisher_thread_.join();
    }
    
    // Cleanup libcurl
    curl_global_cleanup();
    
    std::cout << "Metadata Publisher stopped" << std::endl;
}

void MetadataPublisher::publishDetections(const std::vector<Object>& objects, int frame_width, int frame_height, const std::string& camera_id) {
    if (!running_) return;
    
    DetectionMetadata metadata;
    metadata.timestamp = std::chrono::system_clock::now();
    metadata.objects = objects;
    metadata.frame_width = frame_width;
    metadata.frame_height = frame_height;
    metadata.camera_id = camera_id;
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    metadata_queue_.push(metadata);
    
    // Limit queue size to prevent memory issues
    if (metadata_queue_.size() > 100) {
        metadata_queue_.pop();
    }
}

std::string MetadataPublisher::createJsonMetadata(const std::vector<Object>& objects, int frame_width, int frame_height, const std::string& camera_id) {
    DetectionMetadata metadata;
    metadata.timestamp = std::chrono::system_clock::now();
    metadata.objects = objects;
    metadata.frame_width = frame_width;
    metadata.frame_height = frame_height;
    metadata.camera_id = camera_id;
    
    return createJsonMetadata(metadata);
}

int MetadataPublisher::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return metadata_queue_.size();
}

void MetadataPublisher::publishingLoop() {
    while (running_) {
        DetectionMetadata metadata;
        bool has_metadata = false;
        
        // Get metadata from queue
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!metadata_queue_.empty()) {
                metadata = metadata_queue_.front();
                metadata_queue_.pop();
                has_metadata = true;
            }
        }
        
        if (has_metadata) {
            // Create JSON and send
            std::string json_data = createJsonMetadata(metadata);
            
            // For debugging, print to console instead of HTTP POST
            // metadata 출력하는 코드인데 일단 로그 확인하려고 주석처리함
            //std::cout << "=== Metadata JSON ===" << std::endl;
            //std::cout << json_data << std::endl;
            //std::cout << "===================" << std::endl;
            
            // Attempt HTTP POST (will fail if no server, but that's OK for demo)
            if (sendHttpPost(json_data)) {
                published_count_++;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(publish_interval_ms_));
    }
}

std::string MetadataPublisher::createJsonMetadata(const DetectionMetadata& metadata) {
    std::stringstream json;
    
    json << "{\n";
    json << "  \"timestamp\": \"" << formatTimestamp(metadata.timestamp) << "\",\n";
    json << "  \"camera_id\": \"" << metadata.camera_id << "\",\n";
    json << "  \"frame_width\": " << metadata.frame_width << ",\n";
    json << "  \"frame_height\": " << metadata.frame_height << ",\n";
    json << "  \"detections\": [\n";
    
    for (size_t i = 0; i < metadata.objects.size(); i++) {
        const Object& obj = metadata.objects[i];
        
        json << "    {\n";
        json << "      \"class_id\": " << obj.label << ",\n";
        json << "      \"class_name\": \"" << getClassNameForLabel(obj.label) << "\",\n";
        json << "      \"confidence\": " << std::fixed << std::setprecision(4) << obj.prob << ",\n";
        json << "      \"bbox\": {\n";
        json << "        \"x\": " << std::fixed << std::setprecision(2) << obj.rect.x << ",\n";
        json << "        \"y\": " << std::fixed << std::setprecision(2) << obj.rect.y << ",\n";
        json << "        \"width\": " << std::fixed << std::setprecision(2) << obj.rect.width << ",\n";
        json << "        \"height\": " << std::fixed << std::setprecision(2) << obj.rect.height << "\n";
        json << "      }\n";
        json << "    }";
        
        if (i < metadata.objects.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ],\n";
    json << "  \"detection_count\": " << metadata.objects.size() << "\n";
    json << "}";
    
    return json.str();
}

bool MetadataPublisher::sendHttpPost(const std::string& json_data) {
    CURL *curl;
    CURLcode res;
    std::string response_data;
    bool success = false;
    
    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://" + host_ + ":" + std::to_string(port_) + endpoint_;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5 second timeout
        
        // Set content type header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200) {
                success = true;
            } else {
                std::cout << "HTTP POST failed with response code: " << response_code << std::endl;
            }
        } else {
            // Don't spam error messages - server might not be running
            // std::cerr << "HTTP POST failed: " << curl_easy_strerror(res) << std::endl;
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return success;
}

std::string MetadataPublisher::formatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    
    return ss.str();
}

std::string MetadataPublisher::getClassNameForLabel(int label) {
    if (label >= 0 && label < 80) {
        return std::string(class_names_[label]);
    }
    return "unknown";
}