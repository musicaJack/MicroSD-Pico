/**
 * @file micro_sd.hpp
 * @brief Modern C++ MicroSD Card Library for Raspberry Pi Pico
 * @author 重构自RPi_Pico_WAV_Player项目
 * @version 1.0.0
 * 
 * 基于您的接线：
 * GPIO10(SCK) -> SPI时钟信号
 * GPIO11(MISO) -> 主机接收数据
 * GPIO12(MOSI) -> 主机发送数据  
 * GPIO13(CS) -> 片选信号
 * VCC -> 3.3V
 * GND -> GND
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <optional>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"

namespace MicroSD {

/**
 * @brief SPI配置结构体
 */
struct SPIConfig {
    spi_inst_t* spi_port = spi0;          // SPI端口
    uint32_t clk_slow = 400 * 1000;       // 慢时钟频率 (400KHz 初始化用)
    uint32_t clk_fast = 40 * 1000 * 1000; // 快时钟频率 (40MHz 正常操作用)
    uint pin_miso = 11;                   // MISO引脚 (GPIO11)
    uint pin_cs = 13;                     // CS引脚 (GPIO13)
    uint pin_sck = 10;                    // SCK引脚 (GPIO10)
    uint pin_mosi = 12;                   // MOSI引脚 (GPIO12)
    bool use_internal_pullup = true;      // 使用内部上拉电阻
};

/**
 * @brief 文件信息结构体
 */
struct FileInfo {
    std::string name;           // 文件名
    std::string full_path;      // 完整路径
    size_t size;               // 文件大小（字节）
    bool is_directory;         // 是否为目录
    uint8_t attributes;        // 文件属性
    
    // C++17 结构化绑定支持
    auto tie() const { return std::tie(name, full_path, size, is_directory, attributes); }
};

/**
 * @brief 错误类型枚举
 */
enum class ErrorCode {
    SUCCESS = 0,
    INIT_FAILED,
    MOUNT_FAILED,
    FILE_NOT_FOUND,
    PERMISSION_DENIED,
    DISK_FULL,
    IO_ERROR,
    INVALID_PARAMETER,
    FATFS_ERROR,
    UNKNOWN_ERROR
};

/**
 * @brief 结果模板类 - 现代C++错误处理方式
 */
template<typename T>
class Result {
private:
    std::optional<T> value_;
    ErrorCode error_code_;
    std::string error_message_;

public:
    Result(T&& value) : value_(std::move(value)), error_code_(ErrorCode::SUCCESS) {}
    Result(const T& value) : value_(value), error_code_(ErrorCode::SUCCESS) {}
    Result(ErrorCode error, std::string message = "")
        : error_code_(error), error_message_(std::move(message)) {}

    bool is_ok() const { return error_code_ == ErrorCode::SUCCESS; }
    bool is_error() const { return !is_ok(); }
    ErrorCode error_code() const { return error_code_; }
    const std::string& error_message() const { return error_message_; }
    const T& operator*() const { return value_.value(); }
    T& operator*() { return value_.value(); }
    const T* operator->() const { return &value_.value(); }
    T* operator->() { return &value_.value(); }
};

// Result<void>的完全特化
template<>
class Result<void> {
private:
    ErrorCode error_code_;
    std::string error_message_;

public:
    Result() : error_code_(ErrorCode::SUCCESS) {}
    Result(ErrorCode error, std::string message = "")
        : error_code_(error), error_message_(std::move(message)) {}

    bool is_ok() const { return error_code_ == ErrorCode::SUCCESS; }
    bool is_error() const { return !is_ok(); }
    ErrorCode error_code() const { return error_code_; }
    const std::string& error_message() const { return error_message_; }
};

/**
 * @brief MicroSD卡管理器类
 */
class SDCard {
private:
    SPIConfig config_;
    FATFS fs_;
    bool is_mounted_;
    uint8_t fs_type_;
    
    // RAII资源管理
    std::unique_ptr<DIR> current_dir_;
    std::string current_path_;
    
    // 私有方法
    void initialize_spi();
    void deinitialize_spi();
    Result<void> mount_filesystem();
    void unmount_filesystem();
    
public:
    /**
     * @brief 构造函数
     * @param config SPI配置，使用默认配置如果不提供
     */
    explicit SDCard(SPIConfig config = SPIConfig{});
    
    /**
     * @brief 析构函数 - RAII自动清理资源
     */
    ~SDCard();
    
    // 禁用拷贝构造和赋值（资源管理安全）
    SDCard(const SDCard&) = delete;
    SDCard& operator=(const SDCard&) = delete;
    
    // 支持移动语义
    SDCard(SDCard&& other) noexcept;
    SDCard& operator=(SDCard&& other) noexcept;
    
    /**
     * @brief 初始化SD卡
     * @return 初始化结果
     */
    Result<void> initialize();
    
    /**
     * @brief 检查SD卡是否已挂载
     */
    bool is_mounted() const { return is_mounted_; }
    
    /**
     * @brief 获取文件系统类型
     */
    std::string get_filesystem_type() const;
    
    /**
     * @brief 获取SD卡容量信息
     * @return 总容量和可用容量（字节）
     */
    Result<std::pair<size_t, size_t>> get_capacity() const;
    
