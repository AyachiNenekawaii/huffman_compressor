#include <iostream>
#include <cstdint>

const uint32_t MAGIC_NUMBER = 0x12345678;

struct Header {
    uint32_t magicNumber;
    uint16_t treeSize;
    uint64_t originalSize;
    uint64_t compressedSize;

    Header() 
        : magicNumber(MAGIC_NUMBER)
        , treeSize(0)
        , originalSize(0)
        , compressedSize(0) 
    {}
};

int main() {
    std::cout << sizeof(Header) << std::endl;
}