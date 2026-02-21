#include "HuffmanTree.hpp"
#include <iostream>
#include <algorithm>
#include <queue>
#include <stdexcept>
#include <functional>

namespace huffman {

HuffmanTree::HuffmanTree() : root(nullptr) {}

HuffmanTree::~HuffmanTree() {
    clear();
}

void HuffmanTree::clear() {
    root.reset();
    codeTable.clear();
    reverseCodeTable.clear();
}

bool HuffmanTree::isEmpty() const {
    return root == nullptr;
}

void HuffmanTree::buildFromFrequencies(const std::unordered_map<uint8_t, uint32_t>& frequencies) {
    if (frequencies.empty()) {
        throw std::invalid_argument("频率表为空");
    }

    clear();

    // 创建优先队列
    std::priority_queue<std::shared_ptr<HuffmanNode>,
                        std::vector<std::shared_ptr<HuffmanNode>>,
                        NodeComparator> minHeap;

    // 将所有字符加入优先队列
    for (const auto& pair : frequencies) {
        minHeap.push(std::make_shared<HuffmanNode>(pair.first, pair.second, true));
    }

    // 构建哈夫曼树
    while (minHeap.size() > 1) {
        auto left = minHeap.top();
        minHeap.pop();
        auto right = minHeap.top();
        minHeap.pop();

        auto parent = std::make_shared<HuffmanNode>(left, right);
        minHeap.push(parent);
    }

    if (!minHeap.empty()) {
        root = minHeap.top();
    }

    // 生成编码表
    std::vector<bool> currentCode;
    generateCodes(root, currentCode);
}

void HuffmanTree::buildFromData(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        throw std::invalid_argument("数据为空");
    }

    // 统计频率
    std::unordered_map<uint8_t, uint32_t> frequencies;
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }

    buildFromFrequencies(frequencies);
}

void HuffmanTree::generateCodes(std::shared_ptr<HuffmanNode> node,
    std::vector<bool>& currentCode) {
    if (!node) return;

    // 叶子节点，保存编码
    if (node->isLeaf) {
        // 特殊情况：只有一个节点，赋予编码0
        if (currentCode.empty()) {
            codeTable[node->data] = std::vector<bool> {false};
            reverseCodeTable[std::vector<bool>{false}] = node->data;
        } else {
            codeTable[node->data] = currentCode;
            reverseCodeTable[currentCode] = node->data;
        }
        return;
    }

    // 遍历左子树（编码0）
    currentCode.push_back(false);
    generateCodes(node->left, currentCode);
    currentCode.pop_back();

    // 遍历右子树（编码1）
    currentCode.push_back(true);
    generateCodes(node->right, currentCode);
    currentCode.pop_back();
}

std::vector<bool> HuffmanTree::getCode(uint8_t byte) const {
    auto it = codeTable.find(byte);
    if (it != codeTable.end()) {
        return it->second;
    }
    throw std::runtime_error("未找到字符的编码");
}

const std::unordered_map<uint8_t, std::vector<bool>>& HuffmanTree::getCodeTable() const {
    return codeTable;
}

std::shared_ptr<HuffmanNode> HuffmanTree::getRoot() const {
    return root;
}

// 序列化格式：
// [1字节: 标志位] [如果是叶子: 1字节数据] [如果不是叶子: 递归序列化左右子树]
// 标志位: 0x01 = 叶子节点, 0x00 = 内部节点

void HuffmanTree::serializeTree(std::shared_ptr<HuffmanNode> node,
                                std::vector<uint8_t>& output) const {
    if (!node) return;

    if (node->isLeaf) {
        output.push_back(0x01);       // 叶子节点标志
        output.push_back(node->data); // 数据
    } else {
        output.push_back(0x00); // 内部节点标志 
        serializeTree(node->left, output);
        serializeTree(node->right, output);
    }
}

std::vector<uint8_t> HuffmanTree::serialize() const {
    if (!root) {
        throw std::runtime_error("树为空，无法序列化");
    }

    std::vector<uint8_t> output;
    serializeTree(root, output);
    return output;
}

std::shared_ptr<HuffmanNode> HuffmanTree::deserializeTree(const std::vector<uint8_t>& data,
    size_t& index) {
    if (index >= data.size()) {
        throw std::runtime_error("反序列化数据不完整");
    }

    uint8_t flag = data[index++];

    if (flag == 0x01) { // 叶子节点
        if (index >= data.size()) {
            throw std::runtime_error("反序列化数据不完整：缺少叶子节点数据");
        }
        uint8_t byteData = data[index++];
        return std::make_shared<HuffmanNode>(byteData, 0, true);
    } else { // 内部节点
        auto left = deserializeTree(data, index);
        auto right = deserializeTree(data, index);
        return std::make_shared<HuffmanNode>(left, right);
    }
}

void HuffmanTree::deserialize(const std::vector<uint8_t>& data) {
    if (data.empty()) {
        throw std::invalid_argument("反序列化数据为空");
    }

    clear();
    size_t index = 0;
    root = deserializeTree(data, index);

    // 重新生成编码表
    std::vector<bool> currentCode;
    generateCodes(root, currentCode);
}

size_t HuffmanTree::getDepth() const {
    if (!root) return 0;

    std::function<size_t(std::shared_ptr<HuffmanNode>)> getDepth =
        [&getDepth](std::shared_ptr<HuffmanNode> node) ->size_t {
        if (!node) return 0;
        if (node->isLeaf) return 1;
        return 1 + std::max(getDepth(node->left), getDepth(node->right));
    };

    return getDepth(root);
}

void HuffmanTree::printCodeTable() const {
    std::cout << "哈夫曼编码表：" << std::endl;
    for (const auto& pair : codeTable) {
        std::cout << "字符 '" << (pair.first >= 32 && pair.first < 127 ? static_cast<char>(pair.first) : '?') 
                  << "' (" << static_cast<int>(pair.first) << "): ";
        for (bool bit : pair.second) {
            std::cout << (bit ? '1' : '0');
        }
        std::cout << std::endl;
    }
}

}