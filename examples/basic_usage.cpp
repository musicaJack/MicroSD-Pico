/**
 * @file basic_usage.cpp
 * @brief 基本的MicroSD库使用示例
 * @author 重构自RPi_Pico_WAV_Player项目
 * 
 * 演示如何使用现代C++风格的MicroSD库进行基本的文件操作
 */

#include <iostream>
#include <vector>
#include <string>

#include "pico/stdlib.h"
#include "micro_sd.hpp"

using namespace MicroSD;

void print_file_info(const FileInfo& info) {
    printf("文件名: %s\n", info.name.c_str());
    printf("完整路径: %s\n", info.full_path.c_str());
    printf("大小: %zu 字节\n", info.size);
    printf("类型: %s\n", info.is_directory ? "目录" : "文件");
    printf("---\n");
}

void demonstrate_basic_operations(SDCard& sd_card) {
    printf("=== MicroSD卡基本操作演示 ===\n\n");
    
    // 获取文件系统信息
    printf("文件系统类型: %s\n", sd_card.get_filesystem_type().c_str());
    
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("总容量: %.2f MB\n", total / 1024.0 / 1024.0);
        printf("可用容量: %.2f MB\n", free / 1024.0 / 1024.0);
    }
    printf("\n");
    
    // 4. 列出根目录内容
    printf("=== 根目录内容 ===\n");
    auto list_result = sd_card.list_directory("/");
    if (list_result.is_ok()) {
        for (const auto& file : *list_result) {
            print_file_info(file);
        }
    } else {
        printf("列出目录失败: %s\n", list_result.error_message().c_str());
    }
    
    // 5. 创建测试目录
    printf("=== 创建测试目录 ===\n");
    
    // 首先检查目录是否已存在
    auto check_result = sd_card.get_file_info("/test_dir");
    if (check_result.is_ok() && check_result->is_directory) {
        printf("目录 '/test_dir' 已存在，跳过创建\n");
    } else {
        // 目录不存在，创建新目录
        auto mkdir_result = sd_card.create_directory("/test_dir");
        if (mkdir_result.is_ok()) {
            printf("创建目录 '/test_dir' 成功\n");
        } else {
            printf("创建目录失败: %s\n", mkdir_result.error_message().c_str());
        }
    }
    
    // 6. 写入测试文件
    printf("\n=== 写入测试文件 ===\n");
    std::string test_content = "Hello, MicroSD!\n这是一个测试文件。\n当前时间戳: ";
    test_content += std::to_string(to_ms_since_boot(get_absolute_time()));
    
    auto write_result = sd_card.write_text_file("/test_dir/hello.txt", test_content);
    if (write_result.is_ok()) {
        printf("写入文件 '/test_dir/hello.txt' 成功\n");
    } else {
        printf("写入文件失败: %s\n", write_result.error_message().c_str());
    }
    
    // 7. 读取测试文件
    printf("\n=== 读取测试文件 ===\n");
    auto read_result = sd_card.read_file("/test_dir/hello.txt");
    if (read_result.is_ok()) {
        std::string content(read_result->begin(), read_result->end());
        printf("文件内容:\n%s\n", content.c_str());
    } else {
        printf("读取文件失败: %s\n", read_result.error_message().c_str());
    }
    
    // 8. 获取文件信息
    printf("\n=== 文件信息 ===\n");
    auto info_result = sd_card.get_file_info("/test_dir/hello.txt");
    if (info_result.is_ok()) {
        print_file_info(*info_result);
    }
    
    // 9. 演示追加写入
    printf("=== 追加写入测试 ===\n");
    std::string append_content = "\n追加的内容\n";
    auto append_result = sd_card.write_text_file("/test_dir/hello.txt", append_content, true);
    if (append_result.is_ok()) {
        printf("追加写入成功\n");
        
        // 再次读取文件查看结果
        auto read_again = sd_card.read_file("/test_dir/hello.txt");
        if (read_again.is_ok()) {
            std::string updated_content(read_again->begin(), read_again->end());
            printf("更新后的文件内容:\n%s\n", updated_content.c_str());
        }
    }
}

void demonstrate_stream_operations(SDCard& sd_card) {
    printf("\n=== 流式文件操作演示 ===\n");
    
    // 打开文件进行写入
    auto file_result = sd_card.open_file("/stream_test.txt", "w");
    if (file_result.is_error()) {
        printf("打开文件失败: %s\n", file_result.error_message().c_str());
        return;
    }
    
    auto file_handle = std::move(*file_result);
    
    // 逐行写入数据
    for (int i = 0; i < 10; i++) {
        std::string line = "第 " + std::to_string(i + 1) + " 行数据\n";
        auto write_result = file_handle.write(line);
        if (write_result.is_error()) {
            printf("写入第%d行失败\n", i + 1);
            break;
        }
        printf("已写入第 %d 行, %zu 字节\n", i + 1, *write_result);
    }
    
    // 刷新并关闭文件
    file_handle.flush();
    file_handle.close();
    
    printf("流式写入完成\n");
    
    // 打开文件进行读取
    auto read_file_result = sd_card.open_file("/stream_test.txt", "r");
    if (read_file_result.is_ok()) {
        auto read_handle = std::move(*read_file_result);
        
        printf("\n流式读取结果:\n");
        while (true) {
            auto line_result = read_handle.read_line();
            if (line_result.is_error() || line_result->empty()) {
                break;
            }
            printf("%s\n", line_result->c_str());
        }
        
        read_handle.close();
    }
}

void demonstrate_error_handling(SDCard& sd_card) {
    printf("\n=== 错误处理演示 ===\n");
    
    // 尝试读取不存在的文件
    auto read_result = sd_card.read_file("/nonexistent.txt");
    if (read_result.is_error()) {
        printf("预期的错误: %s (错误代码: %d)\n", 
               read_result.error_message().c_str(),
               static_cast<int>(read_result.error_code()));
        printf("错误描述: %s\n", 
               SDCard::get_error_description(read_result.error_code()).c_str());
    }
    
    // 尝试打开不存在的目录
    auto list_result = sd_card.list_directory("/nonexistent_dir");
    if (list_result.is_error()) {
        printf("预期的错误: %s\n", list_result.error_message().c_str());
    }
}

int main() {
    // 初始化Pico SDK
    stdio_init_all();
    
    // 等待串口连接 - 显示倒计时
    printf("程序启动中，等待串口连接...\n");
    for (int i = 5; i > 0; i--) {
        printf("启动倒计时: %d 秒\r", i);
        fflush(stdout);  // 强制刷新输出缓冲区
        sleep_ms(1000);
    }
    printf("启动完成!        \n\n");  // 多几个空格覆盖倒计时文字
    
    printf("========================================\n");
    printf("       MicroSD库使用示例\n");
    printf("========================================\n");
    printf("程序已启动！\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n\n");
    
    // 创建全局SD卡实例
    SPIConfig config;  // 使用默认的引脚配置
    SDCard sd_card(config);
    
    // 初始化SD卡
    printf("正在初始化SD卡...\n");
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败: %s\n", init_result.error_message().c_str());
        printf("程序终止\n");
        while (true) {
            sleep_ms(1000);
        }
    }
    printf("SD卡初始化成功!\n\n");
    
    // 基本操作演示
    demonstrate_basic_operations(sd_card);
    
    // 流式操作演示
    demonstrate_stream_operations(sd_card);
    
    // 错误处理演示
    demonstrate_error_handling(sd_card);
    
    printf("\n演示完成!\n");
    
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
} 