/**
 * @file rwsd_demo.cpp
 * @brief 可读写SD卡生产级使用示例
 * @version 3.0.0
 */

#include "rw_sd.hpp"
#include "pico/stdlib.h"
#include <stdio.h>

using namespace MicroSD;

int main() {
    stdio_init_all();
    sleep_ms(2000); // 等待串口连接
    printf("\n===== Pico 可读写SD卡生产级示例 =====\n");

    // 初始化SD卡
    RWSD sd;
    auto init_result = sd.initialize();
    if (!init_result.is_ok()) {
        printf("SD卡初始化失败: %s\n", StorageDevice::get_error_description(init_result.error_code()).c_str());
        return 1;
    }
    printf("SD卡初始化成功!\n");
    
    // 显示状态信息
    printf("%s", sd.get_status_info().c_str());
    printf("%s", sd.get_config_info().c_str());

    // === 基础文件操作示例 ===
    printf("\n===== 基础文件操作 =====\n");
    
    // 创建目录
    auto mkdir_result = sd.create_directory("/data");
    if (mkdir_result.is_ok()) {
        printf("成功创建目录 '/data'\n");
    } else {
        printf("创建目录失败: %s\n", StorageDevice::get_error_description(mkdir_result.error_code()).c_str());
    }
    
    // 写入文本文件
    std::string content = "Hello, RWSD!\n这是一个生产级示例。\n时间戳: " + std::to_string(time_us_64()) + "\n";
    auto write_result = sd.write_text_file("/data/example.txt", content);
    if (write_result.is_ok()) {
        printf("成功写入文件 '/data/example.txt'\n");
    } else {
        printf("写入文件失败: %s\n", StorageDevice::get_error_description(write_result.error_code()).c_str());
    }
    
    // 读取文件
    auto read_result = sd.read_text_file("/data/example.txt");
    if (read_result.is_ok()) {
        printf("读取文件内容:\n%s\n", read_result->c_str());
    } else {
        printf("读取文件失败: %s\n", StorageDevice::get_error_description(read_result.error_code()).c_str());
    }
    
    // 追加内容
    std::string append_content = "这是追加的内容。\n";
    auto append_result = sd.append_text_file("/data/example.txt", append_content);
    if (append_result.is_ok()) {
        printf("成功追加内容到文件\n");
    } else {
        printf("追加内容失败: %s\n", StorageDevice::get_error_description(append_result.error_code()).c_str());
    }

    // === 流式文件操作示例 ===
    printf("\n===== 流式文件操作 =====\n");
    
    // 使用文件句柄写入
    auto file_handle = sd.open_file("/data/stream.txt", "w");
    if (file_handle.is_ok()) {
        printf("成功打开文件句柄进行写入\n");
        
        // 写入多行数据
        file_handle->write_line("第一行数据");
        file_handle->write_line("第二行数据");
        file_handle->write_line("第三行数据");
        
        file_handle->close();
        printf("文件句柄写入完成\n");
    } else {
        printf("打开文件句柄失败: %s\n", StorageDevice::get_error_description(file_handle.error_code()).c_str());
    }
    
    // 使用文件句柄读取
    auto read_handle = sd.open_file("/data/stream.txt", "r");
    if (read_handle.is_ok()) {
        printf("成功打开文件句柄进行读取\n");
        
        std::string line;
        while (read_handle->read_text(line, 256).is_ok() && !line.empty()) {
            printf("读取行: %s", line.c_str());
        }
        
        read_handle->close();
        printf("文件句柄读取完成\n");
    } else {
        printf("打开文件句柄失败: %s\n", StorageDevice::get_error_description(read_handle.error_code()).c_str());
    }

    // === 目录操作示例 ===
    printf("\n===== 目录操作 =====\n");
    
    // 列出根目录
    auto list_result = sd.list_directory("/");
    if (list_result.is_ok()) {
        printf("根目录内容:\n");
        for (const auto& file : *list_result) {
            printf("  %s\t%s\t%lu字节\n", 
                file.is_directory ? "[DIR]" : "[FILE]", 
                file.name.c_str(), 
                file.size);
        }
    } else {
        printf("列出目录失败: %s\n", StorageDevice::get_error_description(list_result.error_code()).c_str());
    }
    
    // 列出data目录
    auto data_list_result = sd.list_directory("/data");
    if (data_list_result.is_ok()) {
        printf("\ndata目录内容:\n");
        for (const auto& file : *data_list_result) {
            printf("  %s\t%s\t%lu字节\n", 
                file.is_directory ? "[DIR]" : "[FILE]", 
                file.name.c_str(), 
                file.size);
        }
    } else {
        printf("列出data目录失败: %s\n", StorageDevice::get_error_description(data_list_result.error_code()).c_str());
    }

    // === 树形目录结构演示 ===
    printf("\n===== 树形目录结构 =====\n");
    
    // 创建一些测试目录和文件来演示树形结构
    sd.create_directory("/data/subdir1");
    sd.create_directory("/data/subdir2");
    sd.create_directory("/data/subdir1/nested");
    sd.write_text_file("/data/subdir1/file1.txt", "测试文件1");
    sd.write_text_file("/data/subdir1/file2.txt", "测试文件2");
    sd.write_text_file("/data/subdir1/nested/deep.txt", "深层文件");
    sd.write_text_file("/data/subdir2/config.ini", "配置文件");
    sd.write_text_file("/data/subdir2/data.bin", "二进制数据文件");
    
    // 显示完整的树形结构
    auto tree_result = sd.list_directory_tree("/");
    if (tree_result.is_ok()) {
        printf("SD卡完整目录树结构:\n");
        printf("%s", tree_result->c_str());
    } else {
        printf("生成树形结构失败: %s\n", StorageDevice::get_error_description(tree_result.error_code()).c_str());
    }
    
    // 显示data目录的树形结构
    auto data_tree_result = sd.list_directory_tree("/data");
    if (data_tree_result.is_ok()) {
        printf("\ndata目录树形结构:\n");
        printf("%s", data_tree_result->c_str());
    } else {
        printf("生成data目录树形结构失败: %s\n", StorageDevice::get_error_description(data_tree_result.error_code()).c_str());
    }

    // === 文件管理操作示例 ===
    printf("\n===== 文件管理操作 =====\n");
    
    // 复制文件
    auto copy_result = sd.copy_file("/data/example.txt", "/data/example_copy.txt");
    if (copy_result.is_ok()) {
        printf("成功复制文件\n");
    } else {
        printf("复制文件失败: %s\n", StorageDevice::get_error_description(copy_result.error_code()).c_str());
    }
    
    // 重命名文件
    auto rename_result = sd.rename("/data/stream.txt", "/data/stream_renamed.txt");
    if (rename_result.is_ok()) {
        printf("成功重命名文件\n");
    } else {
        printf("重命名文件失败: %s\n", StorageDevice::get_error_description(rename_result.error_code()).c_str());
    }
    
    // 获取文件信息
    auto file_info_result = sd.get_file_info("/data/example.txt");
    if (file_info_result.is_ok()) {
        const auto& info = *file_info_result;
        printf("文件信息: %s, 大小: %lu字节, 类型: %s\n", 
               info.name.c_str(), info.size, 
               info.is_directory ? "目录" : "文件");
    } else {
        printf("获取文件信息失败: %s\n", StorageDevice::get_error_description(file_info_result.error_code()).c_str());
    }

    // === 系统信息 ===
    printf("\n===== 系统信息 =====\n");
    auto status_result = sd.get_filesystem_status();
    if (status_result.is_ok()) {
        printf("%s", status_result->c_str());
    } else {
        printf("获取文件系统状态失败: %s\n", StorageDevice::get_error_description(status_result.error_code()).c_str());
    }
    printf("%s", sd.get_memory_usage().c_str());

    printf("\n===== 可读写SD卡生产级示例完成 =====\n");
    
    while (true) { tight_loop_contents(); }
    return 0;
} 