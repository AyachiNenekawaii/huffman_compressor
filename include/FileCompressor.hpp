#ifndef FILECOMPRESSOR_HPP
#define FILECOMPRESSOR_HPP

#include "HuffmanTree.hpp"
#include "BitStream.hpp"
#include <chrono>

namespace huffman {

// 压缩统计信息
struct CompressionStats {
    std::string sourcePath;             // 源文件路径
    std::string outputPath;             // 输出文件路径
    size_t originalSize;                // 原始大小（字节）
    size_t compressedSize;              // 压缩后大小（字节）
    double compressionRatio;            // 压缩率
    double compressionPercentage;       // 压缩百分比
    std::chrono::milliseconds duration; // 耗时
    size_t huffmanTreeSize;             // 哈夫曼树大小

    CompressionStats()
        : originalSize(0), compressedSize(0), compressionRatio(0.0),
        compressionPercentage(0.0), huffmanTreeSize(0) {}

    void calculateRatio() {
        if (originalSize > 0) {
            compressionRatio = static_cast<double>(compressedSize) / originalSize;
            compressionPercentage  = (1.0 - compressionRatio) * 100.0;
        }
    }

    void print() const;
};

// 文件压缩器类
class FileCompressor {
private:
    HuffmanTree huffmanTree;
    CompressionStats stats;

    // 读取文件内容
    std::vector<uint8_t> readFile(const std::string& fileName);

    // 写入文件内容
    void writeFile(const std::string& fileName, const std::vector<uint8_t>& data);

public:
    FileCompressor();
    ~FileCompressor();

    // 压缩单个文件
    // compressedData 输出参数，返回压缩后的数据
    CompressionStats compress(const std::string& sourcePath,
        std::vector<uint8_t>& compresedData);

    // 压缩并直接写入文件
    CompressionStats compressToFile(const std::string& sourcePath,
        const std::string& outputPath);

    // 解压数据
    // outputPath: 输出文件路径，如果为空则只返回数据
    CompressionStats decompress(const std::vector<uint8_t>& compressedData,
        const std::string& outputPath, std::vector<uint8_t>& decompressedData);
    
    // 从文件解压
    CompressionStats decompressFromFile(const std::string& compressPath,
        const std::string& outputPath);

    // 获取最后一次压缩/解压的统计信息
    CompressionStats getStats() const;

    // 清空状态
    void clear();
};

}

#endif // FILECOMPRESSOR_HPP
