#include "BitStream.hpp"

namespace huffman {

/*==================== BitOutputStream ======================*/

BitOutputStream::BitOutputStream() : currentByte(0), bitCount(0) {}

BitOutputStream::~BitOutputStream() = default;

void BitOutputStream::writeBit(bool bit) {
    currentByte = (currentByte << 1) | (bit ? 1 : 0);
    bitCount++;

    if (bitCount == 8) {
        buffer.push_back(currentByte);
        currentByte = 0;
        bitCount = 0;
    }
}

void BitOutputStream::writeBits(std::vector<bool> bits) {
    for (auto bit : bits) {
        writeBit(bit);
    }
}

void BitOutputStream::writeByte(uint8_t byte) {
    if (bitCount > 0) {
        currentByte = (currentByte << (8 - bitCount)) | (byte >> bitCount);
        buffer.push_back(currentByte);
        currentByte = byte & ((1 << bitCount) - 1);
    } else {
        buffer.push_back(byte);
    }
}

void BitOutputStream::writeBytes(std::vector<uint8_t> bytes) {
    for (auto byte : bytes) {
        writeByte(byte);
    }
}

void BitOutputStream::flush() {
    if (bitCount > 0) {
        currentByte = currentByte << (8 - bitCount);
        buffer.push_back(currentByte);
        currentByte = 0;
        bitCount = 0;
    }
}

void BitOutputStream::clear() {
    buffer.clear();
    currentByte = 0;
    bitCount = 0;
}

std::vector<uint8_t> BitOutputStream::getBuffer() const {
    return buffer;
}

int BitOutputStream::getBitCount() const {
    return buffer.size() * 8 + bitCount;
}

void BitOutputStream::writeToFile(const std::string& fileName) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件：" + fileName);
    }
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    file.close();
}

/*==================== BitInputStream ======================*/

BitInputStream::BitInputStream() : byteIndex(0), bitIndex(0) {}

BitInputStream::BitInputStream(const std::vector<uint8_t>& data)
    : buffer(data), byteIndex(0), bitIndex(0) {}

BitInputStream::~BitInputStream() = default;

void BitInputStream::loadFromFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("无法打开文件：" + fileName);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    buffer.resize(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("读取文件失败：" + fileName);
    }
    
    byteIndex = 0;
    bitIndex = 0;
}

void BitInputStream::setBuffer(const std::vector<uint8_t>& data) {
    buffer = data;
    byteIndex = 0;
    bitIndex = 0;
}

bool BitInputStream::readBit() {
    if (!hasMoreBits()) {
        throw std::runtime_error("尝试读取超出缓冲区范围");
    }

    bool bit = (buffer[byteIndex] >> (7 - bitIndex)) & 1;
    bitIndex++;

    if (bitIndex == 8) {
        byteIndex++;
        bitIndex = 0;
    }

    return bit;
}

std::vector<bool> BitInputStream::readBits(size_t count) {
    std::vector<bool> result;
    result.reserve(count);

    for (size_t i = 0; i < count; i++) {
        if (!hasMoreBits()) {
            break;
        }
        result.push_back(readBit());
    }

    return result;
}

uint8_t BitInputStream::readByte() {
    if (byteIndex >= buffer.size()) {
        throw std::runtime_error("尝试读取超出缓冲区范围");
    }

    if (bitIndex == 0) {
        return buffer[byteIndex++];
    } else {
        if (byteIndex + 1 >= buffer.size()) {
            throw std::runtime_error("读取字节时数据不足");
        }
        uint8_t highBits = buffer[byteIndex] & ((1 << (8 - bitIndex)) - 1);
        uint8_t lowBits = buffer[byteIndex + 1] >> (8 - bitIndex);
        byteIndex++;
        return (highBits << bitIndex) | lowBits;
    }
}

std::vector<uint8_t> BitInputStream::readBytes(size_t count) {
    std::vector<uint8_t> result;
    result.reserve(count);

    for (size_t i = 0; i < count; i++) {
        if (byteIndex >= buffer.size()) {
            break;
        }
        result.push_back(readByte());
    }

    return result;
}

bool BitInputStream::hasMoreBits() const {
    return byteIndex < buffer.size();
}

size_t BitInputStream::getRemainingBits() const {
    if (byteIndex >= buffer.size()) {
        return 0;
    }
    return (buffer.size() - byteIndex) * 8 - bitIndex;
}

size_t BitInputStream::getCurrentPosition() const {
    return byteIndex * 8 + bitIndex;
}

size_t BitInputStream::getSize() const {
    return buffer.size();
}

void BitInputStream::reset() {
    byteIndex = 0;
    bitIndex = 0;
}

void BitInputStream::clear() {
    buffer.clear();
    byteIndex = 0;
    bitIndex = 0;
}

}