#include "CLI11.hpp"
#include "HuffmanArchiver.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace huffman;

int main(int argc, char* argv[]) {
    try {
        CLI::App app{"A Huffman compression tool"};

        std::string outputPath;

        // 压缩子命令
        auto compressCmd = app.add_subcommand("compress", "Compress files or folders");
        std::vector<std::string> sources;
        compressCmd->add_option("sources", sources)->expected(1, -1);
        compressCmd->add_option("-o,--output", outputPath)->required();

        // 解压子命令
        auto extraCmd = app.add_subcommand("x,extra", "Extea from the archive");
        std::string source;
        extraCmd->add_option("source", source)->required();
        extraCmd->add_option("-o,--output", outputPath)->required();


        // 解析命令行参数
        CLI11_PARSE(app, argc, argv);

        bool isSuccess = true;
        HuffmanArchiver archiver;

        if (compressCmd->parsed()) {
            isSuccess = archiver.compress(sources, outputPath);
        } else if (extraCmd->parsed()) {
            isSuccess = archiver.decompress(source, outputPath);
        }

        return isSuccess ? 0 : 1;
    } catch (const std::exception &e) {
        std::cerr << "ERORR: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "UNKNOWN ERORR" << std::endl;
    }
}