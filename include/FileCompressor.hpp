#ifndef FILECOMPRESSOR_HPP
#define FILECOMPRESSOR_HPP

#include "HuffmanTree.hpp"
#include "BitStream.hpp"
#include "Header.hpp"

namespace huffman {

// 文件压缩器类
class FileCompressor {
private:
    Header header;
    HuffmanTree huffmanTree;

    // 读取文件内容
    std::vector<uint8_t> readFile(const std::string& filename);

    // 写入文件
    void writeFile(const std::string& filename, const std::vector<uint8_t>& data);

    // 设置头信息
    void setHeader(uint16_t treeSize, uint64_t originalSize, uint64_t compressedSize);

    // 从压缩数据中设置头信息
    void setHeader(const std::vector<uint8_t>& compressedData);

public:
    FileCompressor() = default;
    ~FileCompressor() = default;

    // 压缩数据
    std::vector<uint8_t> compress(const std::vector<uint8_t>& originalData);
    
    // 解压数据
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressedData);

    // 从文件压缩数据
    void compressToFile(const std::vector<uint8_t>& originalData, const std::string& output);

    // 从文件解压数据
    void decompressFromFile(const std::string& input, std::vector<uint8_t>& data);

    // 清空状态
    void clear();
};

}

#endif // FILECOMPRESSOR_HPP