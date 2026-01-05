# src/Makefile.am
ACLOCAL_AMFLAGS = $(ACLOCAL_AMFLAGS) -I ../m4

# 源文件
lib_LTLIBRARIES =
noinst_LIBRARIES =
bin_PROGRAMS =

# 构建静态库
if BUILD_STATIC
    noinst_LIBRARIES += libcppcodegen_s.a
endif

# 构建动态库
if BUILD_SHARED
    lib_LTLIBRARIES += libcppcodegen.la
endif

# 静态库源文件
if BUILD_STATIC
libcppcodegen_s_a_SOURCES = \
    zero_copy_stream.cpp \
    file_streams.cpp \
    formatter.cpp \
    cpp_generator.cpp \
    config_parser.cpp \
    enhanced_cpp_generator.cpp \
    stream_adapters.cpp

libcppcodegen_s_a_CPPFLAGS = $(AM_CPPFLAGS)
libcppcodegen_s_a_CXXFLAGS = $(AM_CXXFLAGS)
libcppcodegen_s_a_LIBADD = $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)
endif

# 动态库源文件
if BUILD_SHARED
libcppcodegen_la_SOURCES = \
    zero_copy_stream.cpp \
    file_streams.cpp \
    formatter.cpp \
    cpp_generator.cpp \
    config_parser.cpp \
    enhanced_cpp_generator.cpp \
    stream_adapters.cpp

libcppcodegen_la_CPPFLAGS = $(AM_CPPFLAGS)
libcppcodegen_la_CXXFLAGS = $(AM_CXXFLAGS) -fPIC

# Windows DLL导出定义
if WINDOWS
    libcppcodegen_la_DEF = -Wl,--export-all-symbols
endif

libcppcodegen_la_LIBADD = $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)
libcppcodegen_la_LDFLAGS = $(AM_LDFLAGS) -version-info 1:0:0
endif

# 包含目录
AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../third_party

# 编译器标志
AM_CXXFLAGS = $(CXXFLAGS)

# 主程序
bin_PROGRAMS += cppcodegen

cppcodegen_SOURCES = main.cpp
cppcodegen_CPPFLAGS = $(AM_CPPFLAGS)
cppcodegen_CXXFLAGS = $(AM_CXXFLAGS)

# 链接库选择 - 优先使用动态库
if BUILD_SHARED
    cppcodegen_LDADD = libcppcodegen.la $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)
endif

if BUILD_STATIC
    if !BUILD_SHARED
        cppcodegen_LDADD = libcppcodegen_s.a $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)
    endif
endif

cppcodegen_LDFLAGS = $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS) $(BOOST_JSON_LDFLAGS)

# 测试支持
# if BUILD_TESTS
    # check_PROGRAMS = test_cpp_generator
    # test_cpp_generator_SOURCES = test_cpp_generator.cpp
    # test_cpp_generator_CPPFLAGS = $(AM_CPPFLAGS)
    # test_cpp_generator_CXXFLAGS = $(AM_CXXFLAGS)
    
    #  设置测试程序依赖和链接库 - 优先使用动态库
    # if BUILD_SHARED
        # test_cpp_generator_DEPENDENCIES = libcppcodegen.la
        # test_cpp_generator_LDADD = libcppcodegen.la $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)
    # endif
    
    # if BUILD_STATIC
        # if !BUILD_SHARED
            # test_cpp_generator_DEPENDENCIES = libcppcodegen_s.a
            # test_cpp_generator_LDADD = libcppcodegen_s.a $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_JSON_LIB)
        # endif
    # endif
    
    # test_cpp_generator_LDFLAGS = $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS)
    
    # TESTS = test_cpp_generator
# endif

# 安装头文件
include $(top_srcdir)/include/Makefile.am
