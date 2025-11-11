#ifndef CODE_GENERATOR_FILE_STREAMS_H
#define CODE_GENERATOR_FILE_STREAMS_H

#include "zero_copy_stream.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <memory>

namespace code_generator {

class FileOutputStream : public ZeroCopyOutputStream {
public:
    explicit FileOutputStream(const boost::filesystem::path& filename, 
                             int buffer_size = 8192);
    explicit FileOutputStream(FILE* file, int buffer_size = 8192, bool take_ownership = false);
    ~FileOutputStream() override;
    
    bool Next(void** data, int* size) override;
    void BackUp(int count) override;
    std::int64_t ByteCount() const override;
    bool Flush() override;
    
    bool IsOpen() const { return file_ != nullptr; }
    const boost::filesystem::path& GetFilename() const { return filename_; }
    
private:
    boost::filesystem::path filename_;
    FILE* file_;
    bool own_file_;
    std::vector<char> buffer_;
    int buffer_offset_;
    std::int64_t total_bytes_;
    
    bool FlushBuffer();
};

class FileInputStream : public ZeroCopyInputStream {
public:
    explicit FileInputStream(const boost::filesystem::path& filename, int buffer_size = 8192);
    explicit FileInputStream(FILE* file, int buffer_size = 8192, bool take_ownership = false);
    ~FileInputStream() override;
    
    bool Next(const void** data, int* size) override;
    void BackUp(int count) override;
    std::int64_t ByteCount() const override;
    
    bool IsOpen() const { return file_ != nullptr; }
    const boost::filesystem::path& GetFilename() const { return filename_; }
    bool Eof() const;
    
private:
    boost::filesystem::path filename_;
    FILE* file_;
    bool own_file_;
    std::vector<char> buffer_;
    int buffer_offset_;
    int buffer_available_;
    std::int64_t total_bytes_;
    int last_returned_size_;
    
    bool Refill();
};

// 简化的 Boost 文件输出流 - 使用标准 ofstream
class BoostFileOutputStream : public ZeroCopyOutputStream {
public:
    explicit BoostFileOutputStream(const boost::filesystem::path& filename);
    ~BoostFileOutputStream() override;
    
    bool Next(void** data, int* size) override;
    void BackUp(int count) override;
    std::int64_t ByteCount() const override;
    bool Flush() override;
    
private:
    std::ofstream stream_;
    std::vector<char> buffer_;
    int buffer_offset_;
    std::int64_t total_bytes_;
    
    bool FlushBuffer();
};

} // namespace code_generator

#endif
