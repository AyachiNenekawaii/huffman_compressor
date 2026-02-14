#ifndef DIRECTORYCOMPRESSOR_HPP
#define DIRECTORYCOMPRESSOR_HPP

#include "FileCompressor.hpp"
#include <functional>

namespace huffman {

// 目录项类型
enum class EntryType : uint8_t {
    FILE_ENTRY = 0x01,
    DIRECTORY_ENTRY = 0x02
};

// 目录项信息
struct DirectoryEntry {
    EntryType type;
    std::string relativePath; // 相对路径
    uint64_t size;            // 文件大小（仅文件）
    std::vector<uint8_t> compressedData; // 压缩后的数据（仅文件）

    DirectoryEntry() : type(EntryType::FILE_ENTRY), size(0) {}
};

// 目录压缩统计信息
struct DirectoryCompressionStats {
    std::string sourcePath;
    std::string outputPath;
    size_t totalFiles;
    size_t totalDirectories;
    uint64_t originalSize;
    uint64_t compressedSize;
    double compressionRatio;
    double compressionPercentage;
    std::chrono::milliseconds duration;

    DirectoryCompressionStats()
        : totalFiles(0), totalDirectories(0), originalSize(0), compressedSize(0),
          compressionRatio(0.0), compressionPercentage(0.0) {}

    void calculateRatio() {
        if (originalSize > 0) {
            compressionRatio = static_cast<double>(compressedSize) / originalSize;
            compressionPercentage = (1.0 - compressionRatio) * 100.0;
        }
    }

    void print() const;
};

// 进度回调函数类型
typedef std::function<void(const std::string& currentFile,
    size_t current, size_t total)> ProgressCallback;

// 目录压缩器类
class DirectoryCompressor {
private:
    FileCompressor fileCompressor;
    DirectoryCompressionStats stats;
    ProgressCallback progressCallback;

    // 遍历目录，获取所有文件和子目录
    void traverseDirectory(const std::string& path, const std::string& relativePath,
        std::vector<DirectoryEntry>& entries);
    
    // 创建目录
    void createDirectory(const std::string& path);

    // 序列化目录项
    void serializeEntry(const DirectoryEntry& entry, BitOutputStream& bitStream);

    // 反序列化目录项
    DirectoryEntry deserializeEntry(BitInputStream& bitStream);

    // 获取文件名
    std::string getFileName(const std::string& path);

    // 组合路径
    std::string combinePath(const std::string& dir, const std::string& file);

public:
    DirectoryCompressor();
    ~DirectoryCompressor();

    // 设置进度回调
    void setProgressCallback(ProgressCallback callback);

    // 压缩目录
    DirectoryCompressionStats compress(const std::string& sourceDir,
        const std::string& outputFile);

    // 解压目录
    DirectoryCompressionStats decompress(const std::string&compressedFile,
        const std::string& outputDir);

    // 获取最后一次压缩/解压统计信息
    DirectoryCompressionStats getStats() const;

    // 清空状态
    void clear();

    // 检查路径是否为目录
    static bool isDirectory(const std::string& path);

    // 检查文件是否存在
    static bool fileExists(const std::string& path);

    // 获取文件大小
    static uint64_t getFileSize(const std::string& path);
};

}

#endif // DIRECTORYCOMPRESSOR_HPP
