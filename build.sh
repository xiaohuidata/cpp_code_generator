#!/bin/bash

# 构建脚本 - 支持Windows/Linux/Mac
set -e

# 检测操作系统
OS="$(uname -s)"
case "${OS}" in
    Linux*)     MACHINE=Linux;;
    Darwin*)    MACHINE=Mac;;
    CYGWIN*)    MACHINE=Cygwin;;
    MINGW*)     MACHINE=MinGW;;
    MSYS*)      MACHINE=Msys;;
    Windows*)   MACHINE=Windows;;
    *)          MACHINE="UNKNOWN:${OS}"
esac

# 默认设置
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_DIR="${1:-./install}"
GENERATOR=""
USE_CONFIGURE=0
CLEAN_BUILD=0
VERBOSE=0
THREADS=1
BUILD_SHARED="ON"
BUILD_STATIC="ON"
BOOST_ROOT=""
CMAKE_CXX_FLAGS=""
CMAKE_GENERATOR=""
CMAKE_EXTRA_ARGS=""
CXX_COMPILER=""

# 获取CPU核心数
if command -v nproc > /dev/null 2>&1; then
    THREADS=$(nproc)
elif command -v sysctl > /dev/null 2>&1; then
    THREADS=$(sysctl -n hw.ncpu)
elif [[ "$MACHINE" == "Windows" ]] && command -v wmic > /dev/null 2>&1; then
    THREADS=$(wmic cpu get NumberOfCores /value | grep NumberOfCores | awk -F'=' '{sum += $2} END {print sum}')
else
    THREADS=4
fi

# 帮助信息
show_help() {
    echo "Usage: $0 [OPTIONS] [INSTALL_DIR]"
    echo ""
    echo "Build script for CppCodeGenerator"
    echo ""
    echo "OPTIONS:"
    echo "  -h, --help           Show this help message"
    echo "  -d, --debug          Build in debug mode"
    echo "  -r, --release        Build in release mode (default)"
    echo "  -c, --configure      Use autotools build system"
    echo "  -m, --cmake          Use CMake build system (default)"
    echo "  -g, --generator GEN  CMake generator (e.g., 'Visual Studio 16 2019')"
    echo "  -C, --clean          Clean build directory before building"
    echo "  -j, --jobs N         Number of parallel jobs (default: auto)"
    echo "  -v, --verbose        Verbose output"
    echo "  --msvc               Use MSVC compiler (Windows)"
    echo "  --mingw              Use MinGW compiler (Windows)"
    echo ""
    echo "EXAMPLES:"
    echo "  $0                   # Build with CMake in release mode"
    echo "  $0 -d                # Build with CMake in debug mode"
    echo "  $0 -c                # Build with autotools"
    echo "  $0 -g \"Visual Studio 16 2019\"  # Use specific CMake generator"
    echo "  $0 -C -d             # Clean and debug build"
    echo ""
}

# 日志函数
log_info() {
    if [[ "$VERBOSE" == "1" ]]; then
        echo "[INFO] $1"
    else
        echo "$1"
    fi
}

log_error() {
    echo "[ERROR] $1" >&2
}

log_success() {
    echo "[SUCCESS] $1"
}

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--configure)
            USE_CONFIGURE=1
            shift
            ;;
        -m|--cmake)
            USE_CONFIGURE=0
            shift
            ;;
        -g|--generator)
            GENERATOR="$2"
            shift 2
            ;;
        -C|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -j|--jobs)
            THREADS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        --msvc)
            if [[ "$MACHINE" == "Windows" ]]; then
                GENERATOR="Visual Studio 16 2019"
                export CC=cl
                export CXX=cl
            else
                log_error "MSVC is only available on Windows"
                exit 1
            fi
            shift
            ;;
        --mingw)
            if [[ "$MACHINE" == "Windows" ]]; then
                export CC=gcc
                export CXX=g++
            else
                log_error "MinGW is only available on Windows"
                exit 1
            fi
            shift
            ;;
        --shared-only)
            BUILD_SHARED="ON"
            BUILD_STATIC="OFF"
            shift
            ;;
        --static-only)
            BUILD_SHARED="OFF"
            BUILD_STATIC="ON"
            shift
            ;;
        -b|--boost)
            BOOST_ROOT="$2"
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -*)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
        *)
            INSTALL_DIR="$1"
            shift
            ;;
    esac
done

# 验证安装目录
if [[ -z "$INSTALL_DIR" ]]; then
    INSTALL_DIR="./install"
