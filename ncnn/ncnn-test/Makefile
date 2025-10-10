CXX = g++
CXXFLAGS = -std=c++14 -O2 -Wall
OPENCV_FLAGS = $(shell pkg-config --cflags opencv4)
OPENCV_LIBS = $(shell pkg-config --libs opencv4)
GSTREAMER_FLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
GSTREAMER_LIBS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0 gstreamer-video-1.0)
INCLUDES = -I/home/park/ncnn/include $(OPENCV_FLAGS) $(GSTREAMER_FLAGS)
LIBS = -L/home/park/ncnn/lib -Wl,-rpath,/home/park/ncnn/lib -Wl,-rpath,/usr/local/lib -lncnn $(OPENCV_LIBS) $(GSTREAMER_LIBS) -pthread -lcurl

# Source files
SOURCES = main.cpp Application.cpp ConfigManager.cpp YoloDetector.cpp RtspStreamer.cpp MetadataPublisher.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = ai_detection_system

# Default target
all: $(TARGET)

# Build main application
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install dependencies
install-deps:
	@echo "Installing required dependencies..."
	sudo apt update
	sudo apt install -y libopencv-dev libncnn-dev libcurl4-openssl-dev

# Create sample config
config:
	@echo "Creating sample config.json..."
	@if [ ! -f config.json ]; then \
		echo '{' > config.json; \
		echo '  "detection_threshold": 0.25,' >> config.json; \
		echo '  "nms_threshold": 0.45,' >> config.json; \
		echo '  "camera_id": 2,' >> config.json; \
		echo '  "frame_width": 640,' >> config.json; \
		echo '  "frame_height": 480,' >> config.json; \
		echo '  "frame_fps": 30,' >> config.json; \
		echo '  "rtsp_url": "rtsp://localhost:8554/stream",' >> config.json; \
		echo '  "rtsp_port": 8554,' >> config.json; \
		echo '  "metadata_publish_interval_ms": 100,' >> config.json; \
		echo '  "metadata_host": "localhost",' >> config.json; \
		echo '  "metadata_port": 8080,' >> config.json; \
		echo '  "metadata_endpoint": "/metadata",' >> config.json; \
		echo '  "model_path": "ncnn-model/yolov4-tiny",' >> config.json; \
		echo '  "use_gpu": false,' >> config.json; \
		echo '  "show_display": true,' >> config.json; \
		echo '  "draw_detections": true' >> config.json; \
		echo '}' >> config.json; \
		echo "Config file created: config.json"; \
	else \
		echo "Config file already exists: config.json"; \
	fi

# Run the application
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Generate documentation using Doxygen
docs:
	@echo "Generating documentation with Doxygen..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in docs/html/"; \
		echo "Open docs/html/index.html in your browser"; \
	else \
		echo "Doxygen not found. Install with: sudo apt install doxygen"; \
	fi

# Clean documentation
clean-docs:
	rm -rf docs/

# Install documentation dependencies
install-docs-deps:
	@echo "Installing Doxygen and documentation dependencies..."
	sudo apt update
	sudo apt install -y doxygen graphviz

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build AI detection system"
	@echo "  $(TARGET) - Build main application"
	@echo "  clean        - Remove built files"
	@echo "  clean-docs   - Remove generated documentation"
	@echo "  install-deps - Install required dependencies"
	@echo "  install-docs-deps - Install documentation dependencies (Doxygen)"
	@echo "  config       - Create sample config.json"
	@echo "  run          - Build and run the application"
	@echo "  debug        - Build with debug symbols"
	@echo "  docs         - Generate API documentation"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Features:"
	@echo "  - Real-time object detection with YOLOv4-tiny"
	@echo "  - RTSP video streaming"
	@echo "  - JSON metadata publishing"
	@echo "  - Configurable thresholds and settings"
	@echo "  - Comprehensive API documentation"
	@echo ""
	@echo "Usage: make && make run"
	@echo "Documentation: make install-docs-deps && make docs"

.PHONY: all clean clean-docs install-deps install-docs-deps config run debug docs help