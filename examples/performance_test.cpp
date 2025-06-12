/**
 * @file performance_test.cpp
 * @brief MicroSD卡综合测试程序
 * 
 * 注意：请确保按照以下方式连接MicroSD模块，或修改include/spi_config.hpp中的引脚配置：
 * 
 * MicroSD模块      Raspberry Pi Pico
 * -----------     -----------------
 * VCC         ->  3.3V (Pin 36)
 * GND         ->  GND  (Pin 38)  
 * MISO        ->  GPIO7 (Pin 10)
 * MOSI        ->  GPIO0 (Pin 1)
 * SCK         ->  GPIO6 (Pin 9)
 * CS          ->  GPIO1 (Pin 2)
 * 
 * 本程序包含以下测试：
 * 1. 基本功能测试
 *    - 文件系统信息
 *    - 基本文件操作
 *    - 目录操作
 * 2. 高级功能测试
 *    - 文件搜索
 *    - 批量文件操作
 *    - 大文件处理
 * 3. 性能测试
 *    - 顺序读写速度测试
 *    - 随机读写速度测试
 *    - 小文件批量操作测试
 *    - 大文件读写测试
 *    - 持续写入压力测试
 */

#include <stdio.h>
#include <string>
#include <array>
#include <random>
#include <chrono>
#include <algorithm>
#include "pico/stdlib.h"
#include "micro_sd.hpp"

using namespace MicroSD;
using namespace std::chrono;

// 测试配置
constexpr size_t CHUNK_SIZE = 32 * 1024;     // 32KB 块大小
constexpr size_t SMALL_FILE_SIZE = 4 * 1024;  // 4KB
constexpr size_t MEDIUM_FILE_SIZE = 256 * 1024; // 256KB
constexpr size_t LARGE_FILE_SIZE = 512 * 1024;  // 512KB
constexpr int NUM_FILES = 10;                   // 测试文件数量
constexpr int TEST_DURATION = 30;               // 测试时间（秒）
constexpr size_t MAX_RESULTS = 5;

// 测试数据生成器
class TestDataGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::array<uint8_t, CHUNK_SIZE> buffer;

public:
    TestDataGenerator() : gen(rd()), dis(0, 255) {}

    // 生成固定大小的测试数据块
    const std::array<uint8_t, CHUNK_SIZE>& generate_chunk() {
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            buffer[i] = static_cast<uint8_t>(dis(gen));
        }
        return buffer;
    }
};

// 性能测试结果结构体
struct TestResult {
    double duration_ms;
    double speed_mbps;
    size_t total_bytes;
    std::string operation;

    TestResult() : duration_ms(0), speed_mbps(0), total_bytes(0) {}

    TestResult(double d, double s, size_t t, std::string op)
        : duration_ms(d), speed_mbps(s), total_bytes(t), operation(std::move(op)) {}
};

// 打印测试结果
void print_result(const TestResult& result) {
    printf("测试项目: %s\n", result.operation.c_str());
    printf("  耗时: %.2f ms\n", result.duration_ms);
    printf("  速度: %.2f MB/s\n", result.speed_mbps);
    printf("  总字节数: %zu bytes\n\n", result.total_bytes);
}

// 测试结果管理类
class TestResults {
private:
    static constexpr size_t MAX_RESULTS = 10;
    std::array<TestResult, MAX_RESULTS> results;
    size_t count = 0;

public:
    void add(const TestResult& result) {
        if (count < MAX_RESULTS) {
            results[count++] = result;
            print_result(result);
        }
    }

    size_t size() const {
        return count;
    }

    const TestResult& get(size_t index) const {
        return results[index];
    }

    void print_summary() const {
        printf("\n=== 测试结果汇总 ===\n\n");
        for (size_t i = 0; i < count; ++i) {
            print_result(results[i]);
        }
    }
};

// 文件信息打印
void print_file_info(const FileInfo& info) {
    printf("文件名: %s\n", info.name.c_str());
    printf("完整路径: %s\n", info.full_path.c_str());
    printf("大小: %zu 字节\n", info.size);
    printf("类型: %s\n", info.is_directory ? "目录" : "文件");
    printf("---\n");
}

// 等待用户确认
bool wait_for_user_confirmation(const char* prompt) {
    printf("\n%s (y/n): ", prompt);
    fflush(stdout);
    
    while (true) {
        char c = getchar();
        if (c == 'y' || c == 'Y') {
            printf("\n");
            return true;
        }
        if (c == 'n' || c == 'N') {
            printf("\n");
            return false;
        }
    }
}

