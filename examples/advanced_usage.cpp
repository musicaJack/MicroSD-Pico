/**
 * @file advanced_usage.cpp
 * @brief 高级MicroSD库使用示例
 * @author 重构自RPi_Pico_WAV_Player项目
 * 
 * 演示高级功能如批量文件操作、大文件处理、文件搜索等
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

#include "pico/stdlib.h"
#include "micro_sd.hpp"

using namespace MicroSD;

// 文件搜索功能
std::vector<FileInfo> search_files_by_extension(SDCard& sd_card, 
                                               const std::string& directory,
                                               const std::string& extension) {
    std::vector<FileInfo> matching_files;
    
    auto list_result = sd_card.list_directory(directory);
    if (list_result.is_error()) {
        printf("搜索目录 %s 失败: %s\n", 
               directory.c_str(), list_result.error_message().c_str());
        return matching_files;
    }
    
    for (const auto& file : *list_result) {
        if (file.is_directory) {
            // 递归搜索子目录
            auto sub_results = search_files_by_extension(sd_card, file.full_path, extension);
            matching_files.insert(matching_files.end(), sub_results.begin(), sub_results.end());
        } else {
            // 检查文件扩展名
            if (file.name.length() >= extension.length()) {
                std::string file_ext = file.name.substr(file.name.length() - extension.length());
                std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);
                std::string target_ext = extension;
                std::transform(target_ext.begin(), target_ext.end(), target_ext.begin(), ::tolower);
                
                if (file_ext == target_ext) {
                    matching_files.push_back(file);
                }
            }
        }
    }
    
    return matching_files;
}

// 批量文件操作示例
void demonstrate_batch_operations() {
    printf("=== 批量文件操作演示 ===\n");
    
    SDCard sd_card;
    
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败\n");
        return;
    }
    
    // 创建测试目录结构
    printf("创建测试目录结构...\n");
    std::vector<std::string> test_dirs = {
        "/batch_test",
        "/batch_test/images",
        "/batch_test/documents",
        "/batch_test/audio"
    };
    
    for (const auto& dir : test_dirs) {
        auto result = sd_card.create_directory(dir);
        if (result.is_ok()) {
            printf("创建目录: %s\n", dir.c_str());
        }
    }
    
    // 创建测试文件
    printf("\n创建测试文件...\n");
    std::vector<std::pair<std::string, std::string>> test_files = {
        {"/batch_test/images/photo1.jpg", "假的JPEG数据"},
        {"/batch_test/images/photo2.png", "假的PNG数据"},
        {"/batch_test/documents/readme.txt", "这是一个说明文件"},
        {"/batch_test/documents/report.pdf", "假的PDF数据"},
        {"/batch_test/audio/song1.mp3", "假的MP3数据"},
        {"/batch_test/audio/song2.wav", "假的WAV数据"}
    };
    
    for (const auto& [path, content] : test_files) {
        auto result = sd_card.write_text_file(path, content);
        if (result.is_ok()) {
            printf("创建文件: %s\n", path.c_str());
        }
    }
    
    // 搜索特定扩展名的文件
    printf("\n=== 文件搜索演示 ===\n");
    std::vector<std::string> extensions = {".txt", ".jpg", ".mp3"};
    
    for (const auto& ext : extensions) {
        printf("搜索 %s 文件:\n", ext.c_str());
        auto matching_files = search_files_by_extension(sd_card, "/batch_test", ext);
        
        for (const auto& file : matching_files) {
            printf("  找到: %s (大小: %zu 字节)\n", 
                   file.full_path.c_str(), file.size);
        }
        printf("\n");
    }
}

// 大文件处理示例
void demonstrate_large_file_handling() {
    printf("=== 大文件处理演示 ===\n");
    
    SDCard sd_card;
    
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败\n");
        return;
    }
    
    const std::string large_file_path = "/large_test.bin";
    const size_t chunk_size = 1024;  // 1KB chunks
    const size_t total_chunks = 100;  // 总共100KB
    
    printf("创建大文件 (约 %zu KB)...\n", total_chunks);
    
    // 使用流式操作写入大文件
    auto file_result = sd_card.open_file(large_file_path, "w");
    if (file_result.is_error()) {
        printf("打开大文件失败: %s\n", file_result.error_message().c_str());
        return;
    }
    
    auto file_handle = std::move(*file_result);
    
    // 创建测试数据块
    std::vector<uint8_t> chunk_data(chunk_size);
    for (size_t i = 0; i < chunk_size; i++) {
        chunk_data[i] = static_cast<uint8_t>(i % 256);
    }
    
    // 写入数据块
    printf("写入进度: ");
    for (size_t i = 0; i < total_chunks; i++) {
        auto write_result = file_handle.write(chunk_data);
        if (write_result.is_error()) {
            printf("\n写入第 %zu 块失败\n", i + 1);
            break;
        }
        
        // 显示进度
        if ((i + 1) % 10 == 0) {
            printf("%zu%% ", ((i + 1) * 100) / total_chunks);
        }
        
        // 定期刷新以确保数据写入
        if ((i + 1) % 20 == 0) {
            file_handle.flush();
        }
    }
    printf("\n");
    
    file_handle.close();
    
    // 验证文件大小
    auto info_result = sd_card.get_file_info(large_file_path);
    if (info_result.is_ok()) {
        printf("文件创建完成，大小: %zu 字节\n", info_result->size);
    }
    
    // 分块读取大文件
    printf("\n分块读取大文件...\n");
    auto read_file_result = sd_card.open_file(large_file_path, "r");
    if (read_file_result.is_ok()) {
        auto read_handle = std::move(*read_file_result);
        
        size_t total_read = 0;
        size_t chunk_count = 0;
        
        while (true) {
            auto read_result = read_handle.read(chunk_size);
            if (read_result.is_error() || read_result->empty()) {
                break;
            }
            
            total_read += read_result->size();
            chunk_count++;
            
            // 验证数据正确性（可选）
            if (chunk_count <= 3) {  // 只验证前3块
                bool data_valid = true;
                for (size_t i = 0; i < read_result->size(); i++) {
                    if ((*read_result)[i] != static_cast<uint8_t>(i % 256)) {
                        data_valid = false;
                        break;
                    }
                }
                printf("第 %zu 块数据校验: %s\n", 
                       chunk_count, data_valid ? "正确" : "错误");
            }
        }
        
        printf("总共读取 %zu 字节，分 %zu 块\n", total_read, chunk_count);
        read_handle.close();
    }
    
    // 清理测试文件
    printf("清理测试文件...\n");
    sd_card.delete_file(large_file_path);
}

// 文件系统监控示例
void demonstrate_filesystem_monitoring() {
    printf("=== 文件系统监控演示 ===\n");
    
    SDCard sd_card;
    
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败\n");
        return;
    }
    
    // 获取初始容量信息
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_error()) {
        printf("获取容量信息失败\n");
        return;
    }
    
    auto [total_bytes, initial_free] = *capacity_result;
    printf("文件系统监控开始\n");
    printf("总容量: %.2f MB\n", total_bytes / 1024.0 / 1024.0);
    printf("初始可用空间: %.2f MB\n", initial_free / 1024.0 / 1024.0);
    
    // 创建一些文件并监控空间变化
    const std::string monitor_dir = "/monitor_test";
    sd_card.create_directory(monitor_dir);
    
    for (int i = 1; i <= 5; i++) {
        std::string file_path = monitor_dir + "/file" + std::to_string(i) + ".txt";
        std::string content;
        
        // 创建不同大小的文件
        content.resize(i * 1024, 'A' + (i - 1));  // 1KB, 2KB, 3KB, 4KB, 5KB
        
        auto write_result = sd_card.write_text_file(file_path, content);
        if (write_result.is_ok()) {
            // 获取当前空间信息
            auto current_capacity = sd_card.get_capacity();
            if (current_capacity.is_ok()) {
                auto [total, current_free] = *current_capacity;
                size_t used_space = initial_free - current_free;
                
                printf("创建文件 %s (大小: %d KB)\n", file_path.c_str(), i);
                printf("  已使用空间增加: %.2f KB\n", used_space / 1024.0);
                printf("  剩余空间: %.2f MB\n", current_free / 1024.0 / 1024.0);
            }
        }
        
        sleep_ms(500);  // 短暂延迟
    }
}

// 文件完整性检查示例
void demonstrate_file_integrity_check() {
    printf("\n=== 文件完整性检查演示 ===\n");
    
    SDCard sd_card;
    
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败\n");
        return;
    }
    
    const std::string test_file = "/integrity_test.txt";
    const std::string original_content = "这是用于完整性检查的测试内容。\n包含中文和英文字符。\n123456789\n";
    
    // 写入原始文件
    printf("创建测试文件...\n");
    auto write_result = sd_card.write_text_file(test_file, original_content);
    if (write_result.is_error()) {
        printf("创建测试文件失败\n");
        return;
    }
    
    // 计算简单校验和
    uint32_t original_checksum = 0;
    for (char c : original_content) {
        original_checksum += static_cast<uint32_t>(c);
    }
    
    printf("原始校验和: %u\n", original_checksum);
    
    // 读取文件并验证
    printf("验证文件完整性...\n");
    auto read_result = sd_card.read_file(test_file);
    if (read_result.is_ok()) {
        std::string read_content(read_result->begin(), read_result->end());
        
        uint32_t read_checksum = 0;
        for (char c : read_content) {
            read_checksum += static_cast<uint32_t>(c);
        }
        
        printf("读取校验和: %u\n", read_checksum);
        
        if (original_checksum == read_checksum) {
            printf("✓ 文件完整性检查通过\n");
        } else {
            printf("✗ 文件完整性检查失败\n");
        }
        
        if (original_content == read_content) {
            printf("✓ 内容完全匹配\n");
        } else {
            printf("✗ 内容不匹配\n");
        }
    }
    
    // 清理
    sd_card.delete_file(test_file);
}

int main() {
    stdio_init_all();
    
    // 等待串口连接 - 显示倒计时
    printf("高级示例启动中，等待串口连接...\n");
    for (int i = 5; i > 0; i--) {
        printf("启动倒计时: %d 秒\r", i);
        fflush(stdout);  // 强制刷新输出缓冲区
        sleep_ms(1000);
    }
    printf("启动完成!        \n\n");  // 多几个空格覆盖倒计时文字
    
    printf("========================================\n");
    printf("     MicroSD库高级使用示例\n");
    printf("========================================\n");
    printf("程序已启动！\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n\n");
    
    // 批量文件操作
    demonstrate_batch_operations();
    
    // 大文件处理
    demonstrate_large_file_handling();
    
    // 文件系统监控
    demonstrate_filesystem_monitoring();
    
    // 文件完整性检查
    demonstrate_file_integrity_check();
    
    printf("\n所有高级演示完成!\n");
    
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
} 