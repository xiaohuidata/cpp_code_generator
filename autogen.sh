#!/bin/bash

echo "Generating Autotools build system files..."

# 创建必要的目录
mkdir -p m4

# 清理旧文件
rm -f aclocal.m4
rm -f configure
rm -rf autom4te.cache/

# 检查Makefile.am文件名
if [ ! -f "Makefile.am" ] && [ -f "makefile.am" ]; then
    echo "Renaming makefile.am to Makefile.am"
    mv makefile.am Makefile.am
fi

# 确保Makefile.am存在
if [ ! -f "Makefile.am" ]; then
    echo "Error: Makefile.am not found"
    exit 1
fi

echo "Found Makefile.am"

# 生成autotools文件
echo "Running aclocal..."
aclocal -I m4

echo "Running autoheader..."
autoheader

echo "Running automake..."
automake --add-missing --foreign

echo "Running autoconf..."
autoconf -vif

echo "Autotools files generated successfully!"
