#ifndef CODE_GENERATOR_ZERO_COPY_STREAM_HPP
#define CODE_GENERATOR_ZERO_COPY_STREAM_HPP

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/core/span.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace code_generator{

class ZeroCopyOutputStream : private boost::noncopyable {
public:
	virtual ~ZeroCopyOutputStream() = default;

	virtual bool Next(void** data, int* size) = 0;
	virtual void BackUp(int count) = 0;
	virtual int64_t ByteCount() const = 0;

	virtual bool WriteChar(char value);
	virtual bool WriteRaw(const void* data, int size);
	virtual bool WriteString(const std::string& str);
	virtual bool Flush() { return true; }
};

class ZeroCopyInputStream : private boost::noncopyable {
public:
	virtual ~ZeroCopyInputStream() = default;

	virtual bool Next(const void** data, int* size) = 0;
	virtual void BackUp(int count) = 0;
	virtual bool Skip(int count);
	virtual int64_t ByteCount() const = 0;

	virtual bool ReadChar(char* value);
	virtual bool ReadRaw(void* buffer, int size);
	virtual bool ReadToString(std::string* str, int size = -1);
};

// Boost智能指针别名
using ZeroCopyOutputStreamPtr = boost::shared_ptr<ZeroCopyOutputStream>;
using ZeroCopyInputStreamPtr = boost::shared_ptr<ZeroCopyInputStream>;
} // namespace code_generator

#endif
