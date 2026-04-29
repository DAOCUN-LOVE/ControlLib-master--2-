#!/bin/bash

# Motor Speed Curve GUI - 编译脚本

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo "Motor Speed Curve GUI - 编译工具"
echo "=========================================="

# 检查Qt5
if ! command -v qmake &> /dev/null; then
    echo "❌ 错误：找不到 qmake，请先安装 Qt5"
    echo "   Ubuntu/Debian: sudo apt-get install qt5-qmake qtbase5-dev"
    exit 1
fi

echo "✓ 已找到 Qt5"

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    echo "📁 创建构建目录..."
    mkdir -p "$BUILD_DIR"
fi

# 进入构建目录
cd "$BUILD_DIR"

# 运行CMake
echo "🔨 运行 CMake..."
cmake ..

# 编译
echo "🔨 编译源代码..."
make -j$(nproc)

echo ""
echo "=========================================="
echo "✓ 编译成功！"
echo "=========================================="
echo ""
echo "运行应用：$BUILD_DIR/SpeedCurveGUI"
