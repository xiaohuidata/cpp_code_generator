cmake_minimum_required(VERSION 3.12)
project(CppCodeGenerator VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找Boost库
find_package(Boost 1.65.0 REQUIRED COMPONENTS filesystem program_options json)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# 源代码文件
set(SOURCES
    src/zero_copy_stream.cpp
    src/file_streams.cpp
    src/formatter.cpp
    src/cpp_generator.cpp
    src/config_parser.cpp
    src/enhanced_cpp_generator.cpp
    src/stream_adapters.cpp
)

set(MAIN_SOURCES
    ${SOURCES}
    src/main.cpp
)

# 头文件
set(HEADERS
    include/code_generator/zero_copy_stream.h
    include/code_generator/file_streams.h
    include/code_generator/formatter.h
    include/code_generator/cpp_generator.h
    include/code_generator/config_parser.h
    include/code_generator/enhanced_cpp_generator.h
    include/code_generator/stream_adapters.h
)

set(MAIN_HEADERS
    ${HEADERS}
    include/code_generator.h
)

add_library(cpp_code_generator_lib STATIC ${SOURCES} ${HEADERS})
# 创建可执行文件
#add_executable(cpp_code_generator ${MAIN_SOURCES} ${MAIN_HEADERS})

add_executable(cpp_code_generator src/main.cpp)
target_link_libraries(cpp_code_generator cpp_code_generator_lib)

# 设置目标的导出名称
set_target_properties(cpp_code_generator PROPERTIES EXPORT_NAME CppCodeGenerator)

# 链接Boost库
target_link_libraries(cpp_code_generator 
    Boost::filesystem 
    Boost::program_options
    Boost::json
)

# 安装目标
install(TARGETS cpp_code_generator
    EXPORT CppCodeGeneratorTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件
install(DIRECTORY include/code_generator
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 安装配置文件
install(DIRECTORY config/
    DESTINATION share/cpp_code_generator/config
)

# 测试配置（可选）
if(BUILD_TESTING)
    enable_testing()
    # 添加测试...
endif()

# 编译特性配置
#target_compile_features(cpp_code_generator PRIVATE cxx_std_17)
target_compile_features(cpp_code_generator PRIVATE cxx_std_11)

# 编译器特定设置
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cpp_code_generator PRIVATE -g -Wall -Wextra -Wpedantic -fpermissive)
    target_compile_options(cpp_code_generator_lib PRIVATE -g -Wall -Wextra -Wpedantic -fpermissive)
endif()

# 版本信息
#configure_file(
#    ${CMAKE_SOURCE_DIR}/version.h.in
#    ${CMAKE_BINARY_DIR}/include/code_generator/version.h
#)

# 包配置
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/CppCodeGeneratorConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# 导出目标
export(EXPORT CppCodeGeneratorTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/CppCodeGeneratorTargets.cmake
)

# 安装导出
install(EXPORT CppCodeGeneratorTargets
    FILE CppCodeGeneratorTargets.cmake
    DESTINATION lib/cmake/CppCodeGenerator
)

# 安装包配置文件
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CppCodeGeneratorConfigVersion.cmake
    DESTINATION lib/cmake/CppCodeGenerator
)
