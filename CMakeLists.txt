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
# 在MinGW下默认只构建静态库，因为MinGW和Visual Studio的动态库不兼容
# 在其他平台下默认同时构建静态库和动态库
set(BUILD_SHARED_LIBS_DEFAULT ON)
set(BUILD_STATIC_LIBS_DEFAULT ON)

if(MINGW)
    set(BUILD_SHARED_LIBS_DEFAULT OFF)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" ${BUILD_SHARED_LIBS_DEFAULT})
option(BUILD_STATIC_LIBS "Build static libraries" ${BUILD_STATIC_LIBS_DEFAULT})

# 确保至少构建一种库类型
if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(WARNING "Neither shared nor static libraries will be built. Enabling static libraries by default.")
    set(BUILD_STATIC_LIBS ON)
endif()

# 跨平台库设置
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(BUILD_SHARED_LIBS_DEFAULT ON)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
    set(CMAKE_SHARED_MODULE_SUFFIX ".dll")
    
    if(MINGW)
        set(CMAKE_STATIC_LIBRARY_SUFFIX ".a")
    else()
        set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
    endif()
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

# 设置Boost库 - 使用环境变量BOOST_HOME和BOOST_LIBS
set(Boost_VERSION "1.75.0") # 假设使用的是1.75.0版本

if(BOOST_ROOT)
    set(BOOST_HOME ${BOOST_ROOT})
    if(WIN32)
        # Windows 平台：检查目录中是否有 boost*.lib 或 libboost*.lib 文件
        set(FOUND_BOOST_DIR FALSE)

        # 尝试多个可能的库目录
        set(POSSIBLE_LIB_DIRS 
            "${BOOST_ROOT}/lib64-msvc-14.3"
            "${BOOST_ROOT}/lib64-msvc-14.2"
            "${BOOST_ROOT}/lib64-msvc-14.1"
            "${BOOST_ROOT}/lib64"
            "${BOOST_ROOT}/lib"
            "${BOOST_ROOT}/stage/lib"
            )

        foreach(LIB_DIR ${POSSIBLE_LIB_DIRS})
            if(EXISTS "${LIB_DIR}")
                # 检查该目录下是否有 Boost 库文件
                file(GLOB BOOST_LIBS_CHECK 
                    "${LIB_DIR}/boost*.lib"
                    "${LIB_DIR}/libboost*.lib"
                    "${LIB_DIR}/boost*.a"
                    "${LIB_DIR}/libboost*.a"
                    "${LIB_DIR}/boost*.dll"
                    "${LIB_DIR}/libboost*.dll"
                    )

                if(BOOST_LIBS_CHECK)
                    set(BOOST_LIBS "${LIB_DIR}")
                    set(FOUND_BOOST_DIR TRUE)
                    message(STATUS "Found Boost libraries in: ${LIB_DIR}")
                    break()
                endif()
            endif()
        endforeach()

        if(NOT FOUND_BOOST_DIR)
            message(FATAL_ERROR "Boost library directory not found in ${BOOST_ROOT}")
        endif()

    else()
        # Unix-like 平台 (Linux/macOS)：检查目录中是否有 libboost* 或 boost* 文件
        set(FOUND_BOOST_DIR FALSE)

        # 尝试多个可能的库目录
        set(POSSIBLE_LIB_DIRS 
            "${BOOST_ROOT}/lib64"
            "${BOOST_ROOT}/lib"
            "${BOOST_ROOT}/stage/lib"
            "${BOOST_ROOT}/lib/x86_64-linux-gnu"
            "${BOOST_ROOT}/lib/x64"
            )

        foreach(LIB_DIR ${POSSIBLE_LIB_DIRS})
            if(EXISTS "${LIB_DIR}")
                # 检查该目录下是否有 Boost 库文件
                file(GLOB BOOST_LIBS_CHECK 
                    "${LIB_DIR}/libboost*.so*"
                    "${LIB_DIR}/libboost*.a"
                    "${LIB_DIR}/libboost*.dylib*"
                    "${LIB_DIR}/boost*.so*"
                    "${LIB_DIR}/boost*.a"
                    "${LIB_DIR}/boost*.dylib*"
                    )

                if(BOOST_LIBS_CHECK)
                    set(BOOST_LIBS "${LIB_DIR}")
                    set(FOUND_BOOST_DIR TRUE)
                    message(STATUS "Found Boost libraries in: ${LIB_DIR}")
                    break()
                endif()
            endif()
        endforeach()

        if(NOT FOUND_BOOST_DIR)
            message(FATAL_ERROR "Boost library directory not found in ${BOOST_ROOT}")
        endif()
    endif()
