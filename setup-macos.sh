#!/bin/bash

# Cypescript Compiler Setup Script for macOS
# This script installs the necessary dependencies to build and run the Cypescript compiler

set -e  # Exit on any error

echo "🚀 Setting up Cypescript development environment for macOS..."

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "❌ Homebrew not found. Please install Homebrew first:"
    echo "   /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    exit 1
fi

echo "✅ Homebrew found"

# Update Homebrew
echo "📦 Updating Homebrew..."
brew update

# Install CMake
if ! command -v cmake &> /dev/null; then
    echo "📦 Installing CMake..."
    brew install cmake
else
    echo "✅ CMake already installed"
fi

# Install LLVM (this will install a recent version)
echo "📦 Installing LLVM..."
brew install llvm

# Get LLVM version and path
LLVM_VERSION=$(brew list --versions llvm | head -n1 | awk '{print $2}' | cut -d. -f1)
LLVM_PATH=$(brew --prefix llvm)

echo "✅ LLVM ${LLVM_VERSION} installed at ${LLVM_PATH}"

# Add LLVM to PATH for this session
export PATH="${LLVM_PATH}/bin:$PATH"

# Create or update shell profile
SHELL_PROFILE=""
if [[ "$SHELL" == *"zsh"* ]]; then
    SHELL_PROFILE="$HOME/.zshrc"
elif [[ "$SHELL" == *"bash"* ]]; then
    SHELL_PROFILE="$HOME/.bash_profile"
fi

if [[ -n "$SHELL_PROFILE" ]]; then
    echo "📝 Adding LLVM to PATH in $SHELL_PROFILE"
    if ! grep -q "LLVM_PATH" "$SHELL_PROFILE" 2>/dev/null; then
        echo "" >> "$SHELL_PROFILE"
        echo "# LLVM for Cypescript" >> "$SHELL_PROFILE"
        echo "export LLVM_PATH=\"${LLVM_PATH}\"" >> "$SHELL_PROFILE"
        echo "export PATH=\"\$LLVM_PATH/bin:\$PATH\"" >> "$SHELL_PROFILE"
        echo "export LDFLAGS=\"-L\$LLVM_PATH/lib\"" >> "$SHELL_PROFILE"
        echo "export CPPFLAGS=\"-I\$LLVM_PATH/include\"" >> "$SHELL_PROFILE"
    fi
fi

# Verify installations
echo "🔍 Verifying installations..."
echo "CMake version: $(cmake --version | head -n1)"
echo "LLVM version: $(${LLVM_PATH}/bin/llvm-config --version)"
echo "Clang version: $(${LLVM_PATH}/bin/clang --version | head -n1)"

echo ""
echo "✅ Setup complete!"
echo ""
echo "📋 Next steps:"
echo "1. Restart your terminal or run: source $SHELL_PROFILE"
echo "2. Build the project: ./build.sh"
echo "3. Test with: ./test.sh"
echo ""
echo "🔧 LLVM is installed at: ${LLVM_PATH}"
echo "💡 If you encounter issues, make sure LLVM is in your PATH"
