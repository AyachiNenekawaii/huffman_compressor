#ifndef HUFFMANARCHIVER_HPP
#define HUFFMANARCHIVER_HPP

#include "FileCompressor.hpp"
#include "DirectoryCompressor.hpp"
#include <functional>
#include <string>

namespace huffman {

// 进度回调函数类型定义
typedef std::function<void(const std::string& currentFile,
                          size_t current, 
                          size_t total)> ProgressCallback;

// 压缩模式
enum class CompressMode {
    AUTO,        // 自动检测（默认）
    FILE_ONLY,   // 仅文件模式
    DIRECTORY    // 目录模式
};

// 哈夫曼压缩主类
class HuffmanArchiver {
private:
    std::unique_ptr<FileCompressor> fileCompressor;
    std::unique_ptr<DirectoryCompressor> dirCompressor;

    // 检查是否为目录
    bool isDirectory(const std::string& path);
    
    // 检查文件是否存在
    bool fileExists(const std::string& path);
    
    // 获取文件扩展名
    std::string getExtension(const std::string& path);
    
    // 移除文件扩展名
    std::string removeExtension(const std::string& path);

public:
    HuffmanArchiver();
    ~HuffmanArchiver();

    // 压缩文件或目录
    // source: 源文件或目录路径
    // output: 输出文件路径（如果为空，自动生成）
    // mode: 压缩模式
    // 返回值: 是否成功
    bool compress(const std::string& source,
                  const std::string& output = "",
                  CompressMode mode = CompressMode::AUTO);

    // 解压文件或目录
    // source: 压缩文件路径
    // output: 输出路径（如果为空，自动生成）
    // 返回值: 是否成功
    bool decompress(const std::string& source,
                    const std::string& output = "");

    // 获取文件压缩统计信息
    CompressionStats getFileStats() const;

    // 获取目录压缩统计信息
    DirectoryCompressionStats getDirectoryStats() const;

    // 设置进度回调
    void setProgressCallback(ProgressCallback callback);

    // 获取版本信息
    static std::string getVersion();

    // 打印程序信息
    static void printInfo();
};

}

#endif // HUFFMANARCHIVER_HPP