/**
 * @file debug_simple.cpp
 * @brief 简化的MicroSD调试示例
 * @version 1.0.0
 */

#include <iostream>
#include <stdio.h>
#include "pico/stdlib.h"
#include "micro_sd.hpp"

using namespace MicroSD;

int main() {
    // 初始化标准库
    stdio_init_all();
    
    // 等待USB串口连接 - 显示倒计时
    printf("调试程序启动中，等待USB串口连接...\n");
    for (int i = 5; i > 0; i--) {
        printf("启动倒计时: %d 秒\r", i);
        fflush(stdout);  // 强制刷新输出缓冲区
        sleep_ms(1000);
    }
    printf("启动完成!        \n\n");  // 多几个空格覆盖倒计时文字
    
    printf("========================================\n");
    printf("       MicroSD 调试示例\n");  
    printf("========================================\n");
    printf("程序已启动！USB串口连接成功！\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("如果您看到这条消息，说明串口工作正常！\n");
    printf("========================================\n\n");
    
    // 创建SDCard实例（使用默认引脚配置）
    printf("初始化SD卡...\n");
    SDCard sd_card;
    
    // 初始化SD卡
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("错误: SD卡初始化失败\n");
        printf("错误码: %d\n", static_cast<int>(init_result.error_code()));
        printf("错误信息: %s\n", init_result.error_message().c_str());
        printf("\n请检查:\n");
        printf("1. SD卡是否插入\n");
        printf("2. 硬件连接是否正确\n");
        printf("3. SD卡是否支持FAT32格式\n");
        
        while (true) {
            printf("等待修复后重启...\n");
            sleep_ms(5000);
        }
    }
    
    printf("✓ SD卡初始化成功!\n");
    
    // 显示文件系统信息
    printf("文件系统类型: %s\n", sd_card.get_filesystem_type().c_str());
    
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("总容量: %.2f MB\n", total / (1024.0 * 1024.0));
        printf("可用容量: %.2f MB\n", free / (1024.0 * 1024.0));
    }
    
    printf("\n测试基本文件操作...\n");
    
    // 测试写入文件
    printf("创建测试文件...\n");
    std::string test_content = "Hello from Raspberry Pi Pico!\nMicroSD test successful!\n";
    auto write_result = sd_card.write_text_file("/test.txt", test_content);
    
    if (write_result.is_error()) {
        printf("错误: 写入文件失败\n");
        printf("错误信息: %s\n", write_result.error_message().c_str());
    } else {
        printf("✓ 文件写入成功!\n");
    }
    
    // 测试读取文件
    printf("读取测试文件...\n");
    auto read_result = sd_card.read_file("/test.txt");
    
    if (read_result.is_error()) {
        printf("错误: 读取文件失败\n");
        printf("错误信息: %s\n", read_result.error_message().c_str());
    } else {
        printf("✓ 文件读取成功!\n");
        printf("文件内容:\n%s\n", 
               std::string(read_result->begin(), read_result->end()).c_str());
    }
    
    // 测试列出文件
    printf("列出根目录文件...\n");
    auto list_result = sd_card.list_directory("/");
    
    if (list_result.is_error()) {
        printf("错误: 列出目录失败\n");
        printf("错误信息: %s\n", list_result.error_message().c_str());
    } else {
        printf("✓ 目录列出成功!\n");
        printf("找到 %zu 个文件/目录:\n", list_result->size());
        
        for (const auto& file : *list_result) {
            printf("  %s %s (%zu bytes)\n", 
                   file.is_directory ? "[DIR]" : "[FILE]",
                   file.name.c_str(),
                   file.size);
        }
    }
    
    printf("\n================================\n");
    printf("  测试完成! SD卡工作正常\n");
    printf("================================\n");
    
    // 主循环
    int counter = 0;
    while (true) {
        printf("运行中... %d 秒\n", counter++);
        sleep_ms(1000);
        
        // 每10秒显示一次状态
        if (counter % 10 == 0) {
            printf("系统状态: 正常运行\n");
            printf("SD卡状态: 已连接\n");
        }
    }
    
    return 0;
} 