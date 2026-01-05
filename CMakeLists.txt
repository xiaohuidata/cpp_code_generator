cmake_minimum_required(VERSION 3.16)
project(CppCodeGenerator VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准 - 优先使用C++17，fallback到C++11
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "7.0")
        set(CMAKE_CXX_STANDARD 17)
    else()
        set(CMAKE_CXX_STANDARD 11)
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 启用导出设置
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# 库类型选择选项
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" ON)

# 跨平台库设置
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(BUILD_SHARED_LIBS_DEFAULT ON)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
    set(CMAKE_SHARED_MODULE_SUFFIX ".dll")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
elseif(APPLE)
    set(BUILD_SHARED_LIBS_DEFAULT ON)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
    set(CMAKE_SHARED_MODULE_SUFFIX ".so")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
else()
    set(BUILD_SHARED_LIBS_DEFAULT ON)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
    set(CMAKE_SHARED_MODULE_SUFFIX ".so")
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
endif()

# 确保至少构建一种库类型
if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    set(BUILD_SHARED_LIBS ON)
endif()

# 查找Boost库 - 更健壮的查找方式
if(Boost_FOUND)
    message(STATUS "Found Boost: ${Boost_INCLUDE_DIRS}")
else()
    find_package(Boost 1.75.0 REQUIRED COMPONENTS filesystem program_options json)
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost libraries not found. Please install Boost 1.75.0 or later.")
    endif()
endif()

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# 检查C++17支持
if(CMAKE_CXX_STANDARD EQUAL 17)
    add_definitions(-DENABLE_CXX17)
endif()

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

# 创建库目标
# 根据选项决定构建静态库还是动态库或者两者都构建

