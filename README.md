# Huffman Compressor

基于哈夫曼编码的文件/目录压缩工具。

## 功能特性

- 支持单文件压缩和解压
- 支持目录压缩和解压（包含子目录）
- 提供详细的压缩统计信息
- 自动识别文件类型并选择合适的压缩模式

## 编译要求

- C++17 或更高版本
- GNU Make
- 支持C++17的编译器（如GCC 7+ 或 Clang 5+）

## 使用Makefile构建

### 基本命令

```bash
# 编译项目（默认目标）
make

# 清理构建产物
make clean

# 调试模式编译（包含调试符号）
make debug

# 安装到系统（需要sudo权限）
sudo make install

# 运行程序
make run

# 查看帮助信息
make help

# 查看构建配置信息
make print-info
```

### 测试和质量检查

```bash
# 运行基本功能测试
make test

# 静态代码分析（需要安装cppcheck）
make check

# 生成文档（需要安装doxygen）
make docs
```

## 使用方法

编译完成后，可执行文件位于 `bin/huffman_compressor`。

### 基本用法

```bash
# 压缩文件或目录
./bin/huffman_compressor compress <源路径> [选项]

# 解压文件或目录
./bin/huffman_compressor extra <压缩文件> [选项]

# 显示帮助
./bin/huffman_compressor help

# 显示版本信息
./bin/huffman_compressor version
```

### 常用选项

- `-o <路径>`: 指定输出路径
- `-d`: 强制使用目录模式
- `-f`: 强制使用文件模式
- `-p`: 显示详细进度信息

### 示例

```bash
# 压缩单个文件
./bin/huffman_compressor compress document.txt

# 压缩整个目录
./bin/huffman_compressor compress my_folder -o compressed_archive

# 解压文件
./bin/huffman_compressor extra document.txt.huff

# 解压目录
./bin/huffman_compressor extra compressed_archive.huff -o restored_folder
```

## 项目结构

```
huffman_compressor/
├── include/              # 头文件目录
│   ├── BitStream.hpp
│   ├── DirectoryCompressor.hpp
│   ├── FileCompressor.hpp
│   ├── HuffmanArchiver.hpp
│   └── HuffmanTree.hpp
├── src/                  # 源文件目录
│   ├── BitStream.cpp
│   ├── DirectoryCompressor.cpp
│   ├── FileCompressor.cpp
│   ├── HuffmanArchiver.cpp
│   ├── HuffmanTree.cpp
│   └── main.cpp
├── build/               # 编译中间文件（自动生成）
├── bin/                 # 可执行文件（自动生成）
├── Makefile             # 构建脚本
└── README.md            # 本文档
```

## 开发说明

### 代码规范

- 使用C++17标准
- 遵循现代C++编程实践
- 所有公共接口都有中文注释
- 错误处理使用异常机制

### 内存管理

- 主要使用智能指针（std::unique_ptr, std::shared_ptr）
- RAII原则管理资源
- 自动内存管理，避免内存泄漏

## 许可证

[请在此处添加许可证信息]

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。
