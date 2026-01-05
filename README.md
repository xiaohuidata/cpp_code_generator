

使用 autogen.sh 编译生成
---------------------------
若AX_BOOST_JSON未被成功解析，请运行以下命名，将AX_BOOST_JSON的解析文件放入m4：
# 下载宏文件
	wget -O m4/ax_boost_json.m4 https://raw.githubusercontent.com/autoconf-archive/autoconf-archive/master/m4/ax_boost_json.m4

# 重新生成
	aclocal -I m4
