#include "HuffmanArchiver.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace huffman;

void printUsage(std::string programName) {
    std::string usage = "使用方法:\n  " + programName + " <命令> [选项]\n"
        + "\n命令:\n"
        + "  h, help      显示帮助信息\n"
        + "  v, version   显示版本信息\n"
        + "  c, compress  压缩文件或目录\n"
        + "  x, extra     解压文件或目录\n"
        + "\n选项:\n"
        + "  -o <path>    指定输出路径\n";
    std::cout << usage << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "h" || command == "help") {
        printUsage(argv[0]);
        return 0;
    }
    
    if (command == "v" || command == "version") {
        std::cout << argv[0] << HuffmanArchiver::getVersion() << std::endl;
        return 0;
    }

    std::vector<std::string> sources;
    std::string outputPath;
    bool showProgress = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (arg == "-p") {
            showProgress = true;
        } else if (arg[0] != '-') {
            sources.push_back(arg);
        } else {
            std::cerr << "未知选项:" << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    if (sources.empty()) {
        std::cerr << "错误: 未指定源文件" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    HuffmanArchiver archiver;

    bool isSuccess = true;
    
    if (command == "c" || command == "compress") {
        isSuccess = archiver.compress(sources, outputPath);
    } else if (command == "x" || command == "extra") {
        if (sources.size() != 1) {
            std::cerr << "错误: extra 命令要求指定一个源文件" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        isSuccess = archiver.decompress(sources[0], outputPath);
    } else {
        std::cerr << "未知命令:" << command << std::endl;
        printUsage(argv[0]);
        return 1; 
    }

    return isSuccess ? 0 : 1;
}