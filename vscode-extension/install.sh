#!/bin/bash

# Cypescript VSCode Extension Installation Script

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}🚀 Installing Cypescript VSCode Extension${NC}"

# Check if we're in the right directory
if [[ ! -f "package.json" ]]; then
    echo -e "${RED}❌ Error: package.json not found. Please run this script from the vscode-extension directory.${NC}"
    exit 1
fi

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo -e "${RED}❌ Node.js is not installed. Please install Node.js first.${NC}"
    exit 1
fi

# Check if npm is installed
if ! command -v npm &> /dev/null; then
    echo -e "${RED}❌ npm is not installed. Please install npm first.${NC}"
    exit 1
fi

echo -e "${BLUE}📦 Installing dependencies...${NC}"
npm install

echo -e "${BLUE}🔨 Compiling TypeScript...${NC}"
npm run compile

echo -e "${BLUE}📋 Checking if vsce is installed...${NC}"
if ! command -v vsce &> /dev/null; then
    echo -e "${YELLOW}⚠️  vsce not found. Installing globally...${NC}"
    npm install -g vsce
fi

echo -e "${BLUE}📦 Packaging extension...${NC}"
vsce package

# Find the generated .vsix file
VSIX_FILE=$(ls *.vsix 2>/dev/null | head -n 1)

if [[ -z "$VSIX_FILE" ]]; then
    echo -e "${RED}❌ Error: No .vsix file found after packaging.${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Extension packaged successfully: $VSIX_FILE${NC}"

# Check if code command is available
if command -v code &> /dev/null; then
    echo -e "${BLUE}🔧 Installing extension in VSCode...${NC}"
    code --install-extension "$VSIX_FILE"
    echo -e "${GREEN}✅ Extension installed successfully!${NC}"
    
    echo -e "${BLUE}🎉 Installation complete!${NC}"
    echo -e "${YELLOW}📝 To use the extension:${NC}"
    echo -e "   1. Open VSCode"
    echo -e "   2. Open a Cypescript (.csc) file"
    echo -e "   3. Enjoy syntax highlighting and IntelliSense!"
    echo -e "   4. Use Ctrl+F5 to compile and run"
    
    # Ask if user wants to open a test file
    read -p "Would you like to open a test Cypescript file? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        if [[ -f "../example/basic/hello.csc" ]]; then
            code "../example/basic/hello.csc"
        else
            echo -e "${YELLOW}⚠️  Test file not found. Please open a .csc file manually.${NC}"
        fi
    fi
else
    echo -e "${YELLOW}⚠️  VSCode 'code' command not found in PATH.${NC}"
    echo -e "${BLUE}📝 To install the extension manually:${NC}"
    echo -e "   1. Open VSCode"
    echo -e "   2. Press Ctrl+Shift+P (Cmd+Shift+P on Mac)"
    echo -e "   3. Type 'Extensions: Install from VSIX'"
    echo -e "   4. Select the file: $VSIX_FILE"
fi

echo -e "${GREEN}🎊 Done!${NC}"
