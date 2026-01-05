# include/Makefile.am
code_generator_includedir = $(includedir)/code_generator

code_generator_include_HEADERS = \
    code_generator.h \
    code_generator/zero_copy_stream.h \
    code_generator/file_streams.h \
    code_generator/formatter.h \
    code_generator/cpp_generator.h \
    code_generator/config_parser.h \
    code_generator/enhanced_cpp_generator.h \
    code_generator/stream_adapters.h

# 版本头文件
nodist_code_generator_include_HEADERS = \
    code_generator/version.h

# 确保版本头文件目录存在
$(nodist_code_generator_include_HEADERS): $(top_builddir)/include/code_generator/version.h
	@true

# 清理
CLEANFILES = $(nodist_code_generator_include_HEADERS)