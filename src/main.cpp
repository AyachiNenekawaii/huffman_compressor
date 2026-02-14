#include "HuffmanArchiver.hpp"
#include <iostream>
#include <string>

using namespace huffman;

void printUsage(const char* programName) {
    std::cout << "用法：" << programName << " <命令> [选项] <路径>" << std::endl;
    std::cout << "命令：" << std::endl;
    std::cout << "  h, help      显示帮助信息" << std::endl;
    std::cout << "  v, version   显示版本信息" << std::endl;
    std::cout << "  c, compress  压缩文件或目录" << std::endl;
    std::cout << "  x, extra     解压文件或目录" << std::endl;
    std::cout << "选项：" << std::endl;
    std::cout << "  -o <路径>    指定输出路径" << std::endl;
    std::cout << "  -d           强制目录模式" << std::endl;
    std::cout << "  -f           指定文件模式" << std::endl;
    std::cout << "  -p           显示指定信息" << std::endl;   
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "h" || command == "help" || command == "-h" || command == "--help") {
        HuffmanArchiver::printInfo();
        std::cout << std::endl;
        printUsage(argv[0]);
        return 0;
    }
    
    if (command == "v" || command == "version" || command == "-v" || command == "--version") {
        std::cout << "哈夫曼压缩工具版本：" << HuffmanArchiver::getVersion() << std::endl;
        return 0;
    }

    std::string sourcePath;
    std::string outputPath;
    CompressMode mode = CompressMode::AUTO;
    bool showProgress = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-o" && i + 1 < argc) {
            sourcePath = argv[++i];
        } else if (arg == "-d") {
            mode = CompressMode::DIRECTORY;
        } else if (arg == "-f") {
            mode = CompressMode::FILE_ONLY;
        } else if (arg == "-p") {
            showProgress = true;
        } else if (arg[0] != '-') {
            sourcePath = arg;
        } else {
            std::cerr << "未知选项：" << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    if (sourcePath.empty()) {
        std::cerr << "错误：未指定原路径" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // 创建压缩器实例
    HuffmanArchiver archiver;
    
    // 设置进度回调
    if (showProgress) {
        archiver.setProgressCallback([](const std::string& currentFile, size_t current, size_t total) {
            std::cout << "[" << current << "/" << total << "] " << currentFile << std::endl;
        });
    }

    // 执行命令
    bool success = true;
    
    if (command == "c" || command == "compress") {
        success = archiver.compress(sourcePath, outputPath, mode);
    } else if (command == "x" || command == "extra") {
        success = archiver.decompress(sourcePath, outputPath);
    } else {
        std::cerr << "错误：未知的命令" << std::endl;
        printUsage(argv[0]);
        return 1; 
    }

    return success ? 0 : 1;
}

/*
    Usage:   huffman_compressor <command> [options] <filePath>
    command: help -h --help
             version -v --version
             compress c
             extra x
    option:  -o filePath
             -d
             -f
             -p
*/