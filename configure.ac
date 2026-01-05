# configure.ac - 配置脚本
AC_PREREQ([2.69])
AC_INIT([CppCodeGenerator], [1.0.0], [cppcodegen@example.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# 允许未定义的宏模式
m4_pattern_forbid([^_?AC_])

# 初始化Automake - 支持并行构建和子目录对象
AM_INIT_AUTOMAKE([foreign subdir-objects parallel-tests dist-xz -Wall -Werror])

# 启用C++支持
AC_LANG([C++])

# 检查C++编译器
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR
AX_CXX_COMPILE_STDCXX_17([noext], [mandatory], [CXX17])
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory], [CXX11])

# 初始化Libtool
LT_INIT([shared disable-static])

# 设置编译器标志
AS_IF([test "x$CXX17" = "xyes"],
      [CXXFLAGS="$CXXFLAGS -std=c++17"],
      [CXXFLAGS="$CXXFLAGS -std=c++11"])
      
CXXFLAGS="$CXXFLAGS -Wall -Wextra -pedantic"

# 设置C++标准变量
if test "x$CXX17" = "xyes"; then
    CXX_STANDARD="C++17"
else
    CXX_STANDARD="C++11"
fi

# 默认构建类型
BUILD_TYPE="Release"

# 检查Boost库
AX_BOOST_BASE([1.75.0], [], [AC_MSG_ERROR([Boost 1.75.0 or later is required])])
AX_BOOST_FILESYSTEM
AX_BOOST_JSON
AX_BOOST_PROGRAM_OPTIONS

# 如果Boost JSON不可用，尝试其他JSON库
# AS_IF([test "x$boost_json_found" != "xyes"],
      # [PKG_PROG_PKG_CONFIG
       # PKG_CHECK_MODULES([JSONCPP], [jsoncpp >= 1.7.0], 
                         # [AC_DEFINE([HAVE_JSONCPP], [1], [Use JsonCpp library])
                          # JSONCPP_LIBS="$JSONCPP_LIBS"
                          # JSONCPP_CFLAGS="$JSONCPP_CFLAGS"],
                         # [AC_MSG_WARN([No JSON library found. JSON support will be disabled.])])])

# 检查pkg-config
PKG_PROG_PKG_CONFIG

# 库类型选择
AC_ARG_ENABLE([shared-libs],
              AS_HELP_STRING([--enable-shared-libs], [Build shared libraries]),
              [build_shared="${enableval}"],
              [build_shared="yes"])

AC_ARG_ENABLE([static-libs],
              AS_HELP_STRING([--enable-static-libs], [Build static libraries]),
              [build_static="${enableval}"],
              [build_static="yes"])

# 确保至少构建一种库类型
AS_IF([test "x$build_shared" = "xno" && test "x$build_static" = "xno"],
      [AC_MSG_ERROR([At least one of --enable-shared-libs or --enable-static-libs must be enabled])])

# 检查平台特定的库设置
case "$host_os" in
    cygwin*|mingw*|msys*)
        # Windows平台特定设置
        shared_suffix="dll"
        static_suffix="lib"
        ;;
    darwin*)
        # macOS平台特定设置
        shared_suffix="dylib"
        static_suffix="a"
        ;;
    *)
        # Linux和其他Unix系统
        shared_suffix="so"
        static_suffix="a"
        ;;
esac

# 设置输出变量
AC_SUBST([CXXFLAGS])
AC_SUBST([BUILD_SHARED], [$build_shared])
AC_SUBST([BUILD_STATIC], [$build_static])
AC_SUBST([SHARED_SUFFIX], [$shared_suffix])
AC_SUBST([STATIC_SUFFIX], [$static_suffix])

# 设置Boost版本变量
AS_IF([test "x$boost_major_version" != "x"],
      [BOOST_VERSION="$boost_major_version.$boost_minor_version.$boost_subminor_version"],
      [BOOST_VERSION="unknown"])

# 安装目录设置
AC_ARG_ENABLE([debug],
              AS_HELP_STRING([--enable-debug], [Enable debug builds]),
              [case "${enableval}" in
                   yes) debug=true ;;
                   no)  debug=false ;;
                   *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
               esac],
              [debug=false])

AS_IF([test "x$debug" = "xtrue"],
      [CXXFLAGS="$CXXFLAGS -g -O0 -DDEBUG"],
      [CXXFLAGS="$CXXFLAGS -O3 -DNDEBUG"])

# 启用/禁用测试
AC_ARG_ENABLE([tests],
              AS_HELP_STRING([--enable-tests], [Build test suite]),
              [build_tests="${enableval}"],
              [build_tests="auto"])

# 测试配置
AM_CONDITIONAL([BUILD_TESTS], [test "x$build_tests" != "xno"])

# 库类型条件
AM_CONDITIONAL([BUILD_STATIC], [test "x$build_static" = "xyes"])
AM_CONDITIONAL([BUILD_SHARED], [test "x$build_shared" = "xyes"])

# 平台条件
AM_CONDITIONAL([WINDOWS], [echo "$host_os" | grep -qE '(cygwin|mingw|msys)'])

# 输出文件
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    include/Makefile
    config/Makefile
    config/templates/Makefile
    config/examples/Makefile
])

# 配置摘要
echo ""
echo "=== C++ Code Generator Configuration Summary ==="
echo "Version:                    ${VERSION}"
echo "C++ Standard:               ${CXX_STANDARD}"
echo "Compiler:                   ${CXX}"
echo "Compiler Flags:             ${CXXFLAGS}"
echo ""
echo "Platform:                   ${host_os}"
echo "Shared Library:             ${build_shared} (suffix: .${shared_suffix})"
echo "Static Library:             ${build_static} (suffix: .${static_suffix})"
echo ""
echo "Boost Version:              ${BOOST_VERSION}"
echo "Boost Filesystem:           ${boost_filesystem_found}"
echo "Boost Program Options:      ${boost_program_options_found}"
echo "Boost JSON:                 ${boost_json_found}"
echo ""
echo "pkg-config:                 ${PKG_CONFIG}"
echo ""
echo "Installation Directories:"
echo "  Prefix:                   ${prefix}"
echo "  Bindir:                   ${bindir}"
echo "  Libdir:                   ${libdir}"
echo "  Includedir:               ${includedir}"
echo "  Datarootdir:              ${datarootdir}"
echo ""
echo "Build will be configured to: ${BUILD_TYPE}"
echo "================================================="
echo ""

AC_OUTPUT

# 显示配置摘要
AC_MSG_NOTICE([])
AC_MSG_NOTICE([Configuration Summary:])
AC_MSG_NOTICE([========================])
AC_MSG_NOTICE([Project:          CppCodeGenerator])
AC_MSG_NOTICE([Version:          1.0.0])
AC_MSG_NOTICE([C++ Compiler:     $CXX])
AC_MSG_NOTICE([C++ Standard:     $CXXFLAGS])
AC_MSG_NOTICE([Boost Found:      $boost_system_found])
AC_MSG_NOTICE([Boost JSON:       $boost_json_found])
AC_MSG_NOTICE([Debug Mode:       $debug])
AC_MSG_NOTICE([Build Tests:      $build_tests])
AC_MSG_NOTICE([Prefix:           $prefix])
AC_MSG_NOTICE([========================])
AC_MSG_NOTICE([])
