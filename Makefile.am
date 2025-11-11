# Makefile.am
ACLOCAL_AMFLAGS = -I m4

# 二进制程序
bin_PROGRAMS = cpp_code_generator

# 源代码文件
cpp_code_generator_SOURCES = \
    src/zero_copy_stream.cpp \
    src/file_streams.cpp \
    src/formatter.cpp \
    src/cpp_generator.cpp \
    src/config_parser.cpp \
    src/enhanced_cpp_generator.cpp \
    src/stream_adapters.cpp \
    src/main.cpp

# 包含目录
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/third_party

# 链接库
cpp_code_generator_LDADD = $(BOOST_FILESYSTEM_LIB) $(BOOST_JSON_LIB) $(BOOST_PROGRAM_OPTIONS_LIB)
cpp_code_generator_LDFLAGS = $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_JSON_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS)

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
    config/templates/singleton.json \
    config/templates/factory.json \
    config/templates/observer.json \
    config/examples/basic_project.json \
    config/examples/advanced_project.json \
    config/examples/test_project.json

# 编译器标志
#AM_CXXFLAGS = -Wall -Wextra -pedantic -std=c++17
AM_CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 -fpermissive
