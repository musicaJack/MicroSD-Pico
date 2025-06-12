/**
 * @file spi_config.hpp
 * @brief SPI配置文件
 * @version 1.0.0
 * 
 * 默认接线方案（按实际接线修改）：
 * GPIO10(SCK) -> SPI时钟信号
 * GPIO11(MISO) -> 主机接收数据
 * GPIO12(MOSI) -> 主机发送数据  
 * GPIO13(CS) -> 片选信号
 * VCC -> 3.3V
 * GND -> GND
 */

#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"

namespace MicroSD {

/**
 * @brief SPI配置结构体
 */
struct SPIConfig {
    spi_inst_t* spi_port = spi0;          // SPI端口
    uint32_t clk_slow = 400 * 1000;       // 慢时钟频率 (400KHz 初始化用)
    uint32_t clk_fast = 40 * 1000 * 1000; // 快时钟频率 (40MHz 正常操作用)
    uint pin_miso = 7;                     // MISO引脚 (GPIO7)
    uint pin_cs = 1;                       // CS引脚 (GPIO1)
    uint pin_sck = 6;                      // SCK引脚 (GPIO6)
    uint pin_mosi = 0;                     // MOSI引脚 (GPIO0)
    bool use_internal_pullup = true;       // 使用内部上拉电阻
};

} // namespace MicroSD 