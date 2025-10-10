/**
 * @file RtspStreamer.cpp
 * @brief Direct GStreamer RTSP Server (similar to your reference code)
 * @author AI Detection System
 * @date 2025-10-10
 */

#include "RtspStreamer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>

RtspStreamer::RtspStreamer() 
    : port_(8554), width_(640), height_(480), fps_(30),
      server_(nullptr), loop_(nullptr),
      factory_(nullptr),
      server_running_(false), initialized_(false) {
    // Default values will be overridden in initialize() method with config values
}

RtspStreamer::~RtspStreamer() {
    stop();
}

bool RtspStreamer::initialize(const std::string& rtsp_url, int width, int height, int fps, int port) {
    width_ = width;
    height_ = height;
    fps_ = fps;
    port_ = port;
    
    // Initialize GStreamer
    gst_init(nullptr, nullptr);
    
    if (!setupRtspServer()) {
        return false;
    }
    
    initialized_ = true;
    std::cout << "Direct RTSP Server initialized" << std::endl;
    std::cout << "Resolution: " << width_ << "x" << height_ << " @ " << fps_ << "fps" << std::endl;
    std::cout << "RTSP URL: " << getStreamUrl() << std::endl;
    
    return true;
}

bool RtspStreamer::setupRtspServer() {
    // Create RTSP server
    server_ = gst_rtsp_server_new();
    if (!server_) {
        std::cerr << "Failed to create RTSP server" << std::endl;
        return false;
    }
    
    // Set server address and port - same as working simple_rtsp_test
    gst_rtsp_server_set_address(server_, "0.0.0.0");  // Bind to all interfaces like simple_rtsp_test
    gst_rtsp_server_set_service(server_, std::to_string(port_).c_str());
    
    // Get mount points
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server_);
    
    // Create media factory for appsrc input
    factory_ = gst_rtsp_media_factory_new();
    
    // Pipeline similar to simple_rtsp_test but with appsrc instead of videotestsrc
    std::string pipeline_description = 
        "( appsrc name=mysrc is-live=true "
        "caps=video/x-raw,format=BGR,width=" + std::to_string(width_) + 
        ",height=" + std::to_string(height_) + 
        ",framerate=" + std::to_string(fps_) + "/1 ! "
        "videoconvert ! "
        "x264enc tune=zerolatency speed-preset=ultrafast bitrate=1000 ! "
        "rtph264pay name=pay0 pt=96 )";
    
    std::cout << "RTSP Pipeline: " << pipeline_description << std::endl;
    
    gst_rtsp_media_factory_set_launch(factory_, pipeline_description.c_str());
    
    // Match simple_rtsp_test settings exactly
    gst_rtsp_media_factory_set_shared(factory_, TRUE);  // Share pipeline like simple_rtsp_test
    gst_rtsp_media_factory_set_protocols(factory_, GST_RTSP_LOWER_TRANS_TCP);  // TCP only like simple_rtsp_test
    
    // Connect media constructed signal to get appsrc element
    g_signal_connect(factory_, "media-constructed", G_CALLBACK(onMediaConstructed), this);
    
    // Mount the stream
    gst_rtsp_mount_points_add_factory(mounts, "/stream", factory_);
    g_object_unref(mounts);
    
    return true;
}

bool RtspStreamer::start() {
    if (!initialized_) {
        std::cerr << "RTSP server not initialized" << std::endl;
        return false;
    }
    
    // Create main loop first
    loop_ = g_main_loop_new(NULL, FALSE);
    if (!loop_) {
        std::cerr << "Failed to create GMainLoop" << std::endl;
        return false;
    }
    
    // Start server thread BEFORE attaching
    server_running_ = true;
    server_thread_ = std::thread(&RtspStreamer::serverLoop, this);
    
    // Give server thread time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "RTSP server thread started" << std::endl;
    
    std::cout << "Direct RTSP Server started successfully" << std::endl;
    std::cout << "Stream URL: " << getStreamUrl() << std::endl;
    std::cout << "VLC: Media > Open Network Stream > " << getStreamUrl() << std::endl;
    
    return true;
}

void RtspStreamer::stop() {
    if (server_running_) {
        server_running_ = false;
        
        // Stop main loop
        if (loop_) {
            g_main_loop_quit(loop_);
        }
        
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        
        // Cleanup appsrc list
        {
            std::lock_guard<std::mutex> lock(appsrc_mutex_);
            for (auto appsrc : appsrc_list_) {
                if (appsrc) {
                    gst_object_unref(appsrc);
                }
            }
            appsrc_list_.clear();
        }
        
        if (loop_) {
            g_main_loop_unref(loop_);
            loop_ = nullptr;
        }
        
        if (server_) {
            g_object_unref(server_);
            server_ = nullptr;
        }
        
        std::cout << "Direct RTSP Server stopped" << std::endl;
    }
}

