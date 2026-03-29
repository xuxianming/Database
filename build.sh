#!/bin/bash

set -e  # 遇到错误立即退出

# 默认配置
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
BUILD_TYPE="Release"
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
SKIP_INSTALL=1   # 0=安装, 1=不安装

# 显示帮助信息
usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

Options:
  -c                Clean the build directory, then build but do NOT install
  -h                Show this help message
  -b <dir>          Build directory (default: build)
  -p <prefix>       Install prefix (default: /usr/local)
  -t <type>         Build type: Debug/Release (default: Release)
  -j <jobs>         Number of parallel jobs (default: auto)

Examples:
  $0                Build and install
  $0 -c             Clean, build, but skip installation
  $0 -b build_release -t Release
EOF
    exit 0
}

# 解析命令行参数
while getopts "chb:p:t:j:" opt; do
    case $opt in
        c) CLEAN_ONLY=0    # 注意：我们不是只清理，而是清理后继续
           SKIP_INSTALL=1 ;;
        h) usage ;;
        b) BUILD_DIR="$OPTARG" ;;
        p) INSTALL_PREFIX="$OPTARG" ;;
        t) BUILD_TYPE="$OPTARG" ;;
        j) JOBS="$OPTARG" ;;
        *) usage ;;
    esac
done

# 清理构建目录
if [ -n "$CLEAN_ONLY" ] && [ -d "$BUILD_DIR" ]; then
    echo "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
elif [ -n "$CLEAN_ONLY" ]; then
    echo "Build directory $BUILD_DIR does not exist; skipping clean."
fi

# 正常编译流程（不管是否 clean，都会重新创建构建目录并编译）
echo "=== Build configuration ==="
echo "Build directory: $BUILD_DIR"
echo "Install prefix: $INSTALL_PREFIX"
echo "Build type: $BUILD_TYPE"
echo "Parallel jobs: $JOBS"
if [ $SKIP_INSTALL -eq 1 ]; then
    echo "Installation: SKIPPED"
else
    echo "Installation: ENABLED"
fi

# 创建构建目录（如果之前被删除了，现在重新创建）
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake 配置
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 编译
echo "Building with $JOBS jobs..."
cmake --build . --parallel "$JOBS"

# 可选：运行测试
# ctest --output-on-failure

# 安装（仅在 SKIP_INSTALL=0 时执行）
if [ $SKIP_INSTALL -eq 0 ]; then
    echo "Installing to $INSTALL_PREFIX..."
    # 如果安装目录需要 root 权限，自动使用 sudo
    if [ -w "$INSTALL_PREFIX" ] || [ "$INSTALL_PREFIX" == "/usr/local" ]; then
        sudo cmake --install .
    else
        cmake --install .
    fi
else
    echo "Skipping installation (as requested by -c)."
fi

echo "=== Build completed successfully ==="