#include "HuffmanArchiver.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace huffman {

const std::string VERSION = "1.0.0";
const std::string FILE_EXTENSION = ".huff";

HuffmanArchiver::HuffmanArchiver()
    : fileCompressor(std::make_unique<FileCompressor>()),
      dirCompressor(std::make_unique<DirectoryCompressor>()) {}

HuffmanArchiver::~HuffmanArchiver() = default;

bool HuffmanArchiver::isDirectory(const std::string& path) {
    return fs::is_directory(path);
}

bool HuffmanArchiver::fileExists(const std::string& path) {
    return fs::exists(path);
}

std::string HuffmanArchiver::getExtension(const std::string& path) {
    return fs::path(path).extension().string();
}

std::string HuffmanArchiver::removeExtension(const std::string& path) {
    return fs::path(path).replace_extension("").string();
}

bool HuffmanArchiver::compress(const std::string& source,
                               const std::string& output,
                               CompressMode mode) {
    try {
        if (!fileExists(source)) {
            std::cerr << "错误：原路径不存在：" + source << std::endl;
            return false;
        }

        // 确定输出路径
        std::string actualOutput = output;
        if (actualOutput.empty()) {
            actualOutput = source + FILE_EXTENSION;
        }

        // 检查输出文件是否已经存在
        if (fileExists(actualOutput)) {
            std::cout << "警告：输出文件已存在，将覆盖：" << actualOutput << std::endl;
        }

        // 根据模式选择压缩方式
        bool isDir = isDirectory(source);

        switch (mode) {
            case CompressMode::FILE_ONLY:
                if (isDir) {
                    std::cerr << "错误：FILE_ONLY模式不能用于目录" << std::endl;
                    return false;
                }
                std::cout << "正在压缩文件：" << source << std::endl;
                fileCompressor->compressToFile(source, actualOutput);
                fileCompressor->getStats().print();
                break;
            
            case CompressMode::DIRECTORY:
                if (!isDir) {
                    std::cerr << "错误: DIRECTORY模式只能用于目录" << std::endl;
                    return false;
                }
                std::cout << "正在压缩目录：" << source << std::endl;
                dirCompressor->compress(source, actualOutput);
                dirCompressor->getStats().print();
                break;

            case CompressMode::AUTO:
            default:
                if (isDir) {
                    std::cout << "正在压缩目录: " << source << std::endl;
                    dirCompressor->compress(source, actualOutput);
                    dirCompressor->getStats().print();
                } else {
                    std::cout << "正在压缩文件: " << source << std::endl;
                    fileCompressor->compressToFile(source, actualOutput);
                    fileCompressor->getStats().print();
                }
                break;
        }

        std::cout << "压缩完成：" << actualOutput << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "压缩失败：" << e.what() << std::endl;
        return false;
    }
}

bool HuffmanArchiver::decompress(const std::string& source,
                                 const std::string& output) {
    try {
        if (!fileExists(source)) {
            std::cerr << "错误: 压缩文件不存在: " << source << std::endl;
            return false;
        }

        // 确定输出路径
        std::string actualOutput = output;
        if (actualOutput.empty()) {
            // 如果源文件有.huff扩展名，移除它
            if (getExtension(source) == FILE_EXTENSION) {
                actualOutput = removeExtension(source);
            } else {
                actualOutput = source + "_extracted";
            }
        }

        // 检查输出路径是否已存在
        if (fileExists(actualOutput)) {
            std::cout << "警告: 输出路径已存在，将覆盖: " << actualOutput << std::endl;
        }

        std::cout << "正在解压: " << source << std::endl;

        // 尝试作为目录压缩文件解压
        try {
            dirCompressor->decompress(source, actualOutput);
            dirCompressor->getStats().print();
            std::cout << "解压完成: " << actualOutput << std::endl;
            return true;
        } catch (const std::exception& e) {
            // 目录解压失败，尝试作为文件解压
            std::cout << "尝试作为单文件解压..." << std::endl;
            fileCompressor->decompressFromFile(source, actualOutput);
            fileCompressor->getStats().print();
            std::cout << "解压完成: " << actualOutput << std::endl;
            return true;
        }

    } catch (const std::exception& e) {
        std::cerr << "解压失败: " << e.what() << std::endl;
        return false;
    }
}

CompressionStats HuffmanArchiver::getFileStats() const {
    return fileCompressor->getStats();
}

DirectoryCompressionStats HuffmanArchiver::getDirectoryStats() const {
    return dirCompressor->getStats();
}

void HuffmanArchiver::setProgressCallback(ProgressCallback callback) {
    dirCompressor->setProgressCallback(callback);
}

std::string HuffmanArchiver::getVersion() {
    return VERSION;
}

void HuffmanArchiver::printInfo() {
    std::cout << "======================================" << std::endl;
    std::cout << "  哈夫曼压缩工具 v" << getVersion() << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "基于哈夫曼编码的文件/目录压缩工具" << std::endl;
    std::cout << "支持功能:" << std::endl;
    std::cout << "  - 单文件压缩/解压" << std::endl;
    std::cout << "  - 目录压缩/解压（包含子目录）" << std::endl;
    std::cout << "  - 压缩统计信息" << std::endl;
    std::cout << "======================================" << std::endl;
}

}