void RtspStreamer::serverLoop() {
    if (!loop_) {
        std::cerr << "No main loop available" << std::endl;
        return;
    }
    
    // Create main context for this thread
    GMainContext* context = g_main_context_default();
    
    // Attach server to main context within the thread
    guint server_id = gst_rtsp_server_attach(server_, context);
    if (server_id == 0) {
        std::cerr << "Failed to attach RTSP server in thread" << std::endl;
        return;
    }
    
    std::cout << "RTSP server attached successfully to port " << port_ << std::endl;
    std::cout << "RTSP server loop started" << std::endl;
    server_running_ = true;
    
    // Run the main loop (this will block until quit)
    g_main_loop_run(loop_);
    
    std::cout << "RTSP server loop ended" << std::endl;
}

bool RtspStreamer::pushFrame(const cv::Mat& frame) {
    static int frame_count = 0;
    static int last_log_count = 0;
    static int successful_pushes = 0;
    
    if (!server_running_) {
        return false;
    }
    
    // Get the shared appsrc
    GstElement* current_appsrc = nullptr;
    {
        std::lock_guard<std::mutex> lock(appsrc_mutex_);
        if (appsrc_list_.empty()) {
            if (frame_count % 100 == 0) { // Log every 100 frames when no clients
                std::cout << "[RTSP DEBUG] Waiting for RTSP client connection (frames queued: " << frame_count << ")" << std::endl;
            }
            frame_count++;
            return true; // No clients connected
        }
        
        current_appsrc = appsrc_list_[0];  // Use first (and only) appsrc
        gst_object_ref(current_appsrc);
    }
    
    // Resize frame if necessary (outside of mutex for better performance)
    cv::Mat resized_frame;
    if (frame.cols != width_ || frame.rows != height_) {
        cv::resize(frame, resized_frame, cv::Size(width_, height_));
    } else {
        resized_frame = frame;
    }
    
    // Ensure BGR format (OpenCV default)
    cv::Mat bgr_frame;
    if (resized_frame.channels() == 4) {
        cv::cvtColor(resized_frame, bgr_frame, cv::COLOR_BGRA2BGR);
    } else if (resized_frame.channels() == 1) {
        cv::cvtColor(resized_frame, bgr_frame, cv::COLOR_GRAY2BGR);
    } else {
        bgr_frame = resized_frame;
    }
    
    // Validate frame data
    if (bgr_frame.empty() || bgr_frame.data == nullptr) {
        std::cerr << "[RTSP ERROR] Invalid frame data" << std::endl;
        gst_object_unref(current_appsrc);
        return false;
    }
    
    // Create GstBuffer from OpenCV Mat
    gsize buffer_size = bgr_frame.total() * bgr_frame.elemSize();
    GstBuffer* buffer = gst_buffer_new_allocate(NULL, buffer_size, NULL);
    
    if (!buffer) {
        std::cerr << "Failed to allocate GstBuffer" << std::endl;
        gst_object_unref(current_appsrc);
        return false;
    }
    
    // Copy OpenCV data to GstBuffer
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
        memcpy(map.data, bgr_frame.data, buffer_size);
        gst_buffer_unmap(buffer, &map);
    } else {
        gst_buffer_unref(buffer);
        std::cerr << "Failed to map GstBuffer" << std::endl;
        gst_object_unref(current_appsrc);
        return false;
    }
    
    // Set buffer timestamp
    static GstClockTime timestamp = 0;
    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, fps_);
    timestamp += GST_BUFFER_DURATION(buffer);
    
    // Push buffer to shared appsrc
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(current_appsrc), buffer);
    
    frame_count++;
    
    if (ret == GST_FLOW_OK) {
        successful_pushes++;
    }
    
    // Log frame push status periodically
    if (frame_count - last_log_count >= 30) { // Log every 30 frames
        std::cout << "[RTSP DEBUG] Pushed " << frame_count << " frames (" << successful_pushes << " successful) (flow: ";
        switch (ret) {
            case GST_FLOW_OK:
                std::cout << "OK)";
                break;
            case GST_FLOW_NOT_LINKED:
                std::cout << "NOT_LINKED)";
                break;
            case GST_FLOW_FLUSHING:
                std::cout << "FLUSHING)";
                break;
            default:
                std::cout << "ERROR " << ret << ")";
                break;
        }
        std::cout << std::endl;
        last_log_count = frame_count;
    }
    
    // Release appsrc reference
    gst_object_unref(current_appsrc);
    
    return (ret == GST_FLOW_OK || ret == GST_FLOW_NOT_LINKED);
}

