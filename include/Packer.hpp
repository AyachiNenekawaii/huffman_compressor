#ifndef PACKER_HPP
#define PACKER_HPP

#include "BitStream.hpp"
#include <utility>
#include <functional>

namespace huffman {

// 目录项类型
enum class EntryType : uint8_t {
    END = 0x00,
    FILE = 0x01,
    DIR = 0x02
};

// 目录项信息
struct DirectoryEntry {
    EntryType type;
    std::string relativePath;  // 相对路径
    uint64_t size;             // 文件大小（仅文件）
    std::vector<uint8_t> data; // 文件数据（仅文件）

    DirectoryEntry() : type(EntryType::FILE), size(0) {}
    DirectoryEntry(EntryType type, const std::string& relativePath = "")
        : type(type)
        , relativePath(relativePath) 
    {}

    DirectoryEntry(EntryType type, const std::string& relativePath,
                   uint64_t size, std::vector<uint8_t>&& data)
        : type(type)
        , relativePath(relativePath) 
        , size(size)
        , data(std::move(data)) 
    {}
};

// 进度回调函数类型
typedef std::function<void(const std::string& currentFile,
        size_t current, size_t total)> 
        ProgressCallback;

class Packer {
private:
    ProgressCallback progressCallback;

    // 读取文件内容
    std::vector<uint8_t> readFile(const std::string& filename);

    // 写入文件
    void writeFile(const std::string& filename, const std::vector<uint8_t>& data);

    // 组合路径
    std::string combinePath(const std::string& dir, const std::string& file);

    // 遍历目录，获取所有文件和子目录
    void traverseDirectory(const std::string& path, const std::string& relativePath,
                           std::vector<DirectoryEntry>& entries);

    // 序列化目录项
    void serializeEntry(const DirectoryEntry& entry, BitOutputStream& bitStream);

    // 反序列化目录项
    DirectoryEntry deserializeEntry(BitInputStream& bitStream);

public:
    Packer() = default;
    ~Packer() = default;

    // 打包文件或目录
    std::vector<uint8_t> pack(const std::vector<std::string>& sources);

    // 解包文件或目录
    void unpack(const std::vector<uint8_t>& packedData, const std::string& outputDir);
};

}

#endif // PACKER_HPP
