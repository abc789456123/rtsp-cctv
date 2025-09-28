/**
 * @file ConfigManager.cpp
 * @brief Implementation of configuration management functionality
 * @author AI Detection System
 * @date 2025-09-28
 */

#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ConfigManager::ConfigManager() {
    // Constructor - use default values
}

ConfigManager::~ConfigManager() {
    // Destructor
}

bool ConfigManager::loadConfig(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cout << "Config file not found: " << config_file << std::endl;
        std::cout << "Creating default config file..." << std::endl;
        
        // Create default config file
        std::ofstream outFile(config_file);
        if (outFile.is_open()) {
            outFile << createDefaultConfig();
            outFile.close();
            std::cout << "Default config file created: " << config_file << std::endl;
        }
        return true; // Use default values
    }
    
    // Read entire file
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();
    file.close();
    
    // Parse JSON manually (simple parsing for basic config)
    config_.detection_threshold = parseJsonFloat(json, "detection_threshold", config_.detection_threshold);
    config_.nms_threshold = parseJsonFloat(json, "nms_threshold", config_.nms_threshold);
    
    config_.camera_id = parseJsonInt(json, "camera_id", config_.camera_id);
    config_.frame_width = parseJsonInt(json, "frame_width", config_.frame_width);
    config_.frame_height = parseJsonInt(json, "frame_height", config_.frame_height);
    config_.frame_fps = parseJsonInt(json, "frame_fps", config_.frame_fps);
    
    config_.rtsp_url = parseJsonString(json, "rtsp_url");
    if (config_.rtsp_url.empty()) config_.rtsp_url = "rtsp://localhost:8554/stream";
    config_.rtsp_port = parseJsonInt(json, "rtsp_port", config_.rtsp_port);
    
    config_.metadata_publish_interval_ms = parseJsonInt(json, "metadata_publish_interval_ms", config_.metadata_publish_interval_ms);
    config_.metadata_host = parseJsonString(json, "metadata_host");
    if (config_.metadata_host.empty()) config_.metadata_host = "localhost";
    config_.metadata_port = parseJsonInt(json, "metadata_port", config_.metadata_port);
    config_.metadata_endpoint = parseJsonString(json, "metadata_endpoint");
    if (config_.metadata_endpoint.empty()) config_.metadata_endpoint = "/metadata";
    
    config_.model_path = parseJsonString(json, "model_path");
    if (config_.model_path.empty()) config_.model_path = "ncnn-model/yolov4-tiny";
    config_.use_gpu = parseJsonBool(json, "use_gpu", config_.use_gpu);
    
    config_.show_display = parseJsonBool(json, "show_display", config_.show_display);
    config_.draw_detections = parseJsonBool(json, "draw_detections", config_.draw_detections);
    
    std::cout << "Config loaded from: " << config_file << std::endl;
    return true;
}

bool ConfigManager::saveConfig(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << config_file << std::endl;
        return false;
    }
    
    file << "{\n";
    file << "  \"detection_threshold\": " << config_.detection_threshold << ",\n";
    file << "  \"nms_threshold\": " << config_.nms_threshold << ",\n";
    file << "  \"camera_id\": " << config_.camera_id << ",\n";
    file << "  \"frame_width\": " << config_.frame_width << ",\n";
    file << "  \"frame_height\": " << config_.frame_height << ",\n";
    file << "  \"frame_fps\": " << config_.frame_fps << ",\n";
    file << "  \"rtsp_url\": \"" << config_.rtsp_url << "\",\n";
    file << "  \"rtsp_port\": " << config_.rtsp_port << ",\n";
    file << "  \"metadata_publish_interval_ms\": " << config_.metadata_publish_interval_ms << ",\n";
    file << "  \"metadata_host\": \"" << config_.metadata_host << "\",\n";
    file << "  \"metadata_port\": " << config_.metadata_port << ",\n";
    file << "  \"metadata_endpoint\": \"" << config_.metadata_endpoint << "\",\n";
    file << "  \"model_path\": \"" << config_.model_path << "\",\n";
    file << "  \"use_gpu\": " << (config_.use_gpu ? "true" : "false") << ",\n";
    file << "  \"show_display\": " << (config_.show_display ? "true" : "false") << ",\n";
    file << "  \"draw_detections\": " << (config_.draw_detections ? "true" : "false") << "\n";
    file << "}\n";
    
    file.close();
    std::cout << "Config saved to: " << config_file << std::endl;
    return true;
}

