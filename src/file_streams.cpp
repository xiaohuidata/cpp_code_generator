#include "code_generator/file_streams.h"
#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <cstdio>

namespace code_generator {

FileOutputStream::FileOutputStream(const boost::filesystem::path& filename, int buffer_size)
    : filename_(filename), file_(nullptr), own_file_(true),
      buffer_(buffer_size), buffer_offset_(0), total_bytes_(0) {
    
    // 确保目录存在
    auto path = filename_.parent_path();
    if (!path.empty()){
        if (!boost::filesystem::exists(path)) {
            boost::filesystem::create_directories(filename_.parent_path());
        }
    }
    
    file_ = fopen(filename_.string().c_str(), "wb");
    if (!file_) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open file: " + filename_.string()));
    }
}

FileOutputStream::FileOutputStream(FILE* file, int buffer_size, bool take_ownership)
    : file_(file), own_file_(take_ownership),
      buffer_(buffer_size), buffer_offset_(0), total_bytes_(0) {
}

FileOutputStream::~FileOutputStream() {
    if (buffer_offset_ > 0) {
        FlushBuffer();
    }
    if (file_ && own_file_) {
        fclose(file_);
    }
}

bool FileOutputStream::Next(void** data, int* size) {
    if (buffer_offset_ == buffer_.size()) {
        if (!FlushBuffer()) {
            return false;
        }
    }
    
    *data = buffer_.data() + buffer_offset_;
    *size = static_cast<int>(buffer_.size() - buffer_offset_);
    buffer_offset_ = static_cast<int>(buffer_.size());
    return true;
}

void FileOutputStream::BackUp(int count) {
    if (count > 0 && count <= buffer_offset_) {
        buffer_offset_ -= count;
    }
}

std::int64_t FileOutputStream::ByteCount() const {
    return total_bytes_ + buffer_offset_;
}

bool FileOutputStream::Flush() {
    return FlushBuffer();
}

bool FileOutputStream::FlushBuffer() {
    if (buffer_offset_ > 0) {
        size_t written = fwrite(buffer_.data(), 1, buffer_offset_, file_);
        if (written != static_cast<size_t>(buffer_offset_)) {
            return false;
        }
        total_bytes_ += buffer_offset_;
        buffer_offset_ = 0;
        
        return fflush(file_) == 0;
    }
    return true;
}

FileInputStream::FileInputStream(const boost::filesystem::path& filename, int buffer_size)
    : filename_(filename), file_(nullptr), own_file_(true),
      buffer_(buffer_size), buffer_offset_(0), buffer_available_(0),
      total_bytes_(0), last_returned_size_(0) {
    
    file_ = fopen(filename_.string().c_str(), "rb");
    if (!file_) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open file: " + filename_.string()));
    }
}

FileInputStream::FileInputStream(FILE* file, int buffer_size, bool take_ownership)
    : file_(file), own_file_(take_ownership),
      buffer_(buffer_size), buffer_offset_(0), buffer_available_(0),
      total_bytes_(0), last_returned_size_(0) {
}

FileInputStream::~FileInputStream() {
    if (file_ && own_file_) {
        fclose(file_);
    }
}

bool FileInputStream::Next(const void** data, int* size) {
    if (buffer_offset_ >= buffer_available_) {
        if (!Refill()) {
            return false;
        }
    }
    
    *data = buffer_.data() + buffer_offset_;
    last_returned_size_ = buffer_available_ - buffer_offset_;
    buffer_offset_ = buffer_available_;
    *size = last_returned_size_;
    
    return true;
}

void FileInputStream::BackUp(int count) {
    if (count > 0 && count <= last_returned_size_) {
        buffer_offset_ -= count;
        last_returned_size_ = 0;
    }
}

std::int64_t FileInputStream::ByteCount() const {
    return total_bytes_ + buffer_offset_;
}

bool FileInputStream::Eof() const {
    return file_ ? feof(file_) != 0 : true;
}

bool FileInputStream::Refill() {
    if (buffer_offset_ < buffer_available_) {
        return true;
    }
    
    size_t read_bytes = fread(buffer_.data(), 1, buffer_.size(), file_);
    if (read_bytes == 0) {
        return false;
    }
    
    buffer_offset_ = 0;
    buffer_available_ = static_cast<int>(read_bytes);
    return true;
}

// 修复的 BoostFileOutputStream 实现 - 使用标准 ofstream
BoostFileOutputStream::BoostFileOutputStream(const boost::filesystem::path& filename)
    : stream_(), buffer_(4096), buffer_offset_(0), total_bytes_(0) {
    
    // 确保目录存在
    boost::filesystem::create_directories(filename.parent_path());
    
    // 使用标准 ofstream 打开文件
    stream_.open(filename.string(), std::ios::binary | std::ios::out);
    if (!stream_.is_open()) {
        BOOST_THROW_EXCEPTION(std::runtime_error("Cannot open file: " + filename.string()));
    }
}

BoostFileOutputStream::~BoostFileOutputStream() {
    if (buffer_offset_ > 0) {
        FlushBuffer();
    }
    if (stream_.is_open()) {
        stream_.close();
    }
}

bool BoostFileOutputStream::Next(void** data, int* size) {
    if (buffer_offset_ == buffer_.size()) {
        if (!FlushBuffer()) {
            return false;
        }
    }
    
    *data = buffer_.data() + buffer_offset_;
    *size = static_cast<int>(buffer_.size() - buffer_offset_);
    buffer_offset_ = static_cast<int>(buffer_.size());
    return true;
}

void BoostFileOutputStream::BackUp(int count) {
    if (count > 0 && count <= buffer_offset_) {
        buffer_offset_ -= count;
    }
}

std::int64_t BoostFileOutputStream::ByteCount() const {
    return total_bytes_ + buffer_offset_;
}

bool BoostFileOutputStream::Flush() {
    return FlushBuffer();
}

bool BoostFileOutputStream::FlushBuffer() {
    if (buffer_offset_ > 0) {
        stream_.write(buffer_.data(), buffer_offset_);
        if (!stream_.good()) {
            return false;
        }
        total_bytes_ += buffer_offset_;
        buffer_offset_ = 0;
        stream_.flush();
    }
    return true;
}

} // namespace code_generator
