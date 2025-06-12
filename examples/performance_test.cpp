/**
 * @file performance_test.cpp
 * @brief Comprehensive MicroSD Card Test Program
 * 
 * Note: Please ensure the MicroSD module is connected as follows, or modify the pin configuration in include/spi_config.hpp:
 * 
 * MicroSD Module    Raspberry Pi Pico
 * -------------     -----------------
 * VCC           ->  3.3V (Pin 36)
 * GND           ->  GND  (Pin 38)  
 * MISO          ->  GPIO7 (Pin 10)
 * MOSI          ->  GPIO0 (Pin 1)
 * SCK           ->  GPIO6 (Pin 9)
 * CS            ->  GPIO1 (Pin 2)
 * 
 * This program includes the following tests:
 * 1. Basic Function Tests
 *    - File system information
 *    - Basic file operations
 *    - Directory operations
 * 2. Advanced Function Tests
 *    - File search
 *    - Batch file operations
 *    - Large file handling
 * 3. Performance Tests
 *    - Sequential read/write speed test
 *    - Random read/write speed test
 *    - Small file batch operation test
 *    - Large file read/write test
 *    - Continuous write stress test
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

// Test Configuration
constexpr size_t CHUNK_SIZE = 32 * 1024;     // 32KB block size
constexpr size_t SMALL_FILE_SIZE = 4 * 1024;  // 4KB
constexpr size_t MEDIUM_FILE_SIZE = 256 * 1024; // 256KB
constexpr size_t LARGE_FILE_SIZE = 512 * 1024;  // 512KB
constexpr int NUM_FILES = 10;                   // Number of test files
constexpr int TEST_DURATION = 30;               // Test duration (seconds)
constexpr size_t MAX_RESULTS = 5;

// Test Data Generator
class TestDataGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::array<uint8_t, CHUNK_SIZE> buffer;

public:
    TestDataGenerator() : gen(rd()), dis(0, 255) {}

    // Generate fixed-size test data block
    const std::array<uint8_t, CHUNK_SIZE>& generate_chunk() {
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            buffer[i] = static_cast<uint8_t>(dis(gen));
        }
        return buffer;
    }
};

// Performance Test Result Structure
struct TestResult {
    double duration_ms;
    double speed_mbps;
    size_t total_bytes;
    std::string operation;

    TestResult() : duration_ms(0), speed_mbps(0), total_bytes(0) {}

    TestResult(double d, double s, size_t t, std::string op)
        : duration_ms(d), speed_mbps(s), total_bytes(t), operation(std::move(op)) {}
};

// Print Test Result
void print_result(const TestResult& result) {
    printf("Test Item: %s\n", result.operation.c_str());
    printf("  Duration: %.2f ms\n", result.duration_ms);
    printf("  Speed: %.2f MB/s\n", result.speed_mbps);
    printf("  Total Bytes: %zu bytes\n\n", result.total_bytes);
}

// Test Results Manager Class
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
        printf("\n=== Test Results Summary ===\n\n");
        for (size_t i = 0; i < count; ++i) {
            print_result(results[i]);
        }
    }
};

// Print File Information
void print_file_info(const FileInfo& info) {
    printf("Filename: %s\n", info.name.c_str());
    printf("Full Path: %s\n", info.full_path.c_str());
    printf("Size: %zu bytes\n", info.size);
    printf("Type: %s\n", info.is_directory ? "Directory" : "File");
    printf("---\n");
}

// Wait for User Confirmation
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

// Basic Function Test
void demonstrate_basic_operations(SDCard& sd_card) {
    printf("\n=== Basic Function Test ===\n");
    
    // Get filesystem information
    printf("Filesystem Type: %s\n", sd_card.get_filesystem_type().c_str());
    
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("Total Capacity: %.2f MB\n", total / 1024.0 / 1024.0);
        printf("Available Capacity: %.2f MB\n", free / 1024.0 / 1024.0);
    }
    
    // List root directory contents
    printf("\n=== Root Directory Contents ===\n");
    auto list_result = sd_card.list_directory("/");
    if (list_result.is_ok()) {
        for (const auto& file : *list_result) {
            print_file_info(file);
        }
    }
    
    // Create test directory
    printf("\n=== Create Test Directory ===\n");
    auto mkdir_result = sd_card.create_directory("/test_dir");
    if (mkdir_result.is_ok()) {
        printf("Successfully created directory '/test_dir'\n");
    }
    
    // Write test file
    printf("\n=== Write Test File ===\n");
    std::string test_content = "Hello, MicroSD!\nThis is a test file.\n";
    auto write_result = sd_card.write_text_file("/test_dir/hello.txt", test_content);
    if (write_result.is_ok()) {
        printf("File written successfully\n");
    }
    
    // Read test file
    printf("\n=== Read Test File ===\n");
    auto read_result = sd_card.read_file("/test_dir/hello.txt");
    if (read_result.is_ok()) {
        std::string content(read_result->begin(), read_result->end());
        printf("File content:\n%s\n", content.c_str());
    }
}

// File Search Function
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

// Advanced Function Test
void demonstrate_advanced_operations(SDCard& sd_card) {
    printf("\n=== Advanced Function Test ===\n");
    
    // Batch file operations
    const std::string base_dir = "/batch_test";
    
    // Delete old directory if exists
    auto old_files = sd_card.list_directory(base_dir);
    if (old_files.is_ok()) {
        for (const auto& file : *old_files) {
            if (!file.is_directory) {
                sd_card.delete_file(file.full_path);
            }
        }
        sd_card.remove_directory(base_dir);
    }
    
    // Create test directory structure
    std::vector<std::string> dirs = {
        "/batch_test",
        "/batch_test/images",
        "/batch_test/documents",
        "/batch_test/audio",
        "/batch_test/others"
    };
    
    printf("\nCreating test directory structure...\n");
    for (const auto& dir : dirs) {
        auto mkdir_result = sd_card.create_directory(dir);
        if (mkdir_result.is_ok()) {
            printf("Directory created: %s\n", dir.c_str());
        }
    }
    
    // Create test files
    printf("\nCreating test files...\n");
    std::vector<std::pair<std::string, std::string>> test_files = {
        {"/batch_test/images/photo1.jpg", "This is the data of a photo"},
        {"/batch_test/images/photo2.jpg", "This is the data of another photo"},
        {"/batch_test/images/image.png", "This is the data of a PNG image"},
        {"/batch_test/documents/readme.txt", "This is a readme file"},
        {"/batch_test/documents/notes.txt", "These are some notes"},
        {"/batch_test/documents/report.pdf", "This is a PDF document"},
        {"/batch_test/audio/song1.mp3", "This is an MP3 song"},
        {"/batch_test/audio/song2.mp3", "This is another MP3 song"},
        {"/batch_test/audio/music.wav", "This is a WAV audio file"},
        {"/batch_test/others/test.txt", "This is a test file"}
    };
    
    for (const auto& [path, content] : test_files) {
        auto result = sd_card.write_text_file(path, content);
        if (result.is_ok()) {
            printf("File created: %s\n", path.c_str());
        }
    }
    
    // Search for files with specific extensions
    printf("\n=== File Search Demonstration ===\n");
    std::vector<std::string> extensions = {".txt", ".jpg", ".mp3"};
    
    for (const auto& ext : extensions) {
        printf("\nSearching for %s files:\n", ext.c_str());
        auto matching_files = search_files_by_extension(sd_card, "/batch_test", ext);
        
        if (matching_files.empty()) {
            printf("No %s files found\n", ext.c_str());
        } else {
            for (const auto& file : matching_files) {
                printf("   Found: %s (Size: %zu bytes)\n", 
                       file.full_path.c_str(), file.size);
            }
        }
    }
    
    printf("\nFile Search Demonstration Completed\n");
}

// Sequential Write Test
TestResult sequential_write_test(SDCard& sd_card, size_t file_size) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t written = 0;
    
    auto file_result = sd_card.open_file("/seq_test.bin", "w");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "Sequential Write Test (Failed)");
    }
    
    auto& file = *file_result;
    while (written < file_size) {
        const auto& chunk = gen.generate_chunk();
        size_t to_write = std::min(CHUNK_SIZE, file_size - written);
        
        auto write_result = file.write(std::vector<uint8_t>(chunk.begin(), chunk.begin() + to_write));
        if (write_result.is_error()) {
            return TestResult(0, 0, 0, "Sequential Write Test (Failed)");
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
        "Sequential Write Test"
    );
}

// Sequential Read Test
TestResult sequential_read_test(SDCard& sd_card, size_t file_size) {
    auto start = high_resolution_clock::now();
    size_t total_read = 0;
    
    auto file_result = sd_card.open_file("/seq_test.bin", "r");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "Sequential Read Test (Failed)");
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
        "Sequential Read Test"
    );
}

// Small File Batch Operation Test
TestResult small_files_test(SDCard& sd_card) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t total_bytes = 0;
    
    auto dir_result = sd_card.create_directory("/test_files");
    if (dir_result.is_error()) {
        return TestResult(0, 0, 0, "Small File Test (Failed)");
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
        "Small File Batch Operation Test"
    );
}

// Stress Test
TestResult stress_test(SDCard& sd_card) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t total_bytes = 0;
    const auto test_duration = seconds(TEST_DURATION);
    printf("Starting stress test, duration: %d seconds\n", TEST_DURATION);
    
    auto test_start = high_resolution_clock::now();
    int last_seconds = TEST_DURATION;
    
    while (duration_cast<seconds>(high_resolution_clock::now() - test_start) < test_duration) {
        int remaining = TEST_DURATION - duration_cast<seconds>(high_resolution_clock::now() - test_start).count();
        if (remaining < last_seconds) {
            printf("\rRemaining time: %d seconds ", remaining);
            fflush(stdout);
            last_seconds = remaining;
        }
        
        const auto& chunk = gen.generate_chunk();
        std::string filename = "/stress_test_" + std::to_string(total_bytes) + ".bin";
        
        auto file_result = sd_card.open_file(filename, "w");
        if (file_result.is_error()) {
            printf("\nFailed to create stress test file\n");
            return TestResult(0, 0, 0, "Stress Test (Failed)");
        }
        
        auto& file = *file_result;
        auto write_result = file.write(std::vector<uint8_t>(chunk.begin(), chunk.begin() + SMALL_FILE_SIZE));
        if (write_result.is_error()) {
            printf("\nFailed to write to stress test file\n");
            file.close();
            return TestResult(0, 0, 0, "Stress Test (Failed)");
        }
        
        file.close();
        total_bytes += SMALL_FILE_SIZE;
        
        auto read_file_result = sd_card.open_file(filename, "r");
        if (read_file_result.is_error()) {
            printf("\nFailed to open stress test file\n");
            return TestResult(0, 0, 0, "Stress Test (Failed)");
        }
        
        auto& read_file = *read_file_result;
        auto read_result = read_file.read(SMALL_FILE_SIZE);
        if (read_result.is_error()) {
            printf("\nFailed to read stress test file\n");
            read_file.close();
            return TestResult(0, 0, 0, "Stress Test (Failed)");
        }
        
        read_file.close();
        
        auto delete_result = sd_card.delete_file(filename);
        if (delete_result.is_error()) {
            printf("\nFailed to delete stress test file\n");
            return TestResult(0, 0, 0, "Stress Test (Failed)");
        }
        
        sleep_ms(100);
    }
    
    printf("\nStress Test Completed!\n");
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double speed = (total_bytes * 2 / 1024.0 / 1024.0) / (duration.count() / 1000.0);
    
    return TestResult(
        static_cast<double>(duration.count()),
        speed,
        total_bytes * 2,
        "Stress Test"
    );
}

// Large File Test
TestResult large_file_test(SDCard& sd_card) {
    TestDataGenerator gen;
    auto start = high_resolution_clock::now();
    size_t total_bytes = 0;
    
    auto file_result = sd_card.open_file("/large_test.bin", "w");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "Large File Test (Failed)");
    }
    
    auto& file = *file_result;
    size_t remaining = LARGE_FILE_SIZE;
    
    while (remaining > 0) {
        const auto& chunk = gen.generate_chunk();
        size_t to_write = std::min(CHUNK_SIZE, remaining);
        
        auto write_result = file.write(std::vector<uint8_t>(chunk.begin(), chunk.begin() + to_write));
        if (write_result.is_error()) {
            file.close();
            return TestResult(0, 0, 0, "Large File Test (Failed)");
        }
        
        total_bytes += to_write;
        remaining -= to_write;
    }
    
    file.close();
    
    file_result = sd_card.open_file("/large_test.bin", "r");
    if (file_result.is_error()) {
        return TestResult(0, 0, 0, "Large File Test (Failed)");
    }
    
    auto& read_file = *file_result;
    remaining = LARGE_FILE_SIZE;
    
    while (remaining > 0) {
        size_t to_read = std::min(CHUNK_SIZE, remaining);
        auto read_result = read_file.read(to_read);
        
        if (read_result.is_error() || read_result->empty()) {
            read_file.close();
            return TestResult(0, 0, 0, "Large File Test (Failed)");
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
        "Large File Read/Write Test"
    );
}

int main() {
    stdio_init_all();
    sleep_ms(3000); // Wait for serial connection
    
    printf("\n=== Comprehensive MicroSD Card Test Program ===\n\n");
    printf("Compilation Time: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n\n");
    
    // Initialize SD Card
    SDCard sd_card;
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("SD Card Initialization Failed\n");
        return 1;
    }
    
    // Get Capacity Information
    auto capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("SD Card Total Capacity: %.2f GB\n", total / (1024.0 * 1024.0 * 1024.0));
        printf("Available Space: %.2f GB\n", free / (1024.0 * 1024.0 * 1024.0));
        printf("Filesystem Type: %s\n\n", sd_card.get_filesystem_type().c_str());
    }
    
    // Prompt Formatting
    printf("Warning: Performance Test Requires Formatting SD Card!\n");
    printf("Formatting Will Clear All Data on the SD Card!\n");
    
    if (!wait_for_user_confirmation("Continue? This Will Delete All Data on the SD Card")) {
        printf("Test Cancelled\n");
        return 0;
    }
    
    printf("Formatting SD Card...\n");
    auto format_result = sd_card.format("FAT32");
    if (format_result.is_error()) {
        printf("Formatting Failed: %s\n", sd_card.get_error_description(format_result.error_code()).c_str());
        return 1;
    }
    printf("Formatting Completed!\n\n");
    
    // Re-mount File System
    init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("Failed to Re-mount File System\n");
        return 1;
    }
    
    // Display Formatted Capacity Information
    capacity_result = sd_card.get_capacity();
    if (capacity_result.is_ok()) {
        auto [total, free] = *capacity_result;
        printf("Formatted Capacity Information:\n");
        printf("Total Capacity: %.2f GB\n", total / (1024.0 * 1024.0 * 1024.0));
        printf("Available Space: %.2f GB\n", free / (1024.0 * 1024.0 * 1024.0));
        printf("Filesystem Type: %s\n\n", sd_card.get_filesystem_type().c_str());
    }
    
    if (!wait_for_user_confirmation("Start Test?")) {
        printf("Test Cancelled\n");
        return 0;
    }
    
    // Run Basic Function Test
    demonstrate_basic_operations(sd_card);
    
    // Run Advanced Function Test
    demonstrate_advanced_operations(sd_card);
    
    // Run Performance Test
    TestResults results;
    
    printf("\n=== Starting Performance Test ===\n");
    
    printf("\nRunning Sequential Write Test...\n");
    results.add(sequential_write_test(sd_card, LARGE_FILE_SIZE));
    
    printf("\nRunning Sequential Read Test...\n");
    results.add(sequential_read_test(sd_card, LARGE_FILE_SIZE));
    
    printf("\nRunning Small File Test...\n");
    results.add(small_files_test(sd_card));
    
    printf("\nRunning Stress Test...\n");
    results.add(stress_test(sd_card));
    
    printf("\nRunning Large File Test...\n");
    results.add(large_file_test(sd_card));
    
    // Print Test Results Summary
    results.print_summary();
    
    printf("\n=== All Tests Completed!===\n");
    printf("Test Results Saved, Program Will Continue to Run to Maintain Serial Connection.\n");
    
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
} 