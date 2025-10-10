# AI Detection System with RTSP Streaming

실시간 객체 감지 시스템으로 RTSP 스트리밍과 JSON 메타데이터를 지원합니다.

## 개발 진행사항 기록

openCV의 프레임을 rtsp로 전달하기 때문에 렉이 걸리고 화질이 크게 감소하는 문제가 발생함

yolo모델이 적용된 프레임이 아니라 원 영상을 보내는 식으로 개선해야할 것 같음

추가로 metadata 파싱 테스트도 아직 하지 않았음

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


## 설정 파일 (config.json)

```json
{
  "detection_threshold": 0.25,
  "nms_threshold": 0.45,
  "camera_id": 2,
  "frame_width": 1280,
  "frame_height": 720,
  "frame_fps": 30,
  "rtsp_url": "rtsp://localhost:8554/stream",
  "rtsp_port": 8554,
  "metadata_publish_interval_ms": 100,
  "metadata_host": "localhost",
  "metadata_port": 8080,
  "metadata_endpoint": "/metadata",
  "model_path": "ncnn-model/yolov4-tiny",
  "use_gpu": false,
  "show_display": false,
  "draw_detections": true
}
```

## 실행

### 기본 실행
```bash
make
```

### 사용자 정의 설정 파일로 실행
```bash
./ai_detection_system config.json
```


## JSON 메타데이터 형식

```json
{
  "timestamp": "2025-09-28T12:34:56.789Z",
  "camera_id": "camera_2",
  "frame_width": 1280,
  "frame_height": 720,
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


## 라이센스

이 프로젝트는 MIT 라이센스를 따릅니다.