else()
    # 检查环境变量
    if(NOT DEFINED ENV{BOOST_HOME})
        message(FATAL_ERROR "BOOST_HOME environment variable not set. Please set it to Boost installation directory.")
    endif()

    if(NOT DEFINED ENV{BOOST_LIBS})
        message(FATAL_ERROR "BOOST_LIBS environment variable not set. Please set it to Boost library directory.")
    endif()
    set(BOOST_HOME "$ENV{BOOST_HOME}")
    set(BOOST_LIBS "$ENV{BOOST_LIBS}")
endif()

message(STATUS "Using Boost version: ${Boost_VERSION}")
message(STATUS "Boost home: ${BOOST_HOME}")
message(STATUS "Boost libraries directory: ${BOOST_LIBS}")

# 设置Boost的包含目录和库目录
if(EXISTS "${BOOST_HOME}/boost")
    set(Boost_INCLUDE_DIRS "${BOOST_HOME}")
elseif(EXISTS "${BOOST_HOME}/include/boost")
    set(Boost_INCLUDE_DIRS "${BOOST_HOME}/include")
else()
    message(FATAL_ERROR "Failed ${BOOST_HOME}/boost or ${BOOST_HOME}/include/boost con't finded.")
endif()
set(Boost_LIBRARY_DIRS "${BOOST_LIBS}")

message(STATUS "Using Boost from: ${BOOST_HOME}")
message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library directories: ${Boost_LIBRARY_DIRS}")

# 根据编译器和构建类型设置Boost库名称
set(BOOST_LIBS "")

# 添加所需的Boost库
set(BOOST_COMPONENTS "filesystem" "program_options" "json")

# 在Boost库目录中查找所有库文件
file(GLOB BOOST_ALL_LIBS "${Boost_LIBRARY_DIRS}/*boost*.*")

foreach(COMPONENT IN LISTS BOOST_COMPONENTS)
    set(COMPONENT_FOUND FALSE)

    # 更灵活的正则表达式，覆盖更多命名格式
    # if(WIN32)
        # Windows: 支持多种命名格式
        # boost_system-vc143-mt-x64-1_83.lib
        # boost_system-vc143-mt.lib
        # boost_system-mt.lib
        # boost_system.lib
        # libboost_system-mt-x64.lib (MinGW)
        # set(COMPONENT_MATCHES
        #   "^(lib)?boost_${COMPONENT}([._-].*)?\\.(lib|a)$"
        # )
    # else()
        # Unix-like: 支持静态库和动态库
        # libboost_system.so.1.83.0
        # libboost_system.so
        # libboost_system.a
        # set(COMPONENT_MATCHES
        #   "^libboost_${COMPONENT}([._-].*)?\\.(so(\\.\\d+)*|a|dylib)$"
        # )
    # endif()

    # 尝试匹配不同的库命名格式
    foreach(LIB_FILE IN LISTS BOOST_ALL_LIBS)
        get_filename_component(LIB_NAME "${LIB_FILE}" NAME)
        # if(LIB_NAME MATCHES ${COMPONENT_MATCHES})
        if(LIB_NAME MATCHES "boost_${COMPONENT}")
            if(WIN32)
                if(LIB_NAME MATCHES ".*\\.lib$")
                    set(COMPONENT_FOUND TRUE)
                endif()
            else()
                if (LIB_NAME MATCHES ".*\\.so$")
                    set(COMPONENT_FOUND TRUE)
                endif()
            endif()
        endif()
        if(COMPONENT_FOUND)
            message(STATUS "Found Boost ${COMPONENT}: ${LIB_NAME}")
            list(APPEND BOOST_LIBS "${LIB_FILE}")
            break()
        endif()
    endforeach()

    if(NOT COMPONENT_FOUND AND NOT WIN32)
        foreach(LIB_FILE IN LISTS BOOST_ALL_LIBS)
            get_filename_component(LIB_NAME "${LIB_FILE}" NAME)
            if(LIB_NAME MATCHES "boost_${COMPONENT}" AND LIB_NAME MATCHES ".*\\.a$")
                set(COMPONENT_FOUND_ TRUE)
                message(STATUS "Found Boost ${COMPONENT}: ${LIB_NAME}")
                list(APPEND BOOST_LIBS "${LIB_FILE}")
                break()
            endif()
        endforeach()
    endif()

    if(NOT COMPONENT_FOUND)
        message(FATAL_ERROR "Boost ${COMPONENT} library not found in ${Boost_LIBRARY_DIRS}")
    endif()
