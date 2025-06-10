# MicroSD-Pico

现代C++17 Raspberry Pi Pico MicroSD卡库，具有类型安全错误处理、RAII资源管理和完整文件操作功能。

## ✨ 特性

- **现代C++17** - 使用RAII、智能指针、移动语义和结构化绑定
- **类型安全错误处理** - 自定义Result<T>模板和完整错误码
- **完整文件操作** - 读取、写入、创建、删除、目录管理
- **流式操作** - 逐行文件处理，适合大文件
- **批量操作** - 高效的多文件处理
- **文件系统监控** - 容量追踪和完整性检查
- **USB串口调试** - 115200波特率实时状态输出
- **友好的用户体验** - 可视化倒计时，提升用户体验

## 🔧 硬件要求

### 组件清单
- Raspberry Pi Pico (或Pico W)
- MicroSD卡模块 (SPI接口)
- MicroSD卡 (建议FAT32格式)
- 杜邦线

### 接线图
```
MicroSD模块      Raspberry Pi Pico
-----------     -----------------
VCC         ->  3.3V (Pin 36)
GND         ->  GND  (Pin 38)  
MISO        ->  GPIO11 (Pin 15)
MOSI        ->  GPIO12 (Pin 16)
SCK         ->  GPIO10 (Pin 14)
CS          ->  GPIO13 (Pin 17)
```

## 🚀 快速开始

### 环境要求
- **Pico SDK v1.5.1** 包含ARM GNU工具链
- **CMake 3.13+**
- **Ninja构建系统**
- **Git** 用于克隆

### 安装步骤

1. **克隆仓库:**
```bash
git clone https://github.com/your-repo/MicroSD-Pico.git
cd MicroSD-Pico
```

2. **构建项目:**
```bash
# Windows系统
.\build_pico.bat

# 构建脚本会自动:
# - 配置环境变量
# - 设置ARM工具链路径
# - 使用CMake + Ninja编译
# - 自动生成UF2文件
```

3. **烧录到Pico:**
- 按住Pico的BOOTSEL按钮
- 通过USB连接 (应该出现RPI-RP2磁盘)
- 将`build`目录中的任意`.uf2`文件拖入
- 松开BOOTSEL，程序将开始运行

## 📁 生成的程序

构建完成后，您会在`build`目录中找到这些UF2文件:

### 🧪 `serial_test.uf2`
**用途:** 测试串口通信，不依赖SD卡
```
功能:
- 倒计时显示
- 每5秒心跳消息
- LED闪烁指示
- 系统状态信息
```

### 📚 `basic_example.uf2`
**用途:** 演示核心MicroSD功能
```
功能:
- SD卡初始化和检测
- 文件系统信息显示
- 基本文件读写操作
- 目录创建和列表
- 错误处理演示
```

### 🔧 `advanced_example.uf2`
**用途:** 展示高级功能
```
功能:
- 批量文件操作
- 大文件分块处理
- 文件系统监控
- 校验和完整性检查
- 递归目录操作
```

### 🐛 `debug_example.uf2`
**用途:** 简单调试和故障排除
```
功能:
- 逐步操作日志
- 详细错误消息
- 硬件连接验证
- 实时状态更新
```

## 💻 使用示例

```cpp
#include "micro_sd.hpp"
using namespace MicroSD;

int main() {
    stdio_init_all();
    
    // 使用默认引脚创建SD卡实例
    SPIConfig config; // 默认使用GPIO10-13
    SDCard sd_card(config);
    
    // 初始化SD卡
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败\n");
        return -1;
    }
    
    // 写入文件
    std::string content = "你好, MicroSD!";
    auto write_result = sd_card.write_text_file("/hello.txt", content);
    if (write_result.is_ok()) {
        printf("文件写入成功\n");
    }
    
    // 读取文件
    auto read_result = sd_card.read_file("/hello.txt");
    if (read_result.is_ok()) {
        std::string file_content(read_result->begin(), read_result->end());
        printf("文件内容: %s\n", file_content.c_str());
    }
    
    return 0;
}
```

## 🔍 调试指南

