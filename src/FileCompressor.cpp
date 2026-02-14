#include "FileCompressor.hpp"
#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace huffman {

// 压缩文件格式：
// [8字节: 原始文件大小]
// [8字节: 哈夫曼树大小]
// [N字节: 哈夫曼树数据]
// [M字节: 压缩后的文件内容]

void CompressionStats::print() const {
    std::cout << "========== 压缩统计 ==========" << std::endl;
    std::cout << "源文件：" << sourcePath << std::endl;
    std::cout << "输出文件：" << outputPath << std::endl;
    std::cout << "原始大小：" << originalSize << " 字节" << std::endl;
    std::cout << "压缩后大小：" << compressedSize << " 字节" << std::endl;
    std::cout << "哈夫曼树大小：" << huffmanTreeSize << " 字节" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "压缩率：" << compressionRatio << std::endl;
    std::cout << "压缩率百分比：" << compressionPercentage << "%" << std::endl;
    std::cout << "耗时：" << duration.count() << " 毫秒" << std::endl;
    std::cout << "==============================" << std::endl;
}

FileCompressor::FileCompressor() = default;

FileCompressor::~FileCompressor() = default;

std::vector<uint8_t> FileCompressor::readFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("打开文件失败：" + fileName);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("读取文件失败：" + fileName);
    }

    return buffer;
}

void FileCompressor::writeFile(const std::string& fileName, const std::vector<uint8_t>& data) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file) {
        throw std::runtime_error("创建文件失败：" + fileName);
    }

    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        throw std::runtime_error("写入文件失败：" + fileName);
    }
}

CompressionStats FileCompressor::compress(const std::string& sourcePath,
    std::vector<uint8_t>& compressedData) {
    auto start = std::chrono::high_resolution_clock::now();

    // 读取源文件
    std::vector<uint8_t> originalData = readFile(sourcePath);

    if (originalData.empty()) {
        throw std::runtime_error("源文件为空：" + sourcePath);
    }

    // 构建哈夫曼树
    huffmanTree.buildFromData(originalData);

    // 序列化哈夫曼树
    std::vector<uint8_t> treeData = huffmanTree.serialize();

    // 压缩数据
    BitOutputStream bitStream;
    for (uint8_t byte : originalData) {
        std::vector<bool> code = huffmanTree.getCode(byte);
        bitStream.writeBits(code);
    }
    bitStream.flush();

    std::vector<uint8_t> compressedContent = bitStream.getBuffer();

    // 构建压缩文件格式
    compressedData.clear();

    // 写入原始文件大小（8字节，大端序）
    uint64_t originalSize = originalData.size();
    for (int i = 7; i >= 0; i--) {
        compressedData.push_back(static_cast<uint8_t>((originalSize >> (i * 8)) & 0xFF));
    }

    // 写入哈夫曼树大小（8字节，大端序）
    uint64_t treeSize = treeData.size();
    for (int i = 7; i >= 0; i--) {
        compressedData.push_back(static_cast<uint8_t>((treeSize >> (i * 8)) & 0xFF));
    }

    // 写入哈夫曼树数据
    compressedData.insert(compressedData.end(), treeData.begin(), treeData.end());

    // 写入压缩后的内容
    compressedData.insert(compressedData.end(),
        compressedContent.begin(), compressedContent.end());

    // 计算统计信息
    auto end = std::chrono::high_resolution_clock::now();

    stats = CompressionStats();
    stats.sourcePath = sourcePath;
    stats.originalSize = originalSize;
    stats.compressedSize = compressedData.size();
    stats.huffmanTreeSize = treeSize;
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    stats.calculateRatio();

    return stats;
}

CompressionStats FileCompressor::compressToFile(const std::string& sourcePath,
    const std::string& outputPath) {
    std::vector<uint8_t> compressedData;
    CompressionStats result = compress(sourcePath, compressedData);

    writeFile(outputPath, compressedData);
    result.outputPath = outputPath;
    stats.outputPath = outputPath;

    return result;
}

CompressionStats FileCompressor::decompress(const std::vector<uint8_t>& compressedData,
                                            const std::string& outputPath, 
                                            std::vector<uint8_t>& decompressedData) {
    auto start = std::chrono::high_resolution_clock::now();

    if (compressedData.size() < 16) {
        throw std::runtime_error("压缩数据格式无效：数据太短");
    }

    // 读取原始文件大小
    uint64_t originalSize = 0;
    for (int i = 0; i < 8; i++) {
        originalSize = (originalSize << 8) | compressedData[i];
    }

    // 读取哈夫曼树大小
    uint64_t treeSize = 0;
    for (int i = 8; i < 16; i++) {
        treeSize = (treeSize << 8) | compressedData[i];
    }

    // 检查数据完整性
    if (compressedData.size() < 16 + treeSize) {
        throw std::runtime_error("压缩数据格式无效：哈夫曼树数据不完整");
    }

    // 读取哈夫曼树
    std::vector<uint8_t> treeData(compressedData.begin() + 16,
                                  compressedData.begin() + 16 + treeSize);
    huffmanTree.deserialize(treeData);
    
    // 读取压缩数据
    std::vector<uint8_t> compressedContent(compressedData.begin() + 16 + treeSize,
        compressedData.end());
    
    // 解压数据
    BitInputStream bitStream(compressedContent);
    decompressedData.clear();
    decompressedData.reserve(originalSize);

    std::shared_ptr<HuffmanNode> currentNode = huffmanTree.getRoot();

    if (currentNode->isLeaf) { // 特殊情况：只有一个字符
        decompressedData.resize(originalSize, currentNode->data);
    } else {
        while (decompressedData.size() < originalSize && bitStream.hasMoreBits()) {
            bool bit = bitStream.readBit();
            if (bit) {
                currentNode = currentNode->right;
            } else {
                currentNode = currentNode->left;
            }
            if (currentNode->isLeaf) {
                decompressedData.push_back(currentNode->data);
                currentNode = huffmanTree.getRoot();
            }
        }
    }

    // 如果指定了输出路径，写入文件
    if (!outputPath.empty()) {
        writeFile(outputPath, decompressedData);
    }

    // 计算统计信息
    auto end = std::chrono::high_resolution_clock::now();

    stats = CompressionStats();
    stats.sourcePath = "(压缩数据)";
    stats.outputPath = outputPath.empty() ? "(内存)" : outputPath;
    stats.originalSize = originalSize;
    stats.compressedSize = compressedData.size();
    stats.huffmanTreeSize = treeSize;
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    stats.calculateRatio();

    return stats;
}

CompressionStats FileCompressor::decompressFromFile(const std::string& compressedPath,
                                                    const std::string& outputPath) {
    std::vector<uint8_t> compressedData = readFile(compressedPath);
    std::vector<uint8_t> decompressedData;

    CompressionStats result = decompress(compressedData, outputPath, decompressedData);
    result.sourcePath = compressedPath;
    stats.sourcePath = compressedPath;

    return result;
}

CompressionStats FileCompressor::getStats() const {
    return stats;
}

void FileCompressor::clear() {
    huffmanTree.clear();
    stats = CompressionStats();
}

}