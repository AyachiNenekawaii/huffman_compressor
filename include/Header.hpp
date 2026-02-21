#ifndef HEADER_HPP
#define HEADER_HPP

#include <cstdint>

constexpr uint32_t MAGIC_NUMBER = 0x48554646; // "HUFF"

// 压缩文件格式：
// [4字节：Magic Number]
// [2字节：标志位]
// [2字节: 哈夫曼树大小]
// [8字节: 原始文件大小]
// [8字节: 压缩文件大小]
// [N字节: 哈夫曼树数据]
// [M字节: 压缩后的文件内容]

struct Header {
    uint32_t magicNumber;
    uint16_t flags;
    uint16_t treeSize;
    uint64_t originalSize;
    uint64_t compressedSize;

    Header() 
        : magicNumber(MAGIC_NUMBER)
        , flags(0)
        , treeSize(0)
        , originalSize(0)
        , compressedSize(0) 
    {}
};

constexpr uint8_t HEADER_SIZE = sizeof(Header);

#endif // HEADER_HPP