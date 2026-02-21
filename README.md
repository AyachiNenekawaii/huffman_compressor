# Huffman Compressor

一个基于哈夫曼编码（Huffman Coding）的文件压缩工具，支持文件/目录解压缩功能。

## 安装与配置
```bash
# 克隆仓库
git clone <repository-url>

# 进入项目目录
cd huffman_compressor

# 创建构建目录
mkdir build
cd build

# 配置并编译项目
cmake ..
make
```

## 使用指南

### 基本命令格式

```bash
huffman_compressor <命令> [选项] <源文件/目录>
```

### 命令说明

| 命令 | 简写 | 功能 |
|------|------|------|
| `help` | `h` | 显示帮助信息 |
| `version` | `v` | 显示版本信息 |
| `compress` | `c` | 压缩文件或目录 |
| `extra` | `x` | 解压文件或目录 |

### 选项说明

| 选项 | 说明 |
|------|------|
| `-o <path>` | 指定输出路径 |

### 使用示例

#### 1. 显示帮助信息

```bash
huffman_compressor help
# 或
huffman_compressor h
```

#### 2. 查看版本信息

```bash
huffman_compressor version
# 或
huffman_compressor v
```

#### 3. 压缩单个文件

```bash
# 基本用法
huffman_compressor compress document.txt

# 指定输出文件名
huffman_compressor compress document.txt -o compressed.huff
```

#### 4. 压缩多个文件

```bash
huffman_compressor compress file1.txt file2.txt file3.txt -o archive.huff
```

#### 5. 压缩目录

```bash
# 压缩整个目录
huffman_compressor compress my_folder/

# 指定输出文件名
huffman_compressor compress my_folder/ -o backup.huff
```

#### 6. 解压文件

```bash
# 基本用法（自动恢复原始文件名）
huffman_compressor extra compressed.huff

# 指定输出目录
huffman_compressor extra compressed.huff -o output_folder/
```

### 压缩文件格式

压缩文件（`.huff`）的内部结构如下：

```
[4字节: Magic Number]      - 文件标识 "HUFF"
[2字节: 哈夫曼树大小]        - 编码树数据大小
[8字节: 原始文件大小]        - 未压缩数据大小
[8字节: 压缩文件大小]        - 压缩后数据大小
[N字节: 哈夫曼树数据]        - 序列化的哈夫曼树
[M字节: 压缩数据]           - 实际的压缩内容
```

## 项目结构

```
huffman_compressor/
├── CMakeLists.txt          # CMake 构建配置
├── README.md               # 项目文档
├── .gitignore              # Git 忽略配置
├── include/                # 头文件目录
│   ├── BitStream.hpp       # 位流操作类
│   ├── FileCompressor.hpp  # 文件压缩器
│   ├── Header.hpp          # 文件头格式定义
│   ├── HuffmanArchiver.hpp # 主程序接口
│   ├── HuffmanTree.hpp     # 哈夫曼树实现
│   └── Packer.hpp          # 目录打包器
├── src/                    # 源文件目录
│   ├── BitStream.cpp       # 位流操作实现
│   ├── FileCompressor.cpp  # 文件压缩实现
│   ├── HuffmanArchiver.cpp # 主程序实现
│   ├── HuffmanTree.cpp     # 哈夫曼树算法
│   ├── main.cpp            # 程序入口
│   └── Packer.cpp          # 目录打包实现
└── build/                  # 构建输出目录
```

### 核心类说明

#### HuffmanTree
- 实现哈夫曼树的构建和编码表生成
- 支持树的序列化和反序列化
- 提供最优前缀编码查询

#### FileCompressor
- 封装单个文件的压缩/解压逻辑
- 处理文件读写和头信息管理
- 协调哈夫曼树和数据压缩

#### Packer
- 实现多文件和目录的打包功能
- 维护目录结构和文件元数据
- 支持递归目录遍历

#### BitStream
- 提供精确的位级数据读写
- 支持字节对齐和缓冲区管理
- 用于压缩数据的存储和读取

#### HuffmanArchiver
- 主程序接口类
- 整合压缩、解压、打包功能
- 提供命令行参数处理

## 技术原理

### 哈夫曼编码算法

1. **频率统计**：扫描输入数据，统计每个字节的出现频率
2. **构建树**：使用优先队列构建哈夫曼树，频率低的节点优先合并
3. **生成编码**：从根节点遍历树，左分支为0，右分支为1，生成前缀编码
4. **编码数据**：使用生成的编码表替换原始数据

### 压缩流程

```
输入文件 → 频率统计 → 构建哈夫曼树 → 生成编码表 → 编码数据 → 写入文件
```

### 解压流程

```
压缩文件 → 读取头信息 → 重建哈夫曼树 → 解码数据 → 恢复原始文件
```

## 贡献指南

欢迎提交 Issue 和 Pull Request 来改进项目！
