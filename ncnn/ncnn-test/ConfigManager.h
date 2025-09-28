/**
 * @file ConfigManager.h
 * @brief Configuration management system for the AI detection application
 * @author AI Detection System
 * @date 2025-09-28
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <fstream>
#include <iostream>

/**
 * @class ConfigManager
 * @brief Manages application configuration through JSON files
 * 
 * This class handles loading, saving, and managing configuration parameters
 * for the AI detection system including thresholds, camera settings, and
 * network configurations.
 */
class ConfigManager {
public:
    /**
     * @struct Config
     * @brief Configuration structure holding all application settings
     */
    struct Config {
        // Detection settings
        float detection_threshold = 0.25f;    ///< Object detection confidence threshold
        float nms_threshold = 0.45f;          ///< Non-maximum suppression threshold
        
        // Camera settings
        int camera_id = 2;
        int frame_width = 640;
        int frame_height = 480;
        int frame_fps = 30;
        
        // RTSP settings
        std::string rtsp_url = "rtsp://localhost:8554/stream";
        int rtsp_port = 8554;
        
        // Metadata settings
        int metadata_publish_interval_ms = 100;  // 100ms = 10Hz
        std::string metadata_host = "localhost";
        int metadata_port = 8080;
        std::string metadata_endpoint = "/metadata";
        
        // Model settings
        std::string model_path = "ncnn-model/yolov4-tiny";
        bool use_gpu = false;
        
        // Display settings
        bool show_display = true;
        bool draw_detections = true;
    };

    ConfigManager();
    ~ConfigManager();
    
    bool loadConfig(const std::string& config_file = "config.json");
    bool saveConfig(const std::string& config_file = "config.json");
    
    const Config& getConfig() const { return config_; }
    Config& getConfig() { return config_; }
    
    void printConfig() const;

private:
    Config config_;
    std::string parseJsonString(const std::string& json, const std::string& key);
    float parseJsonFloat(const std::string& json, const std::string& key, float defaultValue);
    int parseJsonInt(const std::string& json, const std::string& key, int defaultValue);
    bool parseJsonBool(const std::string& json, const std::string& key, bool defaultValue);
    std::string createDefaultConfig();
};

#endif // CONFIG_MANAGER_H