fi

# 输出配置信息
log_info "========================================="
log_info "CppCodeGenerator Build Configuration"
log_info "========================================="
log_info "Operating System: $MACHINE"
log_info "Build Type: $BUILD_TYPE"
log_info "Install Directory: $INSTALL_DIR"
log_info "Parallel Jobs: $THREADS"

if [[ -n "$GENERATOR" ]]; then
    log_info "CMake Generator: $GENERATOR"
fi

if [[ "$USE_CONFIGURE" == "1" ]]; then
    log_info "Build System: Autotools"
else
    log_info "Build System: CMake"
fi

log_info "========================================="

# 清理构建目录
if [[ "$CLEAN_BUILD" == "1" ]] && [[ -d "$BUILD_DIR" ]]; then
    log_info "Cleaning build directory: $BUILD_DIR"
    case "$MACHINE" in
        Windows|Cygwin|MinGW|Msys)
            rm -rf "$BUILD_DIR"
            ;;
        *)
            rm -rf "$BUILD_DIR"
            ;;
    esac
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"

# Boost库查找函数
find_boost() {
    local boost_root=""
    
    # 常见的Boost安装路径
    case "$MACHINE" in
        Windows)
            # Windows常见的Boost路径
            if [[ -d "C:/boost" ]]; then
                boost_root="C:/boost"
            elif [[ -d "C:/local/boost" ]]; then
                boost_root="C:/local/boost"
            elif [[ -d "/c/boost" ]]; then
                boost_root="/c/boost"
            elif [[ -d "/c/local/boost" ]]; then
                boost_root="/c/local/boost"
            fi
            ;;
        Linux)
            # Linux常见的Boost路径
            if [[ -d "/usr/include/boost" ]]; then
                boost_root="/usr"
            elif [[ -d "/usr/local/include/boost" ]]; then
                boost_root="/usr/local"
            fi
            ;;
        Mac)
            # macOS常见的Boost路径
            if [[ -d "/usr/local/include/boost" ]]; then
                boost_root="/usr/local"
            elif [[ -d "/opt/homebrew/include/boost" ]]; then
                boost_root="/opt/homebrew"
            fi
            ;;
    esac
    
    echo "$boost_root"
}

# 检查依赖
check_dependencies() {
    log_info "Checking dependencies..."
    
    # 检查CMake
    if ! command -v cmake > /dev/null 2>&1; then
        log_error "CMake not found. Please install CMake 3.16 or later."
        exit 1
    fi
    
    # 检查编译器
    local compiler=""
    if [[ "$MACHINE" == "Windows" ]]; then
        if command -v cl > /dev/null 2>&1; then
            compiler="MSVC"
        elif command -v g++ > /dev/null 2>&1; then
            compiler="MinGW"
        else
            log_error "No C++ compiler found. Please install MSVC or MinGW."
            exit 1
        fi
    else
        if command -v g++ > /dev/null 2>&1; then
            compiler="GCC"
        elif command -v clang++ > /dev/null 2>&1; then
            compiler="Clang"
        else
            log_error "No C++ compiler found. Please install GCC or Clang."
            exit 1
        fi
    fi
    
    log_info "Found C++ compiler: $compiler"
    
    # 检查Boost
    local boost_root=$(find_boost)
    if [[ -z "$boost_root" ]]; then
        log_error "Boost libraries not found. Please install Boost 1.75.0 or later."
        exit 1
    fi
    
    log_info "Found Boost at: $boost_root"
}

# Autotools构建
build_with_autotools() {
    log_info "Building with Autotools..."
    
    #cd "$BUILD_DIR"
    
    # 生成configure脚本
    if [[ ! -f "configure" ]]; then
        log_info "Generating configure script..."
        autoreconf -fi
    fi
    
    # 配置
    local configure_args="--prefix=$(realpath "$INSTALL_DIR")"
    
    # 库类型选择
    if [[ "$BUILD_SHARED" == "ON" && "$BUILD_STATIC" == "ON" ]]; then
        configure_args="$configure_args --enable-shared-libs --enable-static-libs"
    elif [[ "$BUILD_SHARED" == "ON" ]]; then
        configure_args="$configure_args --enable-shared-libs --disable-static-libs"
    elif [[ "$BUILD_STATIC" == "ON" ]]; then
        configure_args="$configure_args --disable-shared-libs --enable-static-libs"
    else
        log_error "至少需要启用一种库类型"
        exit 1
    fi
    
    # Boost路径
    if [[ -n "$BOOST_ROOT" ]]; then
        export BOOST_ROOT="$BOOST_ROOT"
    fi
    
    case "$MACHINE" in
        Windows)
            configure_args="$configure_args CXXFLAGS='-std=c++11 -Wall -Wextra'"
            ;;
        *)
            configure_args="$configure_args CXXFLAGS='-std=c++11 -Wall -Wextra -pedantic'"
            ;;
    esac
    
    log_info "Running configure..."
    eval "./configure $configure_args"
    
    # 构建
    log_info "Building..."
    make -j"$THREADS"
    
    # 安装
    log_info "Installing..."
    make install
    
    log_success "Autotools build completed successfully!"
}