void ConfigManager::printConfig() const {
    std::cout << "=== Current Configuration ===" << std::endl;
    std::cout << "Detection threshold: " << config_.detection_threshold << std::endl;
    std::cout << "NMS threshold: " << config_.nms_threshold << std::endl;
    std::cout << "Camera ID: " << config_.camera_id << std::endl;
    std::cout << "Frame size: " << config_.frame_width << "x" << config_.frame_height << std::endl;
    std::cout << "Frame FPS: " << config_.frame_fps << std::endl;
    std::cout << "RTSP URL: " << config_.rtsp_url << std::endl;
    std::cout << "RTSP Port: " << config_.rtsp_port << std::endl;
    std::cout << "Metadata interval: " << config_.metadata_publish_interval_ms << "ms" << std::endl;
    std::cout << "Metadata host: " << config_.metadata_host << ":" << config_.metadata_port << std::endl;
    std::cout << "Metadata endpoint: " << config_.metadata_endpoint << std::endl;
    std::cout << "Model path: " << config_.model_path << std::endl;
    std::cout << "Use GPU: " << (config_.use_gpu ? "Yes" : "No") << std::endl;
    std::cout << "Show display: " << (config_.show_display ? "Yes" : "No") << std::endl;
    std::cout << "Draw detections: " << (config_.draw_detections ? "Yes" : "No") << std::endl;
    std::cout << "=============================" << std::endl;
}

std::string ConfigManager::parseJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    
    pos = json.find("\"", pos);
    if (pos == std::string::npos) return "";
    pos++; // Skip opening quote
    
    size_t endPos = json.find("\"", pos);
    if (endPos == std::string::npos) return "";
    
    return json.substr(pos, endPos - pos);
}

float ConfigManager::parseJsonFloat(const std::string& json, const std::string& key, float defaultValue) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return defaultValue;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return defaultValue;
    pos++; // Skip colon
    
    // Skip whitespace
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
    
    size_t endPos = pos;
    while (endPos < json.length() && (std::isdigit(json[endPos]) || json[endPos] == '.' || json[endPos] == '-')) endPos++;
    
    if (endPos == pos) return defaultValue;
    
    try {
        return std::stof(json.substr(pos, endPos - pos));
    } catch (...) {
        return defaultValue;
    }
}

int ConfigManager::parseJsonInt(const std::string& json, const std::string& key, int defaultValue) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return defaultValue;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return defaultValue;
    pos++; // Skip colon
    
    // Skip whitespace
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
    
    size_t endPos = pos;
    while (endPos < json.length() && (std::isdigit(json[endPos]) || json[endPos] == '-')) endPos++;
    
    if (endPos == pos) return defaultValue;
    
    try {
        return std::stoi(json.substr(pos, endPos - pos));
    } catch (...) {
        return defaultValue;
    }
}

bool ConfigManager::parseJsonBool(const std::string& json, const std::string& key, bool defaultValue) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return defaultValue;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return defaultValue;
    
    size_t truePos = json.find("true", pos);
    size_t falsePos = json.find("false", pos);
    
    if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos)) {
        return true;
    } else if (falsePos != std::string::npos) {
        return false;
    }
    
    return defaultValue;
}

std::string ConfigManager::createDefaultConfig() {
    return R"({
  "detection_threshold": 0.25,
  "nms_threshold": 0.45,
  "camera_id": 2,
  "frame_width": 640,
  "frame_height": 480,
  "frame_fps": 30,
  "rtsp_url": "rtsp://localhost:8554/stream",
  "rtsp_port": 8554,
  "metadata_publish_interval_ms": 100,
  "metadata_host": "localhost",
  "metadata_port": 8080,
  "metadata_endpoint": "/metadata",
  "model_path": "ncnn-model/yolov4-tiny",
  "use_gpu": false,
  "show_display": true,
  "draw_detections": true
})";
}