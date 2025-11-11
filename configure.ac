# configure.ac
AC_PREREQ([2.69])
AC_INIT([CppCodeGenerator], [1.0.0], [your-email@example.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# 添加这行 - 初始化Automake
AM_INIT_AUTOMAKE([foreign subdir-objects])

# 检查C++编译器
AC_PROG_CXX
AC_LANG([C++])

# 检查C++17支持
#AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

AC_SUBST([CXXFLAGS], ["-g -std=c++11 -Wall -Wextra -pedantic -fpermissive"])

# 检查Boost
#AX_BOOST_BASE([1.65.0])
#AX_BOOST_FILESYSTEM
#AX_BOOST_JSON
#AX_BOOST_PROGRAM_OPTIONS

BOOST_FILESYSTEM_LIB="-lboost_filesystem"
BOOST_JSON_LIB="-lboost_json"
BOOST_PROGRAM_OPTIONS_LIB="-lboost_program_options"
AC_SUBST(BOOST_FILESYSTEM_LIB)
AC_SUBST(BOOST_JSON_LIB)
AC_SUBST(BOOST_PROGRAM_OPTIONS_LIB)

# 检查pkg-config
PKG_PROG_PKG_CONFIG

# 输出文件
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
