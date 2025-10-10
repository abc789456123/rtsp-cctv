#!/bin/bash

# FFmpeg을 사용한 간단한 MJPEG HTTP 스트리밍 테스트

echo "=== FFmpeg HTTP MJPEG 스트리밍 테스트 ==="

# FFmpeg이 설치되어 있는지 확인
if ! command -v ffmpeg &> /dev/null; then
    echo "FFmpeg가 설치되어 있지 않습니다."
    echo "설치하려면: sudo apt install ffmpeg"
    exit 1
fi

# 웹캠에서 HTTP MJPEG 스트림 생성
echo "웹캠에서 HTTP MJPEG 스트림을 시작합니다..."
echo "브라우저에서 http://localhost:8554/stream.mjpg 로 접속하세요"
echo "중지하려면 Ctrl+C를 누르세요"

ffmpeg -f v4l2 -i /dev/video2 -c:v mjpeg -q:v 5 -f mjpeg -listen 1 http://0.0.0.0:8554/stream.mjpg