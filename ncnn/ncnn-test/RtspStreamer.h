/**
 * @file RtspStreamer.h
 * @brief RTSP video streaming implementation
 * @author AI Detection System
 * @date 2025-09-28
 */

#ifndef RTSP_STREAMER_H
#define RTSP_STREAMER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <memory>

/**
 * @class RtspStreamer
 * @brief Handles RTSP video streaming of processed frames
 * 
 * This class manages RTSP video streaming using OpenCV VideoWriter
 * and can stream detection results in real-time.
 */
class RtspStreamer {
public:
    /**
     * @brief Default constructor
     */
    RtspStreamer();
    
    /**
     * @brief Destructor
     */
    ~RtspStreamer();
    
    /**
     * @brief Initialize RTSP streamer with parameters
     * @param rtsp_url Target RTSP stream URL
     * @param width Frame width in pixels
     * @param height Frame height in pixels
     * @param fps Frames per second
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& rtsp_url, int width, int height, int fps);
    
    /**
     * @brief Start the RTSP streaming
     * @return true if streaming started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the RTSP streaming
     */
    void stop();
    
    /**
     * @brief Check if streaming is active
     * @return true if streaming, false otherwise
     */
    bool isStreaming() const { return streaming_; }
    
    /**
     * @brief Push a frame to the RTSP stream
     * @param frame OpenCV Mat frame to stream
     * @return true if frame pushed successfully, false otherwise
     */
    bool pushFrame(const cv::Mat& frame);
    
    /**
     * @brief Get the current stream URL
     * @return RTSP URL string
     */
    std::string getStreamUrl() const { return rtsp_url_; }

private:
    std::string rtsp_url_;
    int width_;
    int height_;
    int fps_;
    
    std::unique_ptr<cv::VideoWriter> writer_;
    std::atomic<bool> streaming_;
    std::atomic<bool> initialized_;
    
    // FFmpeg command for RTSP streaming
    std::string buildFFmpegCommand();
    bool startFFmpegProcess();
    void stopFFmpegProcess();
    
    FILE* ffmpeg_process_;
    std::thread streaming_thread_;
    cv::Mat current_frame_;
    std::mutex frame_mutex_;
    
    void streamingLoop();
};

#endif // RTSP_STREAMER_H