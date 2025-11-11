#!/bin/bash

set -e

BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_DIR="${1:-./install}"
GENERATOR=""

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--configure)
            USE_CONFIGURE=1
            shift
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        --install-dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "Building CppCodeGenerator..."
echo "Build type: $BUILD_TYPE"
echo "Install directory: $INSTALL_DIR"

if [[ $USE_CONFIGURE -eq 1 ]]; then
    # 使用Autotools构建
    echo "Using Autotools build system..."
    
    if [ ! -f "configure" ]; then
        echo "Generating configure script..."
        autoreconf -fi
    fi
    
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    
    ../configure --prefix=$(realpath $INSTALL_DIR) \
                 CXXFLAGS="-O2 -Wall -Wextra -pedantic"
    
    make -j$(nproc)
    make install

elif [[ $USE_CONFIGURE -eq 2 ]]; then
    # g++直接构建
    echo "Using gcc build system..."
    g++ -std=c++11 -g -fpermissive  -o cod_generator  ./src/*.cpp -Iinclude -Iinclude/code_generator  -lboost_json -lboost_program_options -lboost_filesystem
    
else
    # 使用CMake构建
    echo "Using CMake build system..."
    
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    
    CMAKE_CMD="cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
    
    if [ ! -z "$GENERATOR" ]; then
        CMAKE_CMD="$CMAKE_CMD -G \"$GENERATOR\""
    fi
    
    eval $CMAKE_CMD
    cmake --build . --config $BUILD_TYPE --parallel $(nproc)
    cmake --install . --config $BUILD_TYPE
fi

echo "Build completed successfully!"
echo "Executable installed to: $INSTALL_DIR/bin/cpp_code_generator"
