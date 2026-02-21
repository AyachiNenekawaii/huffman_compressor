#include "Packer.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace huffman {

std::vector<uint8_t> Packer::readFile(const std::string& filename) {
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

void Packer::writeFile(const std::string& fileName, const std::vector<uint8_t>& data) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file) {
        throw std::runtime_error("create file failed: " + fileName);
    }

    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        throw std::runtime_error("write file failed: " + fileName);
    }
}

void Packer::createDirectory(const std::string& path) {
    fs::create_directories(path);
}

std::string Packer::getFilename(const std::string& path) {
    return fs::path(path).filename().string();
}
 
std::string Packer::combinePath(const std::string& dir, const std::string& file) {
    return (fs::path(dir) / file).string();
}

void Packer::traverseDirectory(const std::string& path, const std::string& relativePath, 
                               std::vector<DirectoryEntry>& entries) {
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string entryRelativePath = combinePath(relativePath,
                getFilename(entry.path().string()));
            
            if (fs::is_directory(entry)) {
                entries.emplace_back(EntryType::DIR, entryRelativePath);

                // 递归遍历子目录
                traverseDirectory(entry.path().string(), entryRelativePath, entries);
            } else if (fs::is_regular_file(entry)) {
                entries.emplace_back(EntryType::FILE, std::move(entryRelativePath),
                    fs::file_size(entry), readFile(entry.path().string()));
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "traverseDirectory error: " << e.what() << std::endl;
    }
}

void Packer::serializeEntry(const DirectoryEntry& entry, BitOutputStream& bitStream) {
    // 写入类型
    bitStream.writeByte(static_cast<uint8_t>(entry.type));

    // 写入相对路径长度（2字节）
    uint64_t pathLength = entry.relativePath.size();
    bitStream.writeByte(static_cast<uint8_t>((pathLength >> 8) & 0xFF));
    bitStream.writeByte(static_cast<uint8_t>(pathLength & 0xFF));

    // 写入相对路径
    for (char c : entry.relativePath) {
        bitStream.writeByte(static_cast<uint8_t>(c));
    }

    // 如果是文件，写入文件信息（大端序）
    if (entry.type == EntryType::FILE) {
        // 写入文件大小
        for (int i = 7; i >= 0; i--) {
            bitStream.writeByte(static_cast<uint8_t>((entry.size >> (i * 8)) & 0xFF));
        }

        // 写入数据
        bitStream.writeBytes(entry.data);
    }
}

DirectoryEntry Packer::deserializeEntry(BitInputStream& bitStream) {
    DirectoryEntry entry;
    
    // 读取类型
    entry.type = static_cast<EntryType>(bitStream.readByte());
    // 如果是结束标记，直接返回
    if (entry.type == EntryType::END) {
        return entry;
    }
    
    // 读取相对路径长度
    uint16_t pathLength = (static_cast<uint16_t>(bitStream.readByte()) << 8) |
                          bitStream.readByte();

    // 读取相对路径
    entry.relativePath.clear();
    for (uint16_t i = 0; i < pathLength; i++) {
        entry.relativePath += static_cast<char>(bitStream.readByte());
    }

    // 如果是文件，读取文件信息
    if (entry.type == EntryType::FILE) {
        // 读取文件原始大小
        entry.size = 0;
        for (int i = 0; i < 8; i++) {
            entry.size = (entry.size << 8) | bitStream.readByte();
        }

        // 读取数据
        entry.data = bitStream.readBytes(entry.size);
    }

    return entry;
}

std::vector<uint8_t> Packer::pack(const std::vector<std::string>& sources) {
    // 检查源路径是否存在
    for (const auto& source : sources) {
        if (!fileExists(source)) {
            throw std::runtime_error("file not exists: " + source);
        }
    }

    std::vector<DirectoryEntry> entries;

    // 遍历所有源路径，收集目录项
    for (const auto& source : sources) {
        if (isDirectory(source)) {
            // 递归遍历目录
            traverseDirectory(source, "", entries);
        } else if(fs::is_regular_file(source)) {
            // 直接添加文件条目
            entries.emplace_back(EntryType::FILE, getFilename(source),
                getFileSize(source), readFile(source));
        }
    }

    // 添加结束标记
    entries.emplace_back(EntryType::END);

    // 序列化所有目录项
    BitOutputStream bitStream;
    for (const auto& entry : entries) {
        serializeEntry(entry, bitStream);
    }

    return bitStream.getBuffer();
}

void Packer::unpack(const std::vector<uint8_t>& packedData, const std::string& outputDir) {
    // 检查输出目录是否存在，不存在则创建
    if (!isDirectory(outputDir)) {
        createDirectory(outputDir);
    }

    // 反序列化目录项
    BitInputStream bitStream(packedData);
    while (true) {
        DirectoryEntry entry = deserializeEntry(bitStream);
        if (entry.type == EntryType::END) {
            break;
        }

        // 构建完整路径
        std::string fullPath = combinePath(outputDir, entry.relativePath);

        if (entry.type == EntryType::DIR) {
            // 创建目录（如果是目录）
            createDirectory(fullPath);
        } else if (entry.type == EntryType::FILE) {
            // 写入文件
            writeFile(fullPath, entry.data);
        }
    }
}

bool Packer::isDirectory(const std::string& path) {
    return fs::is_directory(path);
}

bool Packer::fileExists(const std::string& path) {
    return fs::exists(path);
}

uint64_t Packer::getFileSize(const std::string& path) {
    if (fs::exists(path) && fs::is_regular_file(path)) {
        return fs::file_size(path);
    }
    return 0;
}

}