endforeach()

# 设置Boost_FOUND为真，以便后续代码使用
set(Boost_FOUND TRUE)
        
# 设置编译选项 - 按编译器类型分别设置
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/Os /DNDEBUG")
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
    ${Boost_INCLUDE_DIRS}
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
    
    if(WIN32)
        target_compile_definitions(cpp_code_generator_shared
            PRIVATE CPP_CODE_GENERATOR_EXPORTS
        )
    endif()
    
    set(SHARED_TARGET cpp_code_generator_shared)
endif()

# 创建可执行文件 - 使用所有必要的源文件
add_executable(cpp_code_generator ${MAIN_SOURCES})

# 设置可执行文件属性
set_target_properties(cpp_code_generator PROPERTIES 
    OUTPUT_NAME "cppcodegen"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Windows调试器工作目录设置
if(WIN32 AND MSVC)
    set_target_properties(cpp_code_generator PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# Linux 特定的设置
if(UNIX AND NOT APPLE)
    # rpath 设置
    set_target_properties(cpp_code_generator PROPERTIES 
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN/../lib"
    )
    
    if(BUILD_SHARED_LIBS)
        set_target_properties(cpp_code_generator_shared PROPERTIES 
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "$ORIGIN/../lib"
        )
    endif()
    
    # Linux下的链接器选项
    target_link_options(cpp_code_generator PRIVATE -Wl,--as-needed)
    
    if(BUILD_SHARED_LIBS)
        target_link_options(cpp_code_generator_shared PRIVATE -Wl,--as-needed)
    endif()
endif()

# 统一链接Boost库
if(BUILD_SHARED_LIBS)
    target_link_libraries(cpp_code_generator_shared PRIVATE ${BOOST_LIBS})
endif()

if(BUILD_STATIC_LIBS)
    target_link_libraries(cpp_code_generator_static PRIVATE ${BOOST_LIBS})
endif()

target_link_libraries(cpp_code_generator PRIVATE ${BOOST_LIBS})

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
set(COMPILER_SPECIFIC_OPTIONS "")

# 根据编译器类型设置不同的编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang编译选项
    set(GCC_CLANG_OPTIONS 
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -fpermissive
    )
    
    # 添加到编译器特定选项
    list(APPEND COMPILER_SPECIFIC_OPTIONS ${GCC_CLANG_OPTIONS})
    
    # 对于MinGW，添加额外选项
    if(MINGW)
        list(APPEND COMPILER_SPECIFIC_OPTIONS 
            -mthreads
            -Wno-deprecated-declarations
            -Wno-unused-variable
        )
    endif()
elseif(MSVC)
    # MSVC编译选项
    set(MSVC_OPTIONS 
        /W4
        /EHsc
        /MP
        /Zc:__cplusplus
    )
    
    # 添加到编译器特定选项
    list(APPEND COMPILER_SPECIFIC_OPTIONS ${MSVC_OPTIONS})
endif()

# 应用编译选项到所有目标
target_compile_options(cpp_code_generator PRIVATE ${COMPILER_SPECIFIC_OPTIONS})

if(BUILD_SHARED_LIBS)
    target_compile_options(cpp_code_generator_shared PRIVATE ${COMPILER_SPECIFIC_OPTIONS})
endif()

if(BUILD_STATIC_LIBS)
    target_compile_options(cpp_code_generator_static PRIVATE ${COMPILER_SPECIFIC_OPTIONS})
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