# CMake构建
build_with_cmake() {
    log_info "Building with CMake..."
    
    cd "$BUILD_DIR"
    
    # CMake命令
    local cmake_cmd="cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
    
    # 设置生成器
    if [[ -n "$GENERATOR" ]]; then
        CMAKE_GENERATOR="$GENERATOR"
    elif [[ "$MACHINE" == "Windows" ]]; then
        # Windows默认使用Ninja或Visual Studio
        if command -v ninja > /dev/null 2>&1; then
            CMAKE_GENERATOR="Ninja"
        else
            CMAKE_GENERATOR="Visual Studio 16 2019"
        fi
    fi
    
    # 设置编译器
    if [[ "$MACHINE" == "Windows" ]]; then
        if command -v cl > /dev/null 2>&1; then
            CXX_COMPILER="cl"
        elif command -v g++ > /dev/null 2>&1; then
            CXX_COMPILER="g++"
        fi
    else
        if command -v g++ > /dev/null 2>&1; then
            CXX_COMPILER="g++"
        elif command -v clang++ > /dev/null 2>&1; then
            CXX_COMPILER="clang++"
        fi
    fi
    
    # Boost路径
    if [[ -z "$BOOST_ROOT" ]]; then
        BOOST_ROOT=$(find_boost)
    fi
    
    log_info "Running CMake..."
    
    # 运行CMake配置
    local cmake_args=""
    cmake_args="$cmake_args -DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    cmake_args="$cmake_args -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
    
    if [[ -n "$CXX_COMPILER" ]]; then
        cmake_args="$cmake_args -DCMAKE_CXX_COMPILER=$CXX_COMPILER"
    fi
    
    if [[ -n "$CMAKE_CXX_FLAGS" ]]; then
        cmake_args="$cmake_args -DCMAKE_CXX_FLAGS=$CMAKE_CXX_FLAGS"
    fi
    
    if [[ -n "$BOOST_ROOT" ]]; then
        cmake_args="$cmake_args -DBOOST_ROOT=$BOOST_ROOT"
    fi
    
    cmake_args="$cmake_args -DBUILD_SHARED_LIBS=$BUILD_SHARED"
    cmake_args="$cmake_args -DBUILD_STATIC_LIBS=$BUILD_STATIC"
    
    if [[ -n "$CMAKE_GENERATOR" ]]; then
        cmake_args="$cmake_args -G \"$CMAKE_GENERATOR\""
    fi
    
    if [[ -n "$CMAKE_EXTRA_ARGS" ]]; then
        cmake_args="$cmake_args $CMAKE_EXTRA_ARGS"
    fi
    
    log_info "CMake arguments: $cmake_args"
    
    eval "cmake .. $cmake_args"
    
    # 构建
    log_info "Building..."
    if [[ "$MACHINE" == "Windows" ]]; then
        cmake --build . --config $BUILD_TYPE --parallel $THREADS
        cmake --install . --config $BUILD_TYPE
    else
        cmake --build . --parallel $THREADS
        cmake --install . 
    fi
    
    log_success "CMake build completed successfully!"
}

# 主构建逻辑
main() {
    check_dependencies
    
    if [[ "$USE_CONFIGURE" == "1" ]]; then
        build_with_autotools
    else
        build_with_cmake
    fi
    
    # 显示安装信息
    log_success "========================================="
    log_success "Build completed successfully!"
    log_success "Install directory: $INSTALL_DIR"
    log_success "Executable: $INSTALL_DIR/bin/cpp_code_generator"
    log_success "========================================="
}

# 运行主函数
main "$@"

echo "Build completed successfully!"
echo "Executable installed to: $INSTALL_DIR/bin/cpp_code_generator"