if(BUILD_STATIC_LIBS)
    add_library(cpp_code_generator_static STATIC ${SOURCES} ${HEADERS})
    set_target_properties(cpp_code_generator_static PROPERTIES 
        OUTPUT_NAME "cppcodegen_s"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    set(STATIC_TARGET cpp_code_generator_static)
endif()

if(BUILD_SHARED_LIBS)
    add_library(cpp_code_generator_shared SHARED ${SOURCES} ${HEADERS})
    set_target_properties(cpp_code_generator_shared PROPERTIES 
        OUTPUT_NAME "cppcodegen"
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    
    # Windows DLL导出设置
    if(WIN32)
        target_compile_definitions(cpp_code_generator_shared
            PRIVATE CPP_CODE_GENERATOR_EXPORTS
        )
    endif()
    
    set(SHARED_TARGET cpp_code_generator_shared)
endif()

# 创建可执行文件
add_executable(cpp_code_generator src/main.cpp)

# 设置可执行文件属性
set_target_properties(cpp_code_generator PROPERTIES 
    OUTPUT_NAME "cppcodegen"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# 链接库 - 如果两者都构建，默认链接共享库
if(BUILD_SHARED_LIBS)
    target_link_libraries(cpp_code_generator PRIVATE ${SHARED_TARGET})
elseif(BUILD_STATIC_LIBS)
    target_link_libraries(cpp_code_generator PRIVATE ${STATIC_TARGET})
endif()

# 链接Boost库 - 使用正确的Boost组件
if(TARGET Boost::filesystem)
    if(BUILD_SHARED_LIBS)
        target_link_libraries(cpp_code_generator_shared PRIVATE Boost::filesystem)
    endif()
    if(BUILD_STATIC_LIBS)
        target_link_libraries(cpp_code_generator_static PRIVATE Boost::filesystem)
    endif()
    target_link_libraries(cpp_code_generator PRIVATE Boost::filesystem)
else()
    if(BUILD_SHARED_LIBS)
        target_link_libraries(cpp_code_generator_shared PRIVATE boost_filesystem)
    endif()
    if(BUILD_STATIC_LIBS)
        target_link_libraries(cpp_code_generator_static PRIVATE boost_filesystem)
    endif()
    target_link_libraries(cpp_code_generator PRIVATE boost_filesystem)
endif()

# Boost Program Options链接
if(TARGET Boost::program_options)
    if(BUILD_SHARED_LIBS)
        target_link_libraries(cpp_code_generator_shared PRIVATE Boost::program_options)
    endif()
    if(BUILD_STATIC_LIBS)
        target_link_libraries(cpp_code_generator_static PRIVATE Boost::program_options)
    endif()
    target_link_libraries(cpp_code_generator PRIVATE Boost::program_options)
else()
    if(BUILD_SHARED_LIBS)
        target_link_libraries(cpp_code_generator_shared PRIVATE boost_program_options)
    endif()
    if(BUILD_STATIC_LIBS)
        target_link_libraries(cpp_code_generator_static PRIVATE boost_program_options)
    endif()
    target_link_libraries(cpp_code_generator PRIVATE boost_program_options)
endif()

# JSON库支持
if(TARGET Boost::json)
    if(BUILD_SHARED_LIBS)
        target_link_libraries(cpp_code_generator_shared PRIVATE Boost::json)
    endif()
    if(BUILD_STATIC_LIBS)
        target_link_libraries(cpp_code_generator_static PRIVATE Boost::json)
    endif()
    target_link_libraries(cpp_code_generator PRIVATE Boost::json)
else()
    if(BUILD_SHARED_LIBS)
        target_link_libraries(cpp_code_generator_shared PRIVATE ${Boost_JSON_LIBRARIES})
    endif()
    if(BUILD_STATIC_LIBS)
        target_link_libraries(cpp_code_generator_static PRIVATE ${Boost_JSON_LIBRARIES})
    endif()
    target_link_libraries(cpp_code_generator PRIVATE ${Boost_JSON_LIBRARIES})
endif()

# 安装目标
if(BUILD_STATIC_LIBS)
    install(TARGETS cpp_code_generator_static
        EXPORT CppCodeGeneratorTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )
endif()

if(BUILD_SHARED_LIBS)
    install(TARGETS cpp_code_generator_shared
        EXPORT CppCodeGeneratorTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

# 安装可执行文件
install(TARGETS cpp_code_generator
    EXPORT CppCodeGeneratorTargets
    RUNTIME DESTINATION bin
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
if(CMAKE_CXX_STANDARD EQUAL 17)
    target_compile_features(cpp_code_generator PRIVATE cxx_std_17)
    if(BUILD_SHARED_LIBS)
        target_compile_features(cpp_code_generator_shared PRIVATE cxx_std_17)
    endif()
    if(BUILD_STATIC_LIBS)
        target_compile_features(cpp_code_generator_static PRIVATE cxx_std_17)
    endif()
else()
    target_compile_features(cpp_code_generator PRIVATE cxx_std_11)
    if(BUILD_SHARED_LIBS)
        target_compile_features(cpp_code_generator_shared PRIVATE cxx_std_11)
    endif()
    if(BUILD_STATIC_LIBS)
        target_compile_features(cpp_code_generator_static PRIVATE cxx_std_11)
    endif()
endif()

# 编译器特定设置
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cpp_code_generator PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -fpermissive
    )
    
    # 为库目标设置编译选项
    if(BUILD_SHARED_LIBS)
        target_compile_options(cpp_code_generator_shared PRIVATE 
            -Wall 
            -Wextra 
            -Wpedantic 
            -Wno-unused-parameter
            -Wno-missing-field-initializers
            -fpermissive
        )
    endif()
    
    if(BUILD_STATIC_LIBS)
        target_compile_options(cpp_code_generator_static PRIVATE 
            -Wall 
            -Wextra 
            -Wpedantic 
            -Wno-unused-parameter
            -Wno-missing-field-initializers
            -fpermissive
        )
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(cpp_code_generator PRIVATE 
        /W4
        /EHsc
        /MP
        /Zc:__cplusplus
    )
    
    # 为库目标设置编译选项
    if(BUILD_SHARED_LIBS)
        target_compile_options(cpp_code_generator_shared PRIVATE 
            /W4
            /EHsc
            /MP
            /Zc:__cplusplus
        )
    endif()
    
    if(BUILD_STATIC_LIBS)
        target_compile_options(cpp_code_generator_static PRIVATE 
            /W4
            /EHsc
            /MP
            /Zc:__cplusplus
        )
    endif()
endif()

# 版本信息
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

configure_file(
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_BINARY_DIR}/include/code_generator/version.h
)

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
