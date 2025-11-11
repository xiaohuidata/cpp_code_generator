#include "code_generator/zero_copy_stream.h"
#include <boost/algorithm/string.hpp>
#include <cstring>
#include <string.h>

namespace code_generator {

bool ZeroCopyOutputStream::WriteChar(char value) {
	void* data;
	int size;
	if (!Next(&data, &size) || size < 1) {
		return false;
	}
	static_cast<char*>(data)[0] = value;
	if (size > 1) {
		BackUp(size -1);
	}
	return true;
}

bool ZeroCopyOutputStream::WriteRaw(const void* data, int size) {
	const char* src = static_cast<const char*>(data);
	int remaining = size;

	while (remaining > 0) {
		void* dst;
		int dst_size;
		if (!Next(&dst, &dst_size)) {
			return false;
		}

		int copy_size = std::min(dst_size, remaining);
		memcpy(dst, src, copy_size);
		src += copy_size;
		remaining -= copy_size;

		if (copy_size < dst_size) {
			BackUp(dst_size - copy_size);
		}
	}
	return true;
}

bool ZeroCopyOutputStream::WriteString(const std::string& str) {
	return WriteRaw(str.data(), static_cast<int>(str.size()));
}

bool ZeroCopyInputStream::Skip(int count) {
	while (count > 0) {
		const void* data;
		int size;
		if (!Next(&data, &size)) {
			return false;
		}

		if (size >= count) {
			BackUp(size - count);
			return true;
		}
		count -= size;
	}
	return true;
}

bool ZeroCopyInputStream::ReadChar(char* value) {
	const void* data;
	int size;
	if (!Next(&data, &size) || size < 1) {
		return false;
	}

	*value =static_cast<const char*>(data)[0];
	if (size > 1) {
		BackUp(size - 1);
	}
	return true;
}

bool ZeroCopyInputStream::ReadRaw(void* buffer, int size) {
	char* dst = static_cast<char*>(buffer);
	int remaining = size;

	while(remaining > 0) {
		const void* src;
		int src_size;
		if (!Next(&src, &src_size)) {
			return false;
		}

		int copy_size = std::min(src_size, remaining);
		memcpy(dst, src, copy_size);
		dst += copy_size;
		remaining -= copy_size;

		if (copy_size < src_size) {
			BackUp(src_size - copy_size);
		}
	}
	return true;
}

bool ZeroCopyInputStream::ReadToString(std::string* str, int size) {
	if (size < 0) {
		// Read all available data
		const void* data;
		int chunk_size;
		while (Next(&data, &chunk_size)) {
			str->append(static_cast<const char*>(data), chunk_size);
		}
		return true;
	} else {
		// Read specific amount
		str->reserve(str->size() + size);
		return ReadRaw(&(*str)[0], size);
	}
}

} // namespace code_generator