// 基本功能测试
void demonstrate_basic_operations(SDCard& sd_card) {
    printf("\n=== 基本功能测试 ===\n");
    
    // 获取文件系统信息
    printf("文件系统类型: %s\n", sd_card.get_filesystem_type().c_str());
    
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("总容量: %.2f MB\n", total / 1024.0 / 1024.0);
        printf("可用容量: %.2f MB\n", free / 1024.0 / 1024.0);
    }
    
    // 列出根目录内容
    printf("\n=== 根目录内容 ===\n");
    auto list_result = sd_card.list_directory("/");
    if (list_result.is_ok()) {
        for (const auto& file : *list_result) {
            print_file_info(file);
        }
    }
    
    // 创建测试目录
    printf("\n=== 创建测试目录 ===\n");
    auto mkdir_result = sd_card.create_directory("/test_dir");
    if (mkdir_result.is_ok()) {
        printf("创建目录 '/test_dir' 成功\n");
    }
    
    // 写入测试文件
    printf("\n=== 写入测试文件 ===\n");
    std::string test_content = "Hello, MicroSD!\n这是一个测试文件。\n";
    auto write_result = sd_card.write_text_file("/test_dir/hello.txt", test_content);
    if (write_result.is_ok()) {
        printf("写入文件成功\n");
    }
    
    // 读取测试文件
    printf("\n=== 读取测试文件 ===\n");
    auto read_result = sd_card.read_file("/test_dir/hello.txt");
    if (read_result.is_ok()) {
        std::string content(read_result->begin(), read_result->end());
        printf("文件内容:\n%s\n", content.c_str());
    }
}

// 文件搜索功能
std::vector<FileInfo> search_files_by_extension(SDCard& sd_card, 
                                               const std::string& directory,
                                               const std::string& extension) {
    std::vector<FileInfo> matching_files;
    
    auto list_result = sd_card.list_directory(directory);
    if (list_result.is_error()) {
        return matching_files;
    }
    
    for (const auto& file : *list_result) {
        if (file.is_directory) {
            auto sub_results = search_files_by_extension(sd_card, file.full_path, extension);
            matching_files.insert(matching_files.end(), sub_results.begin(), sub_results.end());
        } else {
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

// 高级功能测试
void demonstrate_advanced_operations(SDCard& sd_card) {
    printf("\n=== 高级功能测试 ===\n");
    
    // 批量文件操作
    const std::string base_dir = "/batch_test";
    
    // 先删除旧目录（如果存在）
    auto old_files = sd_card.list_directory(base_dir);
    if (old_files.is_ok()) {
        for (const auto& file : *old_files) {
            if (!file.is_directory) {
                sd_card.delete_file(file.full_path);
            }
        }
        sd_card.remove_directory(base_dir);
    }
    
    // 创建测试目录结构
    std::vector<std::string> dirs = {
        "/batch_test",
        "/batch_test/images",
        "/batch_test/documents",
        "/batch_test/audio",
        "/batch_test/others"
    };
    
    printf("\n创建测试目录结构...\n");
    for (const auto& dir : dirs) {
        auto mkdir_result = sd_card.create_directory(dir);
        if (mkdir_result.is_ok()) {
            printf("创建目录: %s\n", dir.c_str());
        }
    }
    
    // 创建测试文件
    printf("\n创建测试文件...\n");
    std::vector<std::pair<std::string, std::string>> test_files = {
        {"/batch_test/images/photo1.jpg", "这是一张照片的数据"},
        {"/batch_test/images/photo2.jpg", "这是另一张照片的数据"},
        {"/batch_test/images/image.png", "这是一个PNG图片的数据"},
        {"/batch_test/documents/readme.txt", "这是一个说明文件"},
        {"/batch_test/documents/notes.txt", "这是一些笔记"},
        {"/batch_test/documents/report.pdf", "这是一个PDF文档"},
        {"/batch_test/audio/song1.mp3", "这是一首MP3歌曲"},
        {"/batch_test/audio/song2.mp3", "这是另一首MP3歌曲"},
        {"/batch_test/audio/music.wav", "这是一个WAV音频文件"},
        {"/batch_test/others/test.txt", "这是一个测试文件"}
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
        printf("\n搜索 %s 文件:\n", ext.c_str());
        auto matching_files = search_files_by_extension(sd_card, "/batch_test", ext);
        
        if (matching_files.empty()) {
            printf("未找到%s文件\n", ext.c_str());
        } else {
            for (const auto& file : matching_files) {
                printf("  找到: %s (大小: %zu 字节)\n", 
                       file.full_path.c_str(), file.size);
            }
        }
    }
    
    printf("\n文件搜索演示完成\n");
}

// 顺序写入测试
TestResult sequential_write_test(SDCard& sd_card, size_t file_size) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t written = 0;
    
    auto file_result = sd_card.open_file("/seq_test.bin", "w");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "顺序写入测试（失败）");
    }
    
    auto& file = *file_result;
    while (written < file_size) {
        const auto& chunk = gen.generate_chunk();
        size_t to_write = std::min(CHUNK_SIZE, file_size - written);
        
        auto write_result = file.write(std::vector<uint8_t>(chunk.begin(), chunk.begin() + to_write));
        if (write_result.is_error()) {
            return TestResult(0, 0, 0, "顺序写入测试（失败）");
        }
        
        written += to_write;
    }
    
    file.close();
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double speed = (written / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    return TestResult(
        static_cast<double>(duration.count()),
        speed,
        written,
        "顺序写入测试"
    );
}

// 顺序读取测试
TestResult sequential_read_test(SDCard& sd_card, size_t file_size) {
    auto start = high_resolution_clock::now();
    size_t total_read = 0;
    
    auto file_result = sd_card.open_file("/seq_test.bin", "r");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "顺序读取测试（失败）");
    }
    
    auto& file = *file_result;
    
    while (total_read < file_size) {
        size_t to_read = std::min(CHUNK_SIZE, file_size - total_read);
        auto read_result = file.read(to_read);
        
        if (read_result.is_error() || read_result->empty()) {
            break;
        }
        
        total_read += read_result->size();
    }
    
    file.close();
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double speed = (total_read / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    return TestResult(
        static_cast<double>(duration.count()),
        speed,
        total_read,
        "顺序读取测试"
    );
}