std::string RtspStreamer::getStreamUrl() const {
    return "rtsp://localhost:" + std::to_string(port_) + "/stream";
}

// Static callback functions
void RtspStreamer::onMediaConstructed(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data) {
    RtspStreamer* streamer = static_cast<RtspStreamer*>(user_data);
    
    std::cout << "[RTSP DEBUG] Client connected, constructing media pipeline..." << std::endl;
    
    // Get the pipeline element
    GstElement* element = gst_rtsp_media_get_element(media);
    if (!element) {
        std::cerr << "[RTSP ERROR] Failed to get media element" << std::endl;
        return;
    }
    
    // Find the appsrc element by name
    GstElement* new_appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");
    if (!new_appsrc) {
        std::cerr << "[RTSP ERROR] Failed to get appsrc element" << std::endl;
        gst_object_unref(element);
        return;
    }
    
    // For shared pipeline, we only need one appsrc
    {
        std::lock_guard<std::mutex> lock(streamer->appsrc_mutex_);
        if (streamer->appsrc_list_.empty()) {
            streamer->appsrc_list_.push_back(new_appsrc);
            std::cout << "[RTSP DEBUG] First appsrc added to shared pipeline" << std::endl;
        } else {
            gst_object_unref(new_appsrc);  // Release extra reference
            std::cout << "[RTSP DEBUG] Using existing shared appsrc" << std::endl;
        }
    }
    
    std::cout << "[RTSP DEBUG] appsrc element found successfully" << std::endl;
    
    // Simple appsrc configuration
    g_object_set(G_OBJECT(new_appsrc),
                 "is-live", TRUE,
                 "format", GST_FORMAT_TIME,
                 "do-timestamp", TRUE,
                 NULL);
    
    // Connect need-data signal (optional, for flow control)
    g_signal_connect(new_appsrc, "need-data", G_CALLBACK(onNeedData), user_data);
    g_signal_connect(new_appsrc, "enough-data", G_CALLBACK(onEnoughData), user_data);
    
    // Connect media signals for better debugging
    g_signal_connect(media, "prepared", G_CALLBACK(onMediaPrepared), user_data);
    g_signal_connect(media, "unprepared", G_CALLBACK(onMediaUnprepared), user_data);
    
    std::cout << "[RTSP DEBUG] Media constructed, appsrc configured and ready for frames" << std::endl;
    
    gst_object_unref(element);
}

void RtspStreamer::onNeedData(GstElement* appsrc, guint unused_size, gpointer user_data) {
    // This callback is called when the appsrc needs more data
    std::cout << "[RTSP DEBUG] Client requesting data (need-data signal)" << std::endl;
}

void RtspStreamer::onEnoughData(GstElement* appsrc, gpointer user_data) {
    // This callback is called when the appsrc has enough data
    std::cout << "[RTSP DEBUG] Client has enough data (enough-data signal)" << std::endl;
}

void RtspStreamer::onMediaPrepared(GstRTSPMedia* media, gpointer user_data) {
    std::cout << "[RTSP DEBUG] Media pipeline prepared and ready" << std::endl;
}

void RtspStreamer::onMediaUnprepared(GstRTSPMedia* media, gpointer user_data) {
    RtspStreamer* streamer = static_cast<RtspStreamer*>(user_data);
    std::cout << "[RTSP DEBUG] Media pipeline unprepared (client disconnected)" << std::endl;
    
    // Remove disconnected appsrc elements from list
    GstElement* element = gst_rtsp_media_get_element(media);
    if (element) {
        GstElement* appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");
        if (appsrc) {
            std::lock_guard<std::mutex> lock(streamer->appsrc_mutex_);
            auto it = std::find(streamer->appsrc_list_.begin(), streamer->appsrc_list_.end(), appsrc);
            if (it != streamer->appsrc_list_.end()) {
                gst_object_unref(*it);
                streamer->appsrc_list_.erase(it);
                std::cout << "[RTSP DEBUG] Removed appsrc from list, " << streamer->appsrc_list_.size() << " clients remaining" << std::endl;
            }
            gst_object_unref(appsrc);
        }
        gst_object_unref(element);
    }
}