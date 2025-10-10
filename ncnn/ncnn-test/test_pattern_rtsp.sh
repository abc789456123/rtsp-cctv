#!/bin/bash

# Test RTSP server with test pattern
echo "Starting test RTSP server with videotestsrc..."

gst-launch-1.0 -v videotestsrc pattern=0 ! \
    video/x-raw,width=640,height=480,framerate=30/1 ! \
    videoconvert ! \
    x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast profile=baseline key-int-max=30 ! \
    rtph264pay pt=96 config-interval=1 ! \
    udpsink host=127.0.0.1 port=5004 &

TEST_PID=$!
echo "Test server PID: $TEST_PID"

# Start RTSP server to serve the UDP stream
gst-rtsp-server-1.0 \
    --port=8555 \
    --mount-point=/test \
    --gst-debug=3 \
    "( udpsrc port=5004 caps=\"application/x-rtp,media=video,payload=96,clock-rate=90000,encoding-name=H264\" ! rtph264depay ! rtph264pay name=pay0 pt=96 )" &

RTSP_PID=$!
echo "RTSP server PID: $RTSP_PID"
echo ""
echo "Test RTSP URL: rtsp://localhost:8555/test"
echo "Press any key to stop..."
read -n 1

kill $TEST_PID $RTSP_PID 2>/dev/null
echo "Test servers stopped"