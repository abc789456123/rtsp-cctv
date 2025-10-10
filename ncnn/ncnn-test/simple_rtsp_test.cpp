/**
 * @file SimpleRtspStreamer.cpp
 * @brief Ultra-simple RTSP Server for testing
 */

#include <iostream>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

static gboolean timeout_handler(gpointer user_data) {
    return TRUE;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    
    gst_init(&argc, &argv);
    
    loop = g_main_loop_new(NULL, FALSE);
    
    // Create RTSP server
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_address(server, "0.0.0.0");
    gst_rtsp_server_set_service(server, "8556");
    
    // Get mount points
    mounts = gst_rtsp_server_get_mount_points(server);
    
    // Create factory
    factory = gst_rtsp_media_factory_new();
    
    // Simple test pattern
    gst_rtsp_media_factory_set_launch(factory,
        "( videotestsrc pattern=0 ! "
        "video/x-raw,width=640,height=480,framerate=15/1 ! "
        "videoconvert ! "
        "x264enc tune=zerolatency speed-preset=ultrafast bitrate=1000 ! "
        "rtph264pay name=pay0 pt=96 )");
    
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_media_factory_set_protocols(factory, GST_RTSP_LOWER_TRANS_TCP);
    
    // Mount
    gst_rtsp_mount_points_add_factory(mounts, "/simple", factory);
    g_object_unref(mounts);
    
    // Attach server
    gst_rtsp_server_attach(server, NULL);
    
    g_timeout_add_seconds(2, timeout_handler, NULL);
    
    std::cout << "Simple RTSP Server started on port 8556" << std::endl;
    std::cout << "URL: rtsp://localhost:8556/simple" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    g_main_loop_run(loop);
    
    return 0;
}