// 小文件批量操作测试
TestResult small_files_test(SDCard& sd_card) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t total_bytes = 0;
    
    auto dir_result = sd_card.create_directory("/test_files");
    if (dir_result.is_error()) {
        return TestResult(0, 0, 0, "小文件测试（失败）");
    }
    
    for (int i = 0; i < NUM_FILES; ++i) {
        std::string filename = "/test_files/small_" + std::to_string(i) + ".bin";
        auto file_result = sd_card.open_file(filename, "w");
        if (file_result.is_error()) {
            continue;
        }
        
        auto& file = *file_result;
        const auto& data = gen.generate_chunk();
        auto write_result = file.write(std::vector<uint8_t>(data.begin(), data.begin() + SMALL_FILE_SIZE));
        
        if (write_result.is_ok()) {
            total_bytes += SMALL_FILE_SIZE;
        }
        
        file.close();
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double speed = (total_bytes / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    return TestResult(
        static_cast<double>(duration.count()),
        speed,
        total_bytes,
        "小文件批量操作测试"
    );
}

// 压力测试
TestResult stress_test(SDCard& sd_card) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t total_bytes = 0;
    const auto test_duration = seconds(TEST_DURATION);
    printf("开始压力测试，持续时间：%d秒\n", TEST_DURATION);
    
    auto test_start = high_resolution_clock::now();
    int last_seconds = TEST_DURATION;
    
    while (duration_cast<seconds>(high_resolution_clock::now() - test_start) < test_duration) {
        int remaining = TEST_DURATION - duration_cast<seconds>(high_resolution_clock::now() - test_start).count();
        if (remaining < last_seconds) {
            printf("\r剩余时间：%d秒 ", remaining);
            fflush(stdout);
            last_seconds = remaining;
        }
        
        const auto& chunk = gen.generate_chunk();
        std::string filename = "/stress_test_" + std::to_string(total_bytes) + ".bin";
        
        auto file_result = sd_card.open_file(filename, "w");
        if (file_result.is_error()) {
            printf("\n创建压力测试文件失败\n");
            return TestResult(0, 0, 0, "压力测试（失败）");
        }
        
        auto& file = *file_result;
        auto write_result = file.write(std::vector<uint8_t>(chunk.begin(), chunk.begin() + SMALL_FILE_SIZE));
        if (write_result.is_error()) {
            printf("\n写入压力测试文件失败\n");
            file.close();
            return TestResult(0, 0, 0, "压力测试（失败）");
        }
        
        file.close();
        total_bytes += SMALL_FILE_SIZE;
        
        auto read_file_result = sd_card.open_file(filename, "r");
        if (read_file_result.is_error()) {
            printf("\n打开压力测试文件失败\n");
            return TestResult(0, 0, 0, "压力测试（失败）");
        }
        
        auto& read_file = *read_file_result;
        auto read_result = read_file.read(SMALL_FILE_SIZE);
        if (read_result.is_error()) {
            printf("\n读取压力测试文件失败\n");
            read_file.close();
            return TestResult(0, 0, 0, "压力测试（失败）");
        }
        
        read_file.close();
        
        auto delete_result = sd_card.delete_file(filename);
        if (delete_result.is_error()) {
            printf("\n删除压力测试文件失败\n");
            return TestResult(0, 0, 0, "压力测试（失败）");
        }
        
        sleep_ms(100);
    }
    
    printf("\n压力测试完成！\n");
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double speed = (total_bytes * 2 / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    return TestResult(
        static_cast<double>(duration.count()),
        speed,
        total_bytes * 2,
        "压力测试"
    );
}

// 大文件测试
TestResult large_file_test(SDCard& sd_card) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t total_bytes = 0;
    
    auto file_result = sd_card.open_file("/large_test.bin", "w");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "大文件测试（失败）");
    }
    
    auto& file = *file_result;
    size_t remaining = LARGE_FILE_SIZE;
    
    while (remaining > 0) {
        const auto& chunk = gen.generate_chunk();
        size_t to_write = std::min(CHUNK_SIZE, remaining);
        
        auto write_result = file.write(std::vector<uint8_t>(chunk.begin(), chunk.begin() + to_write));
        if (write_result.is_error()) {
            file.close();
            return TestResult(0, 0, 0, "大文件测试（失败）");
        }
        
        total_bytes += to_write;
        remaining -= to_write;
    }
    
    file.close();
    
    file_result = sd_card.open_file("/large_test.bin", "r");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "大文件测试（失败）");
    }
    
    auto& read_file = *file_result;
    remaining = LARGE_FILE_SIZE;
    
    while (remaining > 0) {
        size_t to_read = std::min(CHUNK_SIZE, remaining);
        auto read_result = read_file.read(to_read);
        
        if (read_result.is_error() || read_result->empty()) {
            read_file.close();
            return TestResult(0, 0, 0, "大文件测试（失败）");
        }
        
        remaining -= read_result->size();
    }
    
    read_file.close();
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double speed = (total_bytes * 2 / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    return TestResult(
        static_cast<double>(duration.count()),
        speed,
        total_bytes * 2,
        "大文件读写测试"
    );
}

