#!/usr/bin/env python3
"""
Simple RTSP test server using GStreamer
"""

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import Gst, GstRtspServer, GLib
import sys

def main():
    Gst.init(None)
    
    server = GstRtspServer.RTSPServer()
    server.set_address("0.0.0.0")
    server.set_service("8555")
    
    mounts = server.get_mount_points()
    factory = GstRtspServer.RTSPMediaFactory()
    
    # Simple test pattern pipeline
    factory.set_launch("( videotestsrc pattern=0 ! video/x-raw,width=640,height=480,framerate=15/1 ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay name=pay0 pt=96 )")
    factory.set_shared(True)
    
    mounts.add_factory("/test", factory)
    
    server.attach(None)
    
    print("RTSP Test Server started")
    print("URL: rtsp://localhost:8555/test")
    print("Press Ctrl+C to stop")
    
    try:
        loop = GLib.MainLoop()
        loop.run()
    except KeyboardInterrupt:
        print("\nServer stopped")

if __name__ == '__main__':
    main()