#!/usr/bin/env python3
"""
Simple HTTP server for serving Cypescript documentation
Usage: python3 serve.py [port]
"""

import http.server
import socketserver
import os
import sys
import webbrowser
from pathlib import Path

def main():
    # Get port from command line argument or use default
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    
    # Change to docs directory
    docs_dir = Path(__file__).parent
    os.chdir(docs_dir)
    
    # Create server
    handler = http.server.SimpleHTTPRequestHandler
    
    # Add CORS headers for local development
    class CORSRequestHandler(handler):
        def end_headers(self):
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
            self.send_header('Access-Control-Allow-Headers', 'Content-Type')
            super().end_headers()
    
    with socketserver.TCPServer(("", port), CORSRequestHandler) as httpd:
        print(f"🚀 Cypescript Documentation Server")
        print(f"📖 Serving at: http://localhost:{port}")
        print(f"📁 Directory: {docs_dir}")
        print(f"🔗 Opening browser...")
        print(f"⏹️  Press Ctrl+C to stop")
        
        # Open browser
        webbrowser.open(f'http://localhost:{port}')
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print(f"\n👋 Server stopped")

if __name__ == "__main__":
    main()