int main() {
    stdio_init_all();
    sleep_ms(3000); // 等待串口连接
    
    printf("\n=== MicroSD卡综合测试程序 ===\n\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n\n");
    
    // 初始化SD卡
    SDCard sd_card;
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD卡初始化失败\n");
        return 1;
    }
    
    // 获取容量信息
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("SD卡总容量: %.2f GB\n", total / (1024.0 * 1024.0 * 1024.0));
        printf("可用空间: %.2f GB\n", free / (1024.0 * 1024.0 * 1024.0));
        printf("文件系统类型: %s\n\n", sd_card.get_filesystem_type().c_str());
    }
    
    // 提示格式化
    printf("警告：性能测试需要先格式化SD卡！\n");
    printf("格式化将清除卡上的所有数据！\n");
    
    if (!wait_for_user_confirmation("是否继续？这将删除SD卡上的所有数据")) {
        printf("测试已取消\n");
        return 0;
    }
    
    printf("正在格式化SD卡...\n");
    auto format_result = sd_card.format("FAT32");
    if (format_result.is_error()) {
        printf("格式化失败：%s\n", sd_card.get_error_description(format_result.error_code()).c_str());
        return 1;
    }
    printf("格式化完成！\n\n");
    
    // 重新挂载文件系统
    init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("重新挂载文件系统失败\n");
        return 1;
    }
    
    // 显示格式化后的容量信息
    capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("格式化后容量信息：\n");
        printf("总容量: %.2f GB\n", total / (1024.0 * 1024.0 * 1024.0));
        printf("可用空间: %.2f GB\n", free / (1024.0 * 1024.0 * 1024.0));
        printf("文件系统类型: %s\n\n", sd_card.get_filesystem_type().c_str());
    }
    
    if (!wait_for_user_confirmation("是否开始测试？")) {
        printf("测试已取消\n");
        return 0;
    }
    
    // 运行基本功能测试
    demonstrate_basic_operations(sd_card);
    
    // 运行高级功能测试
    demonstrate_advanced_operations(sd_card);
    
    // 运行性能测试
    TestResults results;
    
    printf("\n=== 开始性能测试 ===\n");
    
    printf("\n运行顺序写入测试...\n");
    results.add(sequential_write_test(sd_card, LARGE_FILE_SIZE));
    
    printf("\n运行顺序读取测试...\n");
    results.add(sequential_read_test(sd_card, LARGE_FILE_SIZE));
    
    printf("\n运行小文件测试...\n");
    results.add(small_files_test(sd_card));
    
    printf("\n运行压力测试...\n");
    results.add(stress_test(sd_card));
    
    printf("\n运行大文件测试...\n");
    results.add(large_file_test(sd_card));
    
    // 打印测试结果汇总
    results.print_summary();
    
    printf("\n=== 所有测试已完成！===\n");
    printf("测试结果已保存，程序将继续运行以保持串口连接。\n");
    
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
} 