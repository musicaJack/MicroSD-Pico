# MicroSD-Pico

A modern C++17 MicroSD card library for Raspberry Pi Pico, featuring type-safe error handling, RAII resource management, and comprehensive file operations.

## ✨ Features

- **Modern C++17** - Uses RAII, smart pointers, move semantics, and structured bindings
- **Type-Safe Error Handling** - Custom Result<T> template with comprehensive error codes
- **Complete File Operations** - Read, write, create, delete, directory management
- **Stream Operations** - Line-by-line file processing for large files
- **Batch Operations** - Efficient multiple file handling
- **File System Monitoring** - Capacity tracking and integrity checking
- **USB Serial Debugging** - Real-time status output at 115200 baud
- **User-Friendly Startup** - Visual countdown timers for better UX

## 🔧 Hardware Requirements

### Components
- Raspberry Pi Pico (or Pico W)
- MicroSD card module (SPI interface)
- MicroSD card (FAT32 formatted recommended)
- Jumper wires

### Wiring Diagram
```
MicroSD Module    Raspberry Pi Pico
--------------    -----------------
VCC            -> 3.3V (Pin 36)
GND            -> GND  (Pin 38)  
MISO           -> GPIO11 (Pin 15)
MOSI           -> GPIO12 (Pin 16)
SCK            -> GPIO10 (Pin 14)
CS             -> GPIO13 (Pin 17)
```

## 🚀 Quick Start

### Prerequisites
- **Pico SDK v1.5.1** with ARM GNU Toolchain
- **CMake 3.13+**
- **Ninja Build System**
- **Git** for cloning

### Installation

1. **Clone the repository:**
```bash
git clone https://github.com/your-repo/MicroSD-Pico.git
cd MicroSD-Pico
```

2. **Build the project:**
```bash
# Windows
.\build_pico.bat

# The build script will:
# - Configure environment variables
# - Set up ARM toolchain paths
# - Use CMake + Ninja for compilation
# - Generate UF2 files automatically
```

3. **Flash to Pico:**
- Hold the BOOTSEL button on your Pico
- Connect via USB (RPI-RP2 drive should appear)
- Drag any `.uf2` file from the `build` directory
- Release BOOTSEL and the program will start

## 📁 Generated Programs

After building, you'll find these UF2 files in the `build` directory:

### 🧪 `serial_test.uf2`
**Purpose:** Test serial communication without SD card dependency
```
Features:
- Countdown timer display
- Heartbeat messages every 5 seconds  
- LED blinking indicator
- System status information
```

### 📚 `basic_example.uf2`
**Purpose:** Demonstrate core MicroSD functionality
```
Features:
- SD card initialization and detection
- File system information display
- Basic file read/write operations
- Directory creation and listing
- Error handling demonstrations
```

### 🔧 `advanced_example.uf2`
**Purpose:** Showcase advanced features
```
Features:
- Batch file operations
- Large file handling with chunked I/O
- File system monitoring
- Integrity checking with checksums
- Recursive directory operations
```

### 🐛 `debug_example.uf2`
**Purpose:** Simple debugging and troubleshooting
```
Features:
- Step-by-step operation logging
- Detailed error messages
- Hardware connection verification
- Real-time status updates
```

## 💻 Usage Example

```cpp
#include "micro_sd.hpp"
using namespace MicroSD;

int main() {
    stdio_init_all();
    
    // Create SD card instance with default pins
    SPIConfig config; // Uses GPIO10-13 by default
    SDCard sd_card(config);
    
    // Initialize SD card
    auto init_result = sd_card.initialize();
    if (init_result.is_error()) {
        printf("Failed to initialize SD card\n");
        return -1;
    }
    
    // Write a file
    std::string content = "Hello, MicroSD!";
    auto write_result = sd_card.write_text_file("/hello.txt", content);
    if (write_result.is_ok()) {
        printf("File written successfully\n");
    }
    
    // Read the file back
    auto read_result = sd_card.read_file("/hello.txt");
    if (read_result.is_ok()) {
        std::string file_content(read_result->begin(), read_result->end());
        printf("File content: %s\n", file_content.c_str());
    }
    
    return 0;
}
```

## 🔍 Debugging

### Serial Monitor Setup
1. **Connect USB** after flashing your program
2. **Open serial monitor** (PuTTY, Arduino IDE, VS Code, etc.)
3. **Set baud rate** to `115200`
4. **Watch for countdown** - programs display 3-5 second startup timer

### Common Issues

| Issue | Solution |
|-------|----------|
| No serial output | Check USB connection and baud rate (115200) |
| SD card not detected | Verify wiring and ensure FAT32 format |
| Build fails | Install complete Pico SDK with ARM toolchain |
| Permission denied | Run as administrator or check file permissions |

## 📖 API Reference

### Core Classes

#### `SDCard`
Main interface for SD card operations
```cpp
// Constructor
SDCard(const SPIConfig& config = SPIConfig{});

// Core operations
Result<void> initialize();
Result<std::vector<uint8_t>> read_file(const std::string& path);
Result<void> write_text_file(const std::string& path, const std::string& content, bool append = false);
Result<std::vector<FileInfo>> list_directory(const std::string& path);
Result<void> create_directory(const std::string& path);
Result<void> delete_file(const std::string& path);
```

#### `Result<T>`
Type-safe error handling
```cpp
// Check result
if (result.is_ok()) {
    auto value = *result; // Get value
} else {
    auto error = result.error_code();
    auto message = result.error_message();
}
```

### Configuration

#### `SPIConfig`
Hardware pin configuration
```cpp
struct SPIConfig {
    uint8_t pin_sck = 10;   // Clock
    uint8_t pin_mosi = 12;  // Master Out Slave In  
    uint8_t pin_miso = 11;  // Master In Slave Out
    uint8_t pin_cs = 13;    // Chip Select
    uint32_t baudrate = 12500000; // 12.5 MHz
};
```

## 🛠️ Build System

The project uses a modern CMake + Ninja build system:

### Environment Setup
- **PICO_SDK_PATH**: Points to Pico SDK installation
- **ARM_TOOLCHAIN_PATH**: ARM GNU toolchain binaries
- **CMAKE_PATH**: CMake executable location
- **NINJA_PATH**: Ninja build system

### Build Configuration
- **C++ Standard**: C++17
- **Build Type**: Release (optimized)
- **Target Architecture**: ARM Cortex-M0+
- **Serial Output**: USB CDC (115200 baud)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature-name`)
3. Commit your changes (`git commit -am 'Add feature'`)
4. Push to the branch (`git push origin feature-name`)
5. Create a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Raspberry Pi Foundation** for the excellent Pico SDK
- **pico_fatfs** library for FAT file system support
- **FatFs** by ChaN for the underlying file system implementation

## 📞 Support

- **Issues**: Report bugs and request features via GitHub Issues
- **Documentation**: Comprehensive examples included in `/examples` directory
- **Hardware**: Verify connections match the wiring diagram above

---

*Built with ❤️ for the Raspberry Pi Pico community* 