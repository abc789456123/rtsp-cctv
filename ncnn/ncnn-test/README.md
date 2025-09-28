# AI Detection System with RTSP Streaming

실시간 객체 감지 시스템으로 RTSP 스트리밍과 JSON 메타데이터를 지원합니다.

## 주요 기능

- **실시간 객체 감지**: YOLOv4-tiny 모델을 사용한 고속 객체 감지
- **RTSP 비디오 스트리밍**: 감지 결과가 오버레이된 비디오를 RTSP로 스트리밍
- **JSON 메타데이터 전송**: 감지된 객체 정보를 JSON 형식으로 HTTP POST 전송  
- **설정 가능한 파라미터**: JSON 설정 파일을 통한 threshold, 전송 주기 등 설정
- **클래스 기반 아키텍처**: 확장 가능한 모듈식 설계

## 시스템 구조

```
├── Application.h/cpp      - 메인 애플리케이션 클래스
├── ConfigManager.h/cpp    - JSON 설정 파일 관리
├── YoloDetector.h/cpp     - YOLO 객체 감지 엔진
├── RtspStreamer.h/cpp     - RTSP 비디오 스트리밍
├── MetadataPublisher.h/cpp - JSON 메타데이터 전송
├── main.cpp               - 진입점
└── config.json            - 설정 파일
```

## 빌드 및 설치

### 의존성 설치
```bash
make install-deps
```

### 빌드
```bash
make
```

### 설정 파일 생성
```bash
make config
```

## 설정 파일 (config.json)

```json
{
  "detection_threshold": 0.25,        // 객체 감지 임계값
  "nms_threshold": 0.45,              // NMS 임계값
  "camera_id": 2,                     // 카메라 ID
  "frame_width": 640,                 // 프레임 너비
  "frame_height": 480,                // 프레임 높이
  "frame_fps": 30,                    // 프레임 레이트
  "rtsp_url": "rtsp://localhost:8554/stream", // RTSP 스트림 URL
  "rtsp_port": 8554,                  // RTSP 포트
  "metadata_publish_interval_ms": 100, // 메타데이터 전송 주기 (ms)
  "metadata_host": "localhost",       // 메타데이터 서버 호스트
  "metadata_port": 8080,              // 메타데이터 서버 포트
  "metadata_endpoint": "/metadata",   // 메타데이터 엔드포인트
  "model_path": "ncnn-model/yolov4-tiny", // YOLO 모델 경로
  "use_gpu": false,                   // GPU 사용 여부
  "show_display": true,               // 디스플레이 창 표시
  "draw_detections": true             // 감지 결과 그리기
}
```

## 실행

### 기본 실행
```bash
make run
```

### 사용자 정의 설정 파일로 실행
```bash
./ai_detection_system my_config.json
```

## 키보드 단축키

- **q** 또는 **ESC**: 프로그램 종료
- **s**: 통계 정보 출력
- **c**: 현재 설정 출력
- **r**: 통계 초기화

## JSON 메타데이터 형식

```json
{
  "timestamp": "2025-09-28T12:34:56.789Z",
  "camera_id": "camera_2",
  "frame_width": 640,
  "frame_height": 480,
  "detections": [
    {
      "class_id": 1,
      "class_name": "person",
      "confidence": 0.8945,
      "bbox": {
        "x": 100.50,
        "y": 150.25,
        "width": 120.75,
        "height": 200.00
      }
    }
  ],
  "detection_count": 1
}
```

## RTSP 스트림 접속

VLC 플레이어나 다른 RTSP 클라이언트에서 다음 URL로 접속:
```
rtsp://localhost:8554/stream
```

## 메타데이터 서버 예제

Python Flask 서버 예제:
```python
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/metadata', methods=['POST'])
def receive_metadata():
    data = request.get_json()
    print(f"Received {data['detection_count']} detections at {data['timestamp']}")
    return jsonify({"status": "ok"})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
```

## 확장 가능성

이 시스템은 모듈식으로 설계되어 다음과 같은 확장이 용이합니다:

1. **새로운 감지 모델**: `YoloDetector` 클래스를 상속하여 다른 모델 지원
2. **다양한 스트리밍 프로토콜**: `RtspStreamer`를 확장하여 WebRTC, HLS 등 지원
3. **메타데이터 전송 방식**: `MetadataPublisher`를 확장하여 MQTT, WebSocket 등 지원
4. **추가 센서 데이터**: `Application` 클래스에 온도, 습도 등 센서 데이터 통합

## 문제 해결

### 카메라 접근 오류
```bash
# 카메라 권한 확인
ls -la /dev/video*
sudo usermod -a -G video $USER
```

### RTSP 스트리밍 문제
- FFmpeg 설치 확인: `ffmpeg -version`
- 방화벽 설정 확인: RTSP 포트(8554) 개방
- GStreamer 설치: `sudo apt install gstreamer1.0-tools`

### 의존성 문제
```bash
# OpenCV 설치 확인
pkg-config --modversion opencv4

# NCNN 라이브러리 확인
ls -la /home/park/ncnn/lib/

# libcurl 설치
sudo apt install libcurl4-openssl-dev
```

## 라이센스

이 프로젝트는 MIT 라이센스를 따릅니다.