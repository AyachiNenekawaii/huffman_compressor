cmake_minimum_required(VERSION 3.10)
project(huffman_compressor CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 设置头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 定义源文件列表（排除 DirectoryCompressor 相关文件）
set(SOURCES
    src/BitStream.cpp
    src/FileCompressor.cpp
    src/Packer.cpp
    src/HuffmanArchiver.cpp
    src/HuffmanTree.cpp
    src/main.cpp
)

# 添加可执行文件
add_executable(huffman_compressor ${SOURCES})

# 设置安装规则
install(TARGETS huffman_compressor DESTINATION bin)

# 可选：设置调试版本的额外选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fsanitize=address")
endif()

# 输出构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler Flags: ${CMAKE_CXX_FLAGS}")