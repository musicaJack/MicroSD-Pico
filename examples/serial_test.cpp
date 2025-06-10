/**
 * @file serial_test.cpp
 * @brief 串口输出测试程序
 * @author MicroSD-Pico项目
 * 
 * 这个程序专门用于测试串口输出是否正常工作
 * 如果您能看到这些消息，说明串口配置正确
 */

#include <cstdio>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"

int main() {
    // 初始化标准输入输出
    stdio_init_all();
    
    // 等待串口连接 - 显示倒计时
    printf("串口测试程序启动中...\n");
    for (int i = 3; i > 0; i--) {
        printf("启动倒计时: %d 秒\r", i);
        fflush(stdout);  // 强制刷新输出缓冲区
        sleep_ms(1000);
    }
    printf("启动完成!        \n\n");  // 多几个空格覆盖倒计时文字
    
    printf("========================================\n");
    printf("       串口输出测试程序\n");
    printf("========================================\n");
    printf("如果您能看到这条消息，说明串口工作正常！\n");
    printf("\n");
    
    // 显示系统信息
    printf("系统信息:\n");
    printf("- 板子: Raspberry Pi Pico\n");
    printf("- SDK版本: Pico SDK\n");
    printf("- 串口波特率: 115200\n");
    printf("- 编译时间: %s %s\n", __DATE__, __TIME__);
    printf("\n");
    
    // 测试连续输出
    printf("开始连续测试...\n");
    for (int i = 1; i <= 10; i++) {
        printf("测试消息 #%d - 时间戳: %llu ms\n", i, to_ms_since_boot(get_absolute_time()));
        sleep_ms(1000);
    }
    
    printf("\n");
    printf("========================================\n");
    printf("连续输出测试（每5秒一次）\n");
    printf("按Ctrl+C或断开连接可停止\n");
    printf("========================================\n");
    printf("\n");
    
    int counter = 1;
    while (true) {
        printf("[%04d] 心跳测试 - 运行时间: %llu ms\n", 
               counter++, to_ms_since_boot(get_absolute_time()));
        
        // LED闪烁指示（如果板上有LED）
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(100);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        
        sleep_ms(4900); // 总共5秒
    }
    
    return 0;
} 