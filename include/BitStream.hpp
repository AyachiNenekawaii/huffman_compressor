#ifndef BITSTREAM_HPP
#define BITSTREAM_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace huffman {

class BitOutputStream {
private:
    std::vector<uint8_t> buffer; // 字节缓冲区
    uint8_t currentByte;         // 当前正在写入的字节
    int bitCount;                  // 当前字节中已写入的位数

public:
    BitOutputStream();
    ~BitOutputStream();

    void writeBit(bool bit);
    void writeBits(std::vector<bool> bits);

    void writeByte(uint8_t byte);
    void writeBytes(std::vector<uint8_t> bytes);

    void flush();
    void clear();

    std::vector<uint8_t> getBuffer() const;
    int getBitCount() const;
    void writeToFile(const std::string& filename);
};


class BitInputStream {
private:
    std::vector<uint8_t> buffer; // 字节缓冲区
    int byteIndex;               // 当前字节序列的索引
    int bitIndex;                // 当前位在字节中的索引

public:
    BitInputStream();
    BitInputStream(const std::vector<uint8_t>& data);
    ~BitInputStream();

    void loadFromFile(const std::string& filename);
    void setBuffer(const std::vector<uint8_t>& data);

    bool readBit();
    std::vector<bool> readBits(size_t count);

    uint8_t readByte();
    std::vector<uint8_t> readBytes(size_t count);

    bool hasMoreBits() const;
    size_t getRemainingBits() const;
    size_t getCurrentPosition() const;
    size_t getSize() const;

    void reset();
    void clear();
};

}

#endif // BITSTREAM_HPP
