#!/bin/bash

# Simple RTSP Connection Test
echo "=== Simple RTSP Test ==="

# Test with timeout and verbose output
echo "Testing RTSP connection with 10 second timeout..."
timeout 10 ffmpeg -rtsp_transport tcp -i rtsp://localhost:8554/stream -t 5 -f null - -v info 2>&1 | head -20

echo ""
echo "Testing UDP transport..."
timeout 10 ffmpeg -rtsp_transport udp -i rtsp://localhost:8554/stream -t 5 -f null - -v info 2>&1 | head -20

echo ""
echo "Testing with VLC command line..."
timeout 10 cvlc rtsp://localhost:8554/stream --intf dummy --play-and-exit --stop-time=5 2>/dev/null &
sleep 3
echo "VLC test completed"