#include "code_generator/stream_adapters.h"
#include "code_generator/file_streams.h"
#include <cstring>

namespace code_generator{

OStreamOutputStream::OStreamOutputStream(std::ostream* output, int buffer_size)
		: output_(output), buffer_size_(buffer_size), buffer_offset_(0), total_bytes_(0) {
	buffer_ = new char[buffer_size_];
}

OStreamOutputStream::~OStreamOutputStream() {
	if (buffer_offset_ > 0) {
		output_->write(buffer_, buffer_offset_);
	}
	delete[] buffer_;
}

bool OStreamOutputStream::Next(void** data, int* size) {
	if (buffer_offset_ == buffer_size_) {
		if (!Flush()) {
			return false;
		}
	}

	*data = buffer_ + buffer_offset_;
	*size = buffer_size_ - buffer_offset_;
	buffer_offset_ = buffer_size_;
	return true;
}

void OStreamOutputStream::BackUp(int count) {
	if (count <= buffer_offset_) {
		buffer_offset_ -= count;
	}
}

int64_t OStreamOutputStream::ByteCount() const {
	return total_bytes_ + buffer_offset_;
}

bool OStreamOutputStream::Flush() {
	if (buffer_offset_ > 0) {
		output_->write(buffer_, buffer_offset_);
		if (output_->fail()) {
			return false;
		}
		total_bytes_ += buffer_offset_;
		buffer_offset_ = 0;
	}
	return true;
}

StringOutputStream::StringOutputStream(std::string* target)
		: target_(target), total_bytes_(0) {}

bool StringOutputStream::Next(void** data, int* size) {
	size_t old_size = target_->size();
	size_t new_size = std::max(old_size * 2, static_cast<size_t>(256));
	target_->resize(new_size);

	*data = &(*target_)[old_size];
	*size = static_cast<int>(new_size - old_size);
	total_bytes_ += *size;
	return true;
}

void StringOutputStream::BackUp(int count) {
	if (count > 0) {
		target_->resize(target_->size() - count);
		total_bytes_ -= count;
	}
}

int64_t StringOutputStream::ByteCount() const {
    return total_bytes_;
}

// StreamUtil实现
bool StreamUtil::ReadToString(ZeroCopyInputStreamPtr input, std::string* output) {
	output->clear();
	const void* data;
	int size;

	while (input->Next(&data, &size)) {
		if (size > 0) {
			output->append(static_cast<const char*>(data), size);
		}
	}

	return true;
}

bool StreamUtil::ReadFileToString(const std::string& filename, std::string* content) {
	FileInputStream input(filename);
	if (!input.IsOpen()) {
		return false;
	}
	ZeroCopyInputStreamPtr ptr(&input);
	return ReadToString(ptr, content);
}

bool StreamUtil::WriteStringToFile(const std::string& content, const std::string& filename) {
	FileOutputStream output(filename);
	if (!output.IsOpen()) {
		return false;
	}
	return output.WriteRaw(content.data(), static_cast<int>(content.size()));
}

bool StreamUtil::CopyStream(ZeroCopyInputStreamPtr input, ZeroCopyOutputStreamPtr output) {
	const void* data;
	int size;

	while (input->Next(&data, &size)) {
		if (!output->WriteRaw(data, size)) {
			return false;
		}
	}

	return true;
}

}
