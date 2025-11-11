#ifndef STREAM_ADAPTERS_H
#define STREAM_ADAPTERS_H

#include "zero_copy_stream.h"
#include <iostream>
#include <sstream>
#include <string>

namespace code_generator {

// 将ZeroCopyOutputStream适配到std::ostream
class OStreamOutputStream : public ZeroCopyOutputStream {
public:
	explicit OStreamOutputStream(std::ostream* output, int buffer_size = 8192);
	~OStreamOutputStream() override;

	bool Next(void** data, int* size) override;
	void BackUp(int count) override;
	int64_t ByteCount() const override;
	bool Flush() override;

private:
	std::ostream* output_;
	char* buffer_;
	int buffer_size_;
	int buffer_offset_;
	int64_t total_bytes_;
};

// 字符串输出流
class StringOutputStream : public ZeroCopyOutputStream {
public:
	explicit StringOutputStream(std::string* target);

	bool Next(void** data, int* size) override;
	void BackUp(int count) override;
	int64_t ByteCount() const override;

private:
	std::string* target_;
	int64_t total_bytes_;
};

// 流工具类，提供便捷操作
class StreamUtil {
public:
	// 从输入流读取所有内容到字符串
	static bool ReadToString(ZeroCopyInputStreamPtr input, std::string* output);

	// 从文件读取所有内容到字符串
	static bool ReadFileToString(const std::string& filename, std::string* content);

	// 将字符串写入文件
	static bool WriteStringToFile(const std::string& content, const std::string& filename);

	// 复制流
	static bool CopyStream(ZeroCopyInputStreamPtr input, ZeroCopyOutputStreamPtr output);
};

}
#endif
