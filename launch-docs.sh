#!/bin/bash

# Cypescript Documentation Launcher
# Starts the web documentation server and opens it in the browser

echo "🚀 Starting Cypescript Documentation Server..."

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is required but not installed."
    echo "Please install Python 3 and try again."
    exit 1
fi

# Navigate to docs directory
cd "$(dirname "$0")/docs"

# Check if docs directory exists
if [ ! -d "." ]; then
    echo "❌ Documentation directory not found."
    exit 1
fi

# Find an available port
PORT=8080
while lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null ; do
    PORT=$((PORT + 1))
done

echo "📖 Documentation will be available at: http://localhost:$PORT"
echo "🔗 Opening browser..."
echo "⏹️  Press Ctrl+C to stop the server"
echo ""

# Start the server
python3 serve.py $PORT
