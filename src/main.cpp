#include "HuffmanArchiver.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace huffman;

void printUsage(std::string programName) {
    std::string usage = "Usage:\n  " + programName + " <command> [options]\n"
        + "\nCommands:\n"
        + "  h, help      show help information\n"
        + "  v, version   show version information\n"
        + "  c, compress  compress files or directories\n"
        + "  x, extra     extract files or directories\n"
        + "\nOptions:\n"
        + "  -o <path>    specify output path\n";
        // + "  -p           show progress information\n";
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
            std::cerr << "unknow option:" << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    if (sources.empty()) {
        std::cerr << "error: no source path specified" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    HuffmanArchiver archiver;
    
    // 设置进度回调
    // if (showProgress) {
    //     archiver.setProgressCallback([](const std::string& currentFile, size_t current, size_t total) {
    //         std::cout << "[" << current << "/" << total << "] " << currentFile << std::endl;
    //     });
    // }

    bool isSuccess = true;
    
    if (command == "c" || command == "compress") {
        isSuccess = archiver.compress(sources, outputPath);
    } else if (command == "x" || command == "extra") {
        if (sources.size() != 1) {
            std::cerr << "error: extra command requires exactly one source path" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        isSuccess = archiver.decompress(sources[0], outputPath);
    } else {
        std::cerr << "unknow command:" << command << std::endl;
        printUsage(argv[0]);
        return 1; 
    }

    return isSuccess ? 0 : 1;
}