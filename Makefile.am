# Makefile.am
ACLOCAL_AMFLAGS = -I m4

# 初始化库变量
noinst_LIBRARIES =
lib_LTLIBRARIES =

# 构建静态库
if BUILD_STATIC
    noinst_LIBRARIES += libcppcodegen_s.a
endif

# 构建动态库
if BUILD_SHARED
    lib_LTLIBRARIES += libcppcodegen.la
endif

# 源文件
libcppcodegen_s_a_SOURCES = \
    src/zero_copy_stream.cpp \
    src/file_streams.cpp \
    src/formatter.cpp \
    src/cpp_generator.cpp \
    src/config_parser.cpp \
    src/enhanced_cpp_generator.cpp \
    src/stream_adapters.cpp

libcppcodegen_s_a_CPPFLAGS = $(AM_CPPFLAGS)
libcppcodegen_s_a_CXXFLAGS = $(AM_CXXFLAGS)
libcppcodegen_s_a_LIBADD = $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)

libcppcodegen_la_SOURCES = \
    src/zero_copy_stream.cpp \
    src/file_streams.cpp \
    src/formatter.cpp \
    src/cpp_generator.cpp \
    src/config_parser.cpp \
    src/enhanced_cpp_generator.cpp \
    src/stream_adapters.cpp

libcppcodegen_la_CPPFLAGS = $(AM_CPPFLAGS)
libcppcodegen_la_CXXFLAGS = $(AM_CXXFLAGS) -fPIC
libcppcodegen_la_LDFLAGS = $(AM_LDFLAGS) -version-info 1:0:0

if WINDOWS
    libcppcodegen_la_DEF = -Wl,--export-all-symbols
endif

libcppcodegen_la_LIBADD = $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)

# 二进制程序
bin_PROGRAMS = cpp_code_generator

cpp_code_generator_SOURCES = \
    src/main.cpp

cpp_code_generator_CPPFLAGS = $(AM_CPPFLAGS)
cpp_code_generator_CXXFLAGS = $(AM_CXXFLAGS)

# 链接库选择
if BUILD_SHARED
    cpp_code_generator_LDADD = libcppcodegen.la
endif

if BUILD_STATIC
    if !BUILD_SHARED
        cpp_code_generator_LDADD = libcppcodegen_s.a
    endif
endif

cpp_code_generator_LDFLAGS = $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS) $(BOOST_JSON_LDFLAGS)

# 安装头文件
code_generator_includedir = $(includedir)/code_generator
code_generator_include_HEADERS = \
    include/code_generator/zero_copy_stream.h \
    include/code_generator/file_streams.h \
    include/code_generator/formatter.h \
    include/code_generator/cpp_generator.h \
    include/code_generator/config_parser.h \
    include/code_generator/enhanced_cpp_generator.h \
    include/code_generator/stream_adapters.h \
    include/code_generator.h

# 安装配置文件
configdir = $(datarootdir)/cpp_code_generator/config
config_DATA = \
    config/templates/sigleton.json \
    config/templates/factory.json \
    config/templates/observer.json \
    config/examples/basic_project.json \
    config/examples/advanced_project.json \
    config/examples/test_project.json

# 编译器标志
#AM_CXXFLAGS = -Wall -Wextra -pedantic -std=c++17
AM_CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 -fpermissive

# 包含目录
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/third_party
