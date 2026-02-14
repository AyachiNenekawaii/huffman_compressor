#ifndef HUFFMANTREE_HPP
#define HUFFMANTREE_HPP

#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace huffman {

struct HuffmanNode {
    uint8_t data;                       // 字符数据
    uint32_t frequency;                 // 出现频率
    bool isLeaf;                        // 是否为叶节点
    std::shared_ptr<HuffmanNode> left;  // 左子树
    std::shared_ptr<HuffmanNode> right; // 右子树

    HuffmanNode(uint8_t d, uint32_t freq, bool leaf = true)
        : data(d), frequency(freq), isLeaf(leaf), left(nullptr), right(nullptr) {}

    HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r)
        : data(0), frequency(l->frequency + r->frequency), 
          isLeaf(false), left(l), right(r) {}
};

struct NodeComparator {
    bool operator()(const std::shared_ptr<HuffmanNode>& a,
        const std::shared_ptr<HuffmanNode>& b) {
        return a->frequency > b->frequency;
    }
};

struct VectorBoolHash {
    uint32_t operator()(const std::vector<bool>& bits) const {
        uint32_t result = 0;
        for (auto bit : bits) {
            result = (result << 1) | (bit ? 1 : 0);
        }
        return result;
    }
};

class HuffmanTree {
private:
    std::shared_ptr<HuffmanNode> root;
    std::unordered_map<uint8_t, std::vector<bool>> codeTable;
    std::unordered_map<std::vector<bool>, uint8_t, VectorBoolHash> reverseCodeTable;

    // 递归生成编码表
    void generateCodes(std::shared_ptr<HuffmanNode> node,
        std::vector<bool>& currentCode);

    // 序列化树到字节数组
    void serializeTree(std::shared_ptr<HuffmanNode> node,
        std::vector<uint8_t>& output) const;

    // 反序列化树
    std::shared_ptr<HuffmanNode> deserializeTree(const std::vector<uint8_t>& data,
        size_t& index);

public:
    HuffmanTree();
    ~HuffmanTree();

    // 从频率表构建哈夫曼树
    void buildFromFrequencies(const std::unordered_map<uint8_t, uint32_t>& frequencies);

    // 从原始数据构建哈夫曼树
    void buildFromData(const std::vector<uint8_t>& data);

    // 获取字符的编码
    std::vector<bool> getCode(uint8_t byte) const;

    // 获取编码表
    const std::unordered_map<uint8_t, std::vector<bool>>& getCodeTable() const;

    // 获取根节点
    std::shared_ptr<HuffmanNode> getRoot() const;

    // 序列化哈夫曼树
    std::vector<uint8_t> serialize() const;

    // 反序列化哈夫曼树
    void deserialize(const std::vector<uint8_t>& data);

    // 清空树
    void clear();

    // 检查树是否为空
    bool isEmpty() const;

    // 获取树的深度
    size_t getDepth() const;

    // 打印编码表
    void printCodeTable() const;
};

}

#endif // HUFFMANTREE_HPP