### 串口监视器设置
1. **连接USB** 烧录程序后
2. **打开串口监视器** (PuTTY、Arduino IDE、VS Code等)
3. **设置波特率** 为 `115200`
4. **观察倒计时** - 程序显示3-5秒启动倒计时

### 常见问题

| 问题 | 解决方案 |
|------|----------|
| 没有串口输出 | 检查USB连接和波特率(115200) |
| SD卡未检测到 | 验证接线并确保FAT32格式 |
| 构建失败 | 安装完整的Pico SDK和ARM工具链 |
| 权限拒绝 | 以管理员身份运行或检查文件权限 |

## 📖 API参考

### 核心类

#### `SDCard`
SD卡操作的主要接口
```cpp
// 构造函数
SDCard(const SPIConfig& config = SPIConfig{});

// 核心操作
Result<void> initialize();
Result<std::vector<uint8_t>> read_file(const std::string& path);
Result<void> write_text_file(const std::string& path, const std::string& content, bool append = false);
Result<std::vector<FileInfo>> list_directory(const std::string& path);
Result<void> create_directory(const std::string& path);
Result<void> delete_file(const std::string& path);
```

#### `Result<T>`
类型安全错误处理
```cpp
// 检查结果
if (result.is_ok()) {
    auto value = *result; // 获取值
} else {
    auto error = result.error_code();
    auto message = result.error_message();
}
```

### 配置

#### `SPIConfig`
硬件引脚配置
```cpp
struct SPIConfig {
    uint8_t pin_sck = 10;   // 时钟信号
    uint8_t pin_mosi = 12;  // 主机输出从机输入
    uint8_t pin_miso = 11;  // 主机输入从机输出
    uint8_t pin_cs = 13;    // 片选信号
    uint32_t baudrate = 12500000; // 12.5 MHz
};
```

## 🛠️ 构建系统

项目使用现代的CMake + Ninja构建系统:

### 环境设置
- **PICO_SDK_PATH**: 指向Pico SDK安装目录
- **ARM_TOOLCHAIN_PATH**: ARM GNU工具链二进制文件
- **CMAKE_PATH**: CMake可执行文件位置
- **NINJA_PATH**: Ninja构建系统

### 构建配置
- **C++标准**: C++17
- **构建类型**: Release (优化版本)
- **目标架构**: ARM Cortex-M0+
- **串口输出**: USB CDC (115200波特率)

## 📊 程序运行效果

### 启动序列
```
程序启动中，等待串口连接...
启动倒计时: 5 秒
启动倒计时: 4 秒
启动倒计时: 3 秒
启动倒计时: 2 秒
启动倒计时: 1 秒
启动完成!

========================================
       MicroSD库使用示例
========================================
程序已启动！
编译时间: Jun 10 2025 13:47:37
========================================
```

### 运行示例输出
```
正在初始化SD卡...
SD卡初始化成功!
文件系统类型: FAT32
总容量: 3466.00 MB
可用容量: 3465.97 MB

=== 根目录内容 ===
[DIR] System Volume Information (0 bytes)
[DIR] test_dir (0 bytes)
[FILE] stream_test.txt (161 bytes)

=== 创建测试目录 ===
目录 '/test_dir' 已存在，跳过创建

=== 写入测试文件 ===
写入文件 '/test_dir/hello.txt' 成功
```

## 🤝 贡献

1. Fork 这个仓库
2. 创建功能分支 (`git checkout -b feature-name`)
3. 提交您的更改 (`git commit -am 'Add feature'`)
4. 推送到分支 (`git push origin feature-name`)
5. 创建Pull Request

## 📄 许可证

本项目基于MIT许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

- **Raspberry Pi基金会** 提供优秀的Pico SDK
- **pico_fatfs** 库提供FAT文件系统支持
- **FatFs** by ChaN 提供底层文件系统实现

## 📞 技术支持

- **问题报告**: 通过GitHub Issues报告bug和请求功能
- **文档**: `/examples`目录包含完整示例
- **硬件**: 请确认连接符合上述接线图

---

*用 ❤️ 为Raspberry Pi Pico社区构建* 