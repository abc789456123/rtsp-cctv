/**
 * @file RtspStreamer.h
 * @brief Simple GStreamer RTSP Server implementation
 * @author AI Detection System
 * @date 2025-10-10
 */

#ifndef RTSP_STREAMER_H
#define RTSP_STREAMER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <chrono>
#include <vector>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

/**
 * @class RtspStreamer
 * @brief Simple RTSP Server using GStreamer MediaFactory
 * 
 * Creates RTSP streams using UDP sink -> RTSP server pattern
 * - rtsp://localhost:8554/stream - Video stream
 * - rtsp://localhost:8554/metadata - Metadata stream (optional)
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
     * @brief Initialize RTSP server with parameters
     * @param rtsp_url Not used (legacy compatibility)
     * @param width Frame width in pixels
     * @param height Frame height in pixels
     * @param fps Frames per second
     * @param port RTSP server port (optional, default 8554)
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& rtsp_url, int width, int height, int fps, int port = 8554);
    
    /**
     * @brief Start the RTSP server
     * @return true if server started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the RTSP server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return server_running_; }
    
    /**
     * @brief Push a video frame (uses UDP sink to GStreamer pipeline)
     * @param frame OpenCV Mat frame to stream
     * @return true if frame pushed successfully, false otherwise
     */
    bool pushFrame(const cv::Mat& frame);
    
    /**
     * @brief Get the video stream URL
     * @return Video RTSP URL string
     */
    std::string getStreamUrl() const;

private:
    int port_;
    int width_;
    int height_;
    int fps_;
    
    // GStreamer RTSP Server components
    GstRTSPServer* server_;
    GMainLoop* loop_;
    std::thread server_thread_;
    
    // App source for frame injection - support multiple clients
    std::vector<GstElement*> appsrc_list_;
    GstRTSPMediaFactory* factory_;
    
    std::atomic<bool> server_running_;
    std::atomic<bool> initialized_;
    std::mutex frame_mutex_;
    std::mutex appsrc_mutex_;
    
    // Private methods
    bool setupRtspServer();
    void serverLoop();
    
    // GStreamer callback functions
    static void onMediaConstructed(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data);
    static void onNeedData(GstElement* appsrc, guint unused_size, gpointer user_data);
    static void onEnoughData(GstElement* appsrc, gpointer user_data);
    static void onMediaPrepared(GstRTSPMedia* media, gpointer user_data);
    static void onMediaUnprepared(GstRTSPMedia* media, gpointer user_data);
};

#endif // RTSP_STREAMER_H