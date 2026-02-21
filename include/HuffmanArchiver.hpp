#ifndef HUFFMANARCHIVER_HPP
#define HUFFMANARCHIVER_HPP

#include "FileCompressor.hpp"
#include "Packer.hpp"
#include <functional>
#include <string>

namespace huffman {

// 进度回调函数类型定义
typedef std::function<void(const std::string& currentFile,
                           size_t current, 
                           size_t total)> ProgressCallback;

// 哈夫曼压缩主类
class HuffmanArchiver {
private:
    std::unique_ptr<Packer> packer;
    std::unique_ptr<FileCompressor> fileCompressor;
    
    // 获取文件扩展名
    std::string getExtension(const std::string& path);
    
    // 移除文件扩展名
    std::string removeExtension(const std::string& path);

public:
    HuffmanArchiver();
    ~HuffmanArchiver();

    // 压缩文件或目录
    // sources: 源文件或目录路径列表
    // output: 输出文件路径（如果为空，自动生成）
    // 返回值: 是否成功
    bool compress(const std::vector<std::string>& sources, const std::string& output = "");

    // 解压文件或目录
    // source: 压缩文件路径
    // output: 输出路径（如果为空，自动生成）
    // 返回值: 是否成功
    bool decompress(const std::string& source, const std::string& output = "");

    // 获取版本信息
    static std::string getVersion();

    // 打印程序信息
    static void printInfo();
};

}

#endif // HUFFMANARCHIVER_HPP