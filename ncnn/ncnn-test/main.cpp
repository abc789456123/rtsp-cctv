/**
 * @file main.cpp
 * @brief Main entry point for AI Detection System with RTSP streaming
 * @author AI Detection System
 * @date 2025-09-28
 * 
 * This application provides:
 * - Real-time object detection using YOLOv4-tiny
 * - RTSP video streaming with detection overlays
 * - JSON metadata publishing via HTTP POST
 * - Configurable detection thresholds and network settings
 */

#include "Application.h"
#include <iostream>
#include <signal.h>

/// Global application instance for signal handling
Application* g_app = nullptr;

/**
 * @brief Signal handler for graceful application shutdown
 * @param signal Signal number received (SIGINT, SIGTERM, etc.)
 */
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    if (g_app) {
        g_app->stop();
    }
}

/**
 * @brief Main entry point for the AI Detection System
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Exit code (0 for success, -1 for failure)
 * 
 * Usage: ./ai_detection_system [config_file.json]
 * If no config file is specified, "config.json" is used by default.
 */
int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== AI Detection System with RTSP Streaming ===" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "- Real-time object detection using YOLOv4-tiny" << std::endl;
    std::cout << "- RTSP video streaming" << std::endl;
    std::cout << "- JSON metadata publishing" << std::endl;
    std::cout << "- Configurable thresholds and settings" << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Create application
    Application app;
    g_app = &app;
    
    // Determine config file
    std::string config_file = "config.json";
    if (argc > 1) {
        config_file = argv[1];
        std::cout << "Using config file: " << config_file << std::endl;
    }
    
    // Initialize application
    if (!app.initialize(config_file)) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    // Run application
    int result = app.run();
    
    g_app = nullptr;
    
    std::cout << "Application exited with code: " << result << std::endl;
    return result;
}