    // === 目录操作 ===
    
    /**
     * @brief 打开目录
     * @param path 目录路径
     * @return 操作结果
     */
    Result<void> open_directory(const std::string& path);
    
    /**
     * @brief 获取当前目录路径
     */
    std::string get_current_directory() const { return current_path_; }
    
    /**
     * @brief 列出目录内容
     * @param path 目录路径，空字符串表示当前目录
     * @return 文件列表
     */
    Result<std::vector<FileInfo>> list_directory(const std::string& path = "");
    
    /**
     * @brief 创建目录
     * @param path 目录路径
     * @return 操作结果
     */
    Result<void> create_directory(const std::string& path);
    
    /**
     * @brief 删除目录（必须为空）
     * @param path 目录路径
     * @return 操作结果
     */
    Result<void> remove_directory(const std::string& path);
    
    // === 文件操作 ===
    
    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return 文件是否存在
     */
    bool file_exists(const std::string& path) const;
    
    /**
     * @brief 获取文件信息
     * @param path 文件路径
     * @return 文件信息
     */
    Result<FileInfo> get_file_info(const std::string& path) const;
    
    /**
     * @brief 读取整个文件到内存
     * @param path 文件路径
     * @return 文件内容
     */
    Result<std::vector<uint8_t>> read_file(const std::string& path) const;
    
    /**
     * @brief 读取文件的一部分
     * @param path 文件路径
     * @param offset 偏移量
     * @param size 读取大小
     * @return 文件内容
     */
    Result<std::vector<uint8_t>> read_file_chunk(const std::string& path, 
                                                  size_t offset, size_t size) const;
    
    /**
     * @brief 写入数据到文件
     * @param path 文件路径
     * @param data 要写入的数据
     * @param append 是否追加模式
     * @return 操作结果
     */
    Result<void> write_file(const std::string& path, 
                           const std::vector<uint8_t>& data, 
                           bool append = false);
    
    /**
     * @brief 写入字符串到文件
     * @param path 文件路径
     * @param content 字符串内容
     * @param append 是否追加模式
     * @return 操作结果
     */
    Result<void> write_text_file(const std::string& path, 
                                const std::string& content, 
                                bool append = false);
    
    /**
     * @brief 删除文件
     * @param path 文件路径
     * @return 操作结果
     */
    Result<void> delete_file(const std::string& path);
    
    /**
     * @brief 重命名文件或目录
     * @param old_path 原路径
     * @param new_path 新路径
     * @return 操作结果
     */
    Result<void> rename(const std::string& old_path, const std::string& new_path);
    
    /**
     * @brief 复制文件
     * @param src_path 源文件路径
     * @param dst_path 目标文件路径
     * @return 操作结果
     */
    Result<void> copy_file(const std::string& src_path, const std::string& dst_path);
    
    // === 流式文件操作 ===
    
    /**
     * @brief 文件句柄类 - RAII管理文件资源
     */
    class FileHandle {
    private:
        FIL file_;
        bool is_open_;
        std::string path_;
        
    public:
        FileHandle() : is_open_(false) {}
        ~FileHandle() { close(); }
        
        // 禁用拷贝，支持移动
        FileHandle(const FileHandle&) = delete;
        FileHandle& operator=(const FileHandle&) = delete;
        FileHandle(FileHandle&& other) noexcept;
        FileHandle& operator=(FileHandle&& other) noexcept;
        
        bool is_open() const { return is_open_; }
        const std::string& get_path() const { return path_; }
        
        Result<void> open(const std::string& path, const std::string& mode);
        void close();
        
        Result<std::vector<uint8_t>> read(size_t size);
        Result<std::string> read_line();
        Result<size_t> write(const std::vector<uint8_t>& data);
        Result<size_t> write(const std::string& text);
        
        Result<void> seek(size_t position);
        Result<size_t> tell() const;
        Result<size_t> size() const;
        Result<void> flush();
        
        friend class SDCard;
    };
    
    /**
     * @brief 打开文件获取句柄
     * @param path 文件路径
     * @param mode 打开模式 ("r", "w", "a", "r+", "w+", "a+")
     * @return 文件句柄
     */
    Result<FileHandle> open_file(const std::string& path, const std::string& mode);
    
    // === 实用工具方法 ===
    
    /**
     * @brief 同步数据到SD卡
     * @return 操作结果
     */
    Result<void> sync();
    
    /**
     * @brief 格式化SD卡 (危险操作!)
     * @param filesystem_type 文件系统类型 (FAT32等)
     * @return 操作结果
     */
    Result<void> format(const std::string& filesystem_type = "FAT32");
    
    /**
     * @brief 获取错误描述字符串
     * @param error_code 错误代码
     * @return 错误描述
     */
    static std::string get_error_description(ErrorCode error_code);
    
    /**
     * @brief 路径工具函数
     */
    static std::string normalize_path(const std::string& path);
    static std::string join_path(const std::string& dir, const std::string& file);
    static std::pair<std::string, std::string> split_path(const std::string& path);
};

// === 辅助函数 ===

/**
 * @brief FRESULT到ErrorCode的转换
 */
ErrorCode fresult_to_error_code(FRESULT fr);

} // namespace MicroSD 