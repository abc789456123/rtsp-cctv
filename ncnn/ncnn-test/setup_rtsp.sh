#!/bin/bash

# RTSP 서버 설정 및 실행 스크립트
# MediaMTX (RTSP Simple Server)를 사용한 RTSP 스트리밍

echo "=== AI Detection System RTSP Setup ==="

# MediaMTX 다운로드 및 설정
setup_mediamtx() {
    echo "Setting up MediaMTX RTSP server..."
    
    # MediaMTX 다운로드 (최신 버전)
    if [ ! -f "mediamtx" ]; then
        echo "Downloading MediaMTX..."
        wget https://github.com/bluenviron/mediamtx/releases/latest/download/mediamtx_v1.3.1_linux_amd64.tar.gz
        tar -xzf mediamtx_v1.3.1_linux_amd64.tar.gz
        chmod +x mediamtx
        echo "MediaMTX downloaded successfully"
    fi
    
    # 설정 파일 생성
    cat > mediamtx.yml << EOF
# MediaMTX configuration for AI Detection System

# RTSP server settings
rtspAddress: :8554
protocols: [tcp, udp]
encryption: "no"
serverKey: server.key
serverCert: server.crt

# Path settings for stream
paths:
  stream:
    source: publisher
    publishUser: ""
    publishPass: ""
    readUser: ""
    readPass: ""

# Logging
logLevel: info
logDestinations: [stdout]

# Performance settings
readTimeout: 10s
writeTimeout: 10s
readBufferCount: 512
EOF
    
    echo "MediaMTX configuration created"
}

# FFmpeg 기반 RTSP 서버 설정
setup_ffmpeg_server() {
    echo "Setting up FFmpeg RTSP server..."
    
    # FFmpeg 설치 확인
    if ! command -v ffmpeg &> /dev/null; then
        echo "Installing FFmpeg..."
        sudo apt update
        sudo apt install -y ffmpeg
    fi
    
    echo "FFmpeg is ready"
}

# 선택한 방법에 따라 설정
case "$1" in
    "mediamtx"|"")
        setup_mediamtx
        echo ""
        echo "To start MediaMTX RTSP server:"
        echo "./mediamtx"
        echo ""
        echo "Then run your AI detection system:"
        echo "./ai_detection_system"
        echo ""
        echo "Connect with VLC: rtsp://localhost:8554/stream"
        ;;
    "ffmpeg")
        setup_ffmpeg_server
        echo ""
        echo "FFmpeg setup complete."
        echo "The AI detection system will start its own RTSP server."
        ;;
    *)
        echo "Usage: $0 [mediamtx|ffmpeg]"
        echo ""
        echo "Methods:"
        echo "  mediamtx - Use MediaMTX as external RTSP server (recommended)"
        echo "  ffmpeg   - Use FFmpeg built-in RTSP server"
        exit 1
        ;;
esac

echo "=== Setup Complete ==="