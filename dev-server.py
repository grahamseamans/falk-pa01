#!/usr/bin/env python3
"""
Simple development server for testing the web interface locally
without needing to flash the ESP32 every time.
"""

import http.server
import socketserver
import json
import os
from urllib.parse import urlparse, parse_qs

class DevHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory="data/www", **kwargs)
    
    def do_GET(self):
        parsed = urlparse(self.path)
        
        # Mock API endpoints
        if parsed.path == '/api/status':
            self.send_mock_status()
        elif parsed.path == '/events':
            self.send_mock_events()  
        elif parsed.path == '/api/networks':
            self.send_mock_networks()
        else:
            # Serve static files
            super().do_GET()
    
    def do_POST(self):
        parsed = urlparse(self.path)
        
        # Mock API endpoints
        if parsed.path == '/api/volume':
            self.send_mock_response({'status': 'ok'})
        elif parsed.path == '/api/input': 
            self.send_mock_response({'status': 'ok'})
        elif parsed.path.startswith('/api/settings/'):
            self.send_mock_response({'status': 'ok'})
        elif parsed.path == '/update':
            self.send_mock_response({'status': 'ok', 'message': 'Firmware updated'})
        else:
            self.send_response(404)
            self.end_headers()
    
    def send_mock_status(self):
        mock_data = {
            "volume": {
                "current": 32,
                "max": 63
            },
            "inputs": [
                {"id": 1, "name": "Input 1", "enabled": True, "selected": True},
                {"id": 2, "name": "Input 2", "enabled": True, "selected": False},
                {"id": 3, "name": "Input 3", "enabled": True, "selected": False},
                {"id": 4, "name": "Input 4", "enabled": False, "selected": False}
            ],
            "settings": {
                "dim": False,
                "absoluteVol": True
            }
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(mock_data).encode())
    
    def send_mock_events(self):
        # Simple mock for EventSource
        self.send_response(200)
        self.send_header('Content-type', 'text/event-stream')
        self.send_header('Cache-Control', 'no-cache')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(b'data: {"volume": 32}\n\n')
    
    def send_mock_networks(self):
        self.send_mock_response({'status': 'scanning'})
    
    def send_mock_response(self, data):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

if __name__ == "__main__":
    PORT = 8000
    
    with socketserver.TCPServer(("", PORT), DevHandler) as httpd:
        print(f"Development server running at http://localhost:{PORT}")
        print("Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\\nShutting down server...")
            httpd.shutdown()