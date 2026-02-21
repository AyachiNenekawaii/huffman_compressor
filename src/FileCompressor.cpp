#include "FileCompressor.hpp"
#include <stdexcept>

namespace huffman {

std::vector<uint8_t> FileCompressor::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("open file failed: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("read file failed: " + filename);
    }

    return buffer;
}

void FileCompressor::writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("create file failed: " + filename);
    }

    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        throw std::runtime_error("write file failed: " + filename);
    }
}

void FileCompressor::setHeader(uint16_t treeSize, uint64_t originalSize, uint64_t compressedSize) {
    header.treeSize = treeSize;
    header.originalSize = originalSize;
    header.compressedSize = compressedSize;
}

void FileCompressor::setHeader(const std::vector<uint8_t>& compressedData) {
    if (compressedData.size() < HEADER_SIZE) {
        throw std::runtime_error("invalid compressed data");
    }

    // 从压缩数据中读取头信息
    std::copy(compressedData.begin(), compressedData.begin() + HEADER_SIZE,
        reinterpret_cast<uint8_t*>(&header));
    
    // 检查数据是否有效
    if (header.magicNumber != MAGIC_NUMBER 
        || header.treeSize + header.compressedSize + HEADER_SIZE > compressedData.size()) {
        throw std::runtime_error("invalid compressed data");
    }
}

std::vector<uint8_t> FileCompressor::compress(const std::vector<uint8_t>& originalData) {
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

    // 获取压缩后的内容
    std::vector<uint8_t> compressedContent = bitStream.getBuffer();

    // 设置头信息
    setHeader(treeData.size(), originalData.size(), compressedContent.size());

    // 构建压缩文件
    std::vector<uint8_t> compressedData;

    // 写入头信息
    compressedData.insert(compressedData.end(), 
        reinterpret_cast<const uint8_t*>(&header), 
        reinterpret_cast<const uint8_t*>(&header) + sizeof(Header));

    // 写入哈夫曼树数据
    compressedData.insert(compressedData.end(), treeData.begin(), treeData.end());

    // 写入压缩后的内容
    compressedData.insert(compressedData.end(),
        compressedContent.begin(), compressedContent.end());

    return compressedData;
}

std::vector<uint8_t> FileCompressor::decompress(const std::vector<uint8_t>& compressedData) {
    // 获取头信息
    setHeader(compressedData);

    // 读取哈夫曼树
    std::vector<uint8_t> treeData(compressedData.begin() + sizeof(Header),
        compressedData.begin() + sizeof(Header) + header.treeSize);
    huffmanTree.deserialize(treeData);
    
    // 读取压缩数据
    std::vector<uint8_t> compressedContent(compressedData.begin() + sizeof(Header) + header.treeSize,
        compressedData.end());
    
    // 解压数据
    BitInputStream bitStream(compressedContent);

    std::vector<uint8_t> decompressedData;
    decompressedData.reserve(header.originalSize);

    std::shared_ptr<HuffmanNode> currentNode = huffmanTree.getRoot();

    if (currentNode->isLeaf) { // 特殊情况：只有一个字符
        decompressedData.resize(header.originalSize, currentNode->data);
    } else {
        while (decompressedData.size() < header.originalSize && bitStream.hasMoreBits()) {
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

    return decompressedData;
}

void FileCompressor::compressToFile(const std::vector<uint8_t>& originalData, const std::string& output) {
    // 压缩数据
    std::vector<uint8_t> compressedData = compress(originalData);

    // 写入压缩文件
    writeFile(output, compressedData);
}

void FileCompressor::decompressFromFile(const std::string& input, std::vector<uint8_t>& decompressedData) {
    // 读取压缩文件
    std::vector<uint8_t> compressedData = readFile(input);

    // 解压数据
    decompressedData = decompress(compressedData);
}

void FileCompressor::clear() {
    huffmanTree.clear();
}

}