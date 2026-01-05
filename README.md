
cmake 相关命令
---------------

	cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR -BUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON

	BUILD_TYPE=Debug/Release

使用 autogen.sh 编译生成
---------------------------
相关命令

	# 只构建静态库
	./configure --enable-static --disable-shared
	
	# 同时构建动态库和静态库
	./configure --enable-static

若AX_BOOST_JSON未被成功解析，请运行以下命名，将AX_BOOST_JSON的解析文件放入m4：
# 下载宏文件

	wget -O m4/ax_boost_json.m4 https://raw.githubusercontent.com/autoconf-archive/autoconf-archive/master/m4/ax_boost_json.m4

# 重新生成

	aclocal -I m4
