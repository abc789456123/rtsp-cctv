#!/bin/bash

# RTSP Stream Test Script
echo "=== RTSP Stream Test ==="
echo "Testing RTSP server connectivity..."

# Test 1: Check if port is open
echo "1. Checking if RTSP port 8554 is listening..."
if netstat -an | grep -q ":8554.*LISTEN"; then
    echo "✓ Port 8554 is listening"
else
    echo "✗ Port 8554 is not listening"
fi

# Test 2: Try to connect with FFprobe
echo "2. Testing RTSP stream with FFprobe..."
timeout 10 ffprobe -v quiet -print_format json -show_streams rtsp://localhost:8554/stream 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ FFprobe can connect to RTSP stream"
else
    echo "✗ FFprobe cannot connect to RTSP stream"
fi

# Test 3: Try to grab a frame with FFmpeg
echo "3. Testing frame capture with FFmpeg..."
timeout 5 ffmpeg -i rtsp://localhost:8554/stream -vframes 1 -f image2 /tmp/test_frame.jpg -y 2>/dev/null
if [ $? -eq 0 ] && [ -f /tmp/test_frame.jpg ]; then
    echo "✓ Successfully captured a frame"
    ls -la /tmp/test_frame.jpg
    rm -f /tmp/test_frame.jpg
else
    echo "✗ Failed to capture frame"
fi

# Test 4: Check GStreamer pipeline
echo "4. Testing with GStreamer..."
timeout 5 gst-launch-1.0 -v rtspsrc location=rtsp://localhost:8554/stream ! fakesink 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ GStreamer can connect"
else
    echo "✗ GStreamer cannot connect"
fi

echo "=== Test Complete ==="