/**
 * @file pin_config.hpp
 * @brief Pin Configuration and SPI Settings for MicroSD Card
 * @version 2.0.0
 * 
 * 默认接线方案 (可根据实际接线修改):
 * GPIO10(SCK) -> SPI时钟信号
 * GPIO11(MISO) -> 主入从出
 * GPIO12(MOSI) -> 主出从入  
 * GPIO13(CS) -> 片选信号
 * VCC -> 3.3V
 * GND -> GND
 */

#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <string>

namespace MicroSD {

// === 引脚定义 ===
#define PIN_MISO_DEFAULT        11      // MISO引脚 (GPIO11)
#define PIN_CS_DEFAULT          13      // CS引脚 (GPIO13)
#define PIN_SCK_DEFAULT         10      // SCK引脚 (GPIO10)
#define PIN_MOSI_DEFAULT        12      // MOSI引脚 (GPIO12)

// === 频率定义 ===
#define SPI_CLK_SLOW_DEFAULT    (400 * 1000)      // 慢速时钟频率 (400KHz用于初始化)
#define SPI_CLK_FAST_DEFAULT    (40 * 1000 * 1000) // 快速时钟频率 (40MHz用于正常操作)
#define SPI_CLK_SLOW_COMPAT     (200 * 1000)      // 兼容性慢速频率
#define SPI_CLK_FAST_COMPAT     (20 * 1000 * 1000) // 兼容性快速频率
#define SPI_CLK_FAST_HIGH       (50 * 1000 * 1000) // 高速频率

// === 配置标志 ===
#define USE_INTERNAL_PULLUP_DEFAULT  true    // 默认使用内部上拉电阻

/**
 * @brief 引脚配置结构体
 */
struct PinConfig {
    uint pin_miso = PIN_MISO_DEFAULT;        // MISO引脚
    uint pin_cs = PIN_CS_DEFAULT;            // CS引脚
    uint pin_sck = PIN_SCK_DEFAULT;          // SCK引脚
    uint pin_mosi = PIN_MOSI_DEFAULT;        // MOSI引脚
    bool use_internal_pullup = USE_INTERNAL_PULLUP_DEFAULT;  // 使用内部上拉电阻
    
    // 引脚功能验证
    bool is_valid() const {
        return pin_miso <= 29 && pin_cs <= 29 && 
               pin_sck <= 29 && pin_mosi <= 29;
    }
    
    // 获取引脚描述
    std::string get_description() const {
        return "MISO:" + std::to_string(pin_miso) + 
               " CS:" + std::to_string(pin_cs) + 
               " SCK:" + std::to_string(pin_sck) + 
               " MOSI:" + std::to_string(pin_mosi);
    }
};

/**
 * @brief SPI配置结构体
 */
struct SPIConfig {
    spi_inst_t* spi_port = spi0;          // SPI端口
    uint32_t clk_slow = SPI_CLK_SLOW_DEFAULT;       // 慢速时钟频率
    uint32_t clk_fast = SPI_CLK_FAST_DEFAULT;       // 快速时钟频率
    PinConfig pins;                       // 引脚配置
    
    // 验证配置
    bool is_valid() const {
        return spi_port != nullptr && pins.is_valid();
    }
    
    // 获取配置描述
    std::string get_description() const {
        return "SPI" + std::to_string(spi_port == spi0 ? 0 : 1) + 
               " Slow:" + std::to_string(clk_slow/1000) + "KHz" +
               " Fast:" + std::to_string(clk_fast/1000000) + "MHz" +
               " Pins:" + pins.get_description();
    }
};

/**
 * @brief 预定义配置
 */
namespace Config {
    // 默认配置
    inline const SPIConfig DEFAULT = {
        .spi_port = spi0,
        .clk_slow = SPI_CLK_SLOW_DEFAULT,
        .clk_fast = SPI_CLK_FAST_DEFAULT,
        .pins = {PIN_MISO_DEFAULT, PIN_CS_DEFAULT, PIN_SCK_DEFAULT, PIN_MOSI_DEFAULT, USE_INTERNAL_PULLUP_DEFAULT}
    };
    
    // 高速配置
    inline const SPIConfig HIGH_SPEED = {
        .spi_port = spi0,
        .clk_slow = SPI_CLK_SLOW_DEFAULT,
        .clk_fast = SPI_CLK_FAST_HIGH,
        .pins = {PIN_MISO_DEFAULT, PIN_CS_DEFAULT, PIN_SCK_DEFAULT, PIN_MOSI_DEFAULT, USE_INTERNAL_PULLUP_DEFAULT}
    };
    
    // 兼容性配置 (较低频率)
    inline const SPIConfig COMPATIBLE = {
        .spi_port = spi0,
        .clk_slow = SPI_CLK_SLOW_COMPAT,
        .clk_fast = SPI_CLK_FAST_COMPAT,
        .pins = {PIN_MISO_DEFAULT, PIN_CS_DEFAULT, PIN_SCK_DEFAULT, PIN_MOSI_DEFAULT, USE_INTERNAL_PULLUP_DEFAULT}
    };
}

} // namespace MicroSD 