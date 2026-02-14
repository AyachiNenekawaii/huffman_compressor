#include "DirectoryCompressor.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace huffman {

// 目录压缩文件格式：
// [8字节: 目录项数量]
// [8字节: 原始总大小]
// [8字节: 压缩后总大小]
// [目录项列表...]
//
// 每个目录项格式：
// [1字节: 类型 (0x01=文件, 0x02=目录)]
// [2字节: 相对路径长度]
// [N字节: 相对路径]
// [如果是文件:]
//   [8字节: 原始文件大小]
//   [8字节: 压缩数据大小]
//   [M字节: 压缩数据]

void DirectoryCompressionStats::print() const {
    std::cout << "========== 目录压缩统计 ==========" << std::endl;
    std::cout << "源目录: " << sourcePath << std::endl;
    std::cout << "输出文件: " << outputPath << std::endl;
    std::cout << "文件数量: " << totalFiles << std::endl;
    std::cout << "目录数量: " << totalDirectories << std::endl;
    std::cout << "原始总大小: " << originalSize << " 字节 (" 
              << std::fixed << std::setprecision(2) 
              << originalSize / 1024.0 / 1024.0 << " MB)" << std::endl;
    std::cout << "压缩后大小: " << compressedSize << " 字节 ("
              << compressedSize / 1024.0 / 1024.0 << " MB)" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "压缩率: " << compressionRatio << std::endl;
    std::cout << "压缩百分比: " << compressionPercentage << "%" << std::endl;
    std::cout   << "耗时: " << duration.count() << " 毫秒" << std::endl;
    std::cout << "==================================" << std::endl;
}

DirectoryCompressor::DirectoryCompressor() = default;

DirectoryCompressor::~DirectoryCompressor() = default;

void DirectoryCompressor::setProgressCallback(ProgressCallback callback) {
    progressCallback = callback;
}

bool DirectoryCompressor::isDirectory(const std::string& path) {
    return fs::is_directory(path);
}

bool DirectoryCompressor::fileExists(const std::string& path) {
    return fs::exists(path);
}

uint64_t DirectoryCompressor::getFileSize(const std::string& path) {
    if (fs::exists(path) && fs::is_regular_file(path)) {
        return fs::file_size(path);
    }
}

std::string DirectoryCompressor::getFileName(const std::string& path) {
    return fs::path(path).filename().string();
}

std::string DirectoryCompressor::combinePath(const std::string& dir,
                                             const std::string& file) {
    return (fs::path(dir) / file).string();
}

void DirectoryCompressor::createDirectory(const std::string& path) {
    fs::create_directories(path);
}

void DirectoryCompressor::traverseDirectory(const std::string& path,
                                            const std::string& relativePath,
                                            std::vector<DirectoryEntry>& entries) {
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string entryRelativePath = combinePath(relativePath,
                                                        getFileName(entry.path().string()));
            if (fs::is_directory(entry)) {
                DirectoryEntry dirEntry;
                dirEntry.type = EntryType::DIRECTORY_ENTRY;
                dirEntry.relativePath = entryRelativePath;
                entries.push_back(dirEntry);

                // 递归遍历子目录
                traverseDirectory(entry.path().string(), entryRelativePath, entries);
            } else if (fs::is_regular_file(entry)) {
                DirectoryEntry fileEntry;
                fileEntry.type = EntryType::FILE_ENTRY;
                fileEntry.relativePath = entryRelativePath;
                fileEntry.size = fs::file_size(entry);
                entries.push_back(fileEntry);
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "遍历目录时出错：" << e.what() << std::endl;
    }
}

void DirectoryCompressor::serializeEntry(const DirectoryEntry& entry,
                                         BitOutputStream& bitStream) {
    // 写入类型
    bitStream.writeByte(static_cast<uint8_t>(entry.type));

    // 写入相对路径长度（2字节）
    uint64_t pathLength = entry.relativePath.length();
    bitStream.writeByte(static_cast<uint8_t>((pathLength >> 8) & 0xFF));
    bitStream.writeByte(static_cast<uint8_t>(pathLength & 0xFF));

    // 写入相对路径
    for (char c : entry.relativePath) {
        bitStream.writeByte(static_cast<uint8_t>(c));
    }

    // 如果是文件，写入文件信息（大端序）
    if (entry.type == EntryType::FILE_ENTRY) {
        // 写入原始文件大小
        for (int i = 7; i >= 0; i--) {
            bitStream.writeByte(static_cast<uint8_t>((entry.size >> (i * 8)) & 0xFF));
        }

        // 写入压缩数据大小
        uint64_t compressedSize = entry.compressedData.size();
        for (int i = 7; i >= 0; i--) {
            bitStream.writeByte(static_cast<uint8_t>((compressedSize >> (i * 8)) & 0xFF));
        }

        // 写入压缩数据
        bitStream.writeBytes(entry.compressedData);
    }
}

DirectoryEntry DirectoryCompressor::deserializeEntry(BitInputStream& bitStream) {
    DirectoryEntry entry;
    
    // 读取类型
    entry.type = static_cast<EntryType>(bitStream.readByte());
    
    // 读取相对路径长度
    uint16_t pathLength = static_cast<uint16_t>(bitStream.readByte() << 8) |
                          bitStream.readByte();

    // 读取相对路径
    entry.relativePath.clear();
    for (uint16_t i = 0; i < pathLength; i++) {
        entry.relativePath += static_cast<char>(bitStream.readByte());
    }

    // 如果是文件，读取文件信息
    if (entry.type == EntryType::FILE_ENTRY) {
        // 读取文件原始大小
        entry.size = 0;
        for (int i = 0; i < 8; i++) {
            entry.size = (entry.size << 8) | bitStream.readByte();
        }

        // 读取压缩数据大小
        uint64_t compressedSize = 0;
        for (int i = 0; i < 8; i++) {
            compressedSize = (compressedSize << 8) | bitStream.readByte();
        }

        // 读取压缩数据
        entry.compressedData = bitStream.readBytes(compressedSize);
    }

    return entry;
}

DirectoryCompressionStats DirectoryCompressor::compress(const std::string& sourceDir,
                                                        const std::string& outputFile) {
    auto start = std::chrono::high_resolution_clock::now();

    if (!isDirectory(sourceDir)) {
        throw std::runtime_error("源路径不是目录：" + sourceDir);
    }

    // 遍历目录获取所有条目
    std::vector<DirectoryEntry> entries;
    traverseDirectory(sourceDir, "", entries);

    // 分离文件和目录
    std::vector<DirectoryEntry*> fileEntries;
    size_t dirCount = 0;
    for (auto& entry : entries) {
        if (entry.type == EntryType::FILE_ENTRY) {
            fileEntries.push_back(&entry);
        } else {
            dirCount++;
        }
    }

    // 压缩每个文件
    uint64_t totalOriginalSize = 0;
    uint64_t totalCompressedSize = 0;

    for (size_t i = 0; i < fileEntries.size(); i++) {
        auto& entry = *fileEntries[i];
        std::string fullPath = combinePath(sourceDir, entry.relativePath);

        // 设置进度回调
        if (progressCallback) {
            progressCallback(entry.relativePath, i + 1, fileEntries.size());
        }

        // 压缩文件
        fileCompressor.compress(fullPath, entry.compressedData);

        totalOriginalSize += entry.size;
        totalCompressedSize += entry.compressedData.size();
    }

    // 构建压缩文件
    BitOutputStream bitStream;

    // 写入条目数量
    uint64_t entryCount = entries.size();
    for (int i = 7; i >= 0; i--) {
        bitStream.writeByte(static_cast<uint8_t>((entryCount >> (i * 8)) & 0xFF));
    }

    // 写入原始总大小
    for (int i = 7; i >= 0; i--) {
        bitStream.writeByte(static_cast<uint8_t>((totalOriginalSize >> (i * 8)) & 0xFF));
    }

    // 预留压缩总大小位置（稍后更新）
    size_t compressedSizePos = bitStream.getBuffer().size();
    for (int i = 0; i < 8; i++) {
        bitStream.writeByte(0);
    }

    // 序列化所有条目
    for (const auto& entry : entries) {
        serializeEntry(entry, bitStream);
    }

    bitStream.flush();

    // 获取最终数据并更新压缩总大小
    std::vector<uint8_t> finalData = bitStream.getBuffer();
    uint64_t finalCompressedSize = finalData.size();

    // 更新压缩总大小（在位置compressedSizePos处）
    for (int i = 0; i < 8; i++) {
        finalData[compressedSizePos + i] = static_cast<uint8_t>((finalCompressedSize >> ((7 - i) * 8)) & 0xFF);
    }

    // 写入文件
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("无法创建输出文件: " + outputFile);
    }
    outFile.write(reinterpret_cast<const char*>(finalData.data()), finalData.size());
    outFile.close();

    // 计算统计信息
    auto end = std::chrono::high_resolution_clock::now();
    
    stats = DirectoryCompressionStats();
    stats.sourcePath = sourceDir;
    stats.outputPath = outputFile;
    stats.totalFiles = fileEntries.size();
    stats.totalDirectories = dirCount;
    stats.originalSize = totalOriginalSize;
    stats.compressedSize = finalCompressedSize;
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    stats.calculateRatio();

    return stats;
}

DirectoryCompressionStats DirectoryCompressor::decompress(const std::string& compressedFile,
                                                          const std::string& outputDir) {
    auto start = std::chrono::high_resolution_clock::now();

    if (!fileExists(compressedFile)) {
        throw std::runtime_error("压缩文件不存在: " + compressedFile);
    }

    // 读取压缩文件
    std::ifstream inFile(compressedFile, std::ios::binary | std::ios::ate);
    if (!inFile) {
        throw std::runtime_error("无法打开压缩文件: " + compressedFile);
    }

    std::streamsize fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    std::vector<uint8_t> fileData(fileSize);
    if (!inFile.read(reinterpret_cast<char*>(fileData.data()), fileSize)) {
        throw std::runtime_error("读取压缩文件失败: " + compressedFile);
    }
    inFile.close();

    // 解析压缩文件
    BitInputStream bitStream(fileData);
    
    // 读取条目数量
    uint64_t entryCount = 0;
    for (int i = 0; i < 8; i++) {
        entryCount = (entryCount << 8) | bitStream.readByte();
    }
    
    // 读取原始总大小
    uint64_t totalOriginalSize = 0;
    for (int i = 0; i < 8; i++) {
        totalOriginalSize = (totalOriginalSize << 8) | bitStream.readByte();
    }
    
    // 读取压缩总大小（跳过）
    for (int i = 0; i < 8; i++) {
        bitStream.readByte();
    }

    // 创建输出目录
    createDirectory(outputDir);

    // 解压所有条目
    size_t fileCount = 0;
    size_t dirCount = 0;
    
    for (uint64_t i = 0; i < entryCount; i++) {
        DirectoryEntry entry = deserializeEntry(bitStream);
        std::string fullPath = combinePath(outputDir, entry.relativePath);
        
        if (progressCallback) {
            progressCallback(entry.relativePath, i + 1, entryCount);
        }
        
        if (entry.type == EntryType::DIRECTORY_ENTRY) {
            createDirectory(fullPath);
            dirCount++;
        } else {
            // 解压文件
            std::vector<uint8_t> decompressedData;
            fileCompressor.decompress(entry.compressedData, fullPath, decompressedData);
            fileCount++;
        }
    }

    // 计算统计信息
    auto end = std::chrono::high_resolution_clock::now();
    
    stats = DirectoryCompressionStats();
    stats.sourcePath = compressedFile;
    stats.outputPath = outputDir;
    stats.totalFiles = fileCount;
    stats.totalDirectories = dirCount;
    stats.originalSize = totalOriginalSize;
    stats.compressedSize = fileSize;
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    stats.calculateRatio();

    return stats;
}

DirectoryCompressionStats DirectoryCompressor::getStats() const {
    return stats;
}

void DirectoryCompressor::clear() {
    fileCompressor.clear();
    stats = DirectoryCompressionStats();
}

}