cmake_minimum_required(VERSION 3.13)

# 设置项目名称和语言
set(PROJECT_NAME "MicroSD-Pico")
project(${PROJECT_NAME} C CXX ASM)

# 设置C++标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 导入Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 如果有Pico Extras，也导入它
if(DEFINED ENV{PICO_EXTRAS_PATH})
    include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
endif()

# 初始化Pico SDK
pico_sdk_init()

# 添加编译器选项
add_compile_options(-Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # 允许未使用的函数
    -Wno-maybe-uninitialized
)

# 添加pico_fatfs库
add_subdirectory(lib/pico_fatfs)

# 创建MicroSD库
add_library(micro_sd
    src/micro_sd.cpp
)

target_include_directories(micro_sd PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(micro_sd
    pico_stdlib
    hardware_spi
    hardware_gpio
    pico_fatfs      # 添加pico_fatfs库
)

# 如果没有pico_fatfs库，我们需要包含FatFs
# 这里假设您会添加FatFs作为外部依赖
target_compile_definitions(micro_sd PRIVATE
    -DFF_USE_LFN=1
    -DFF_LFN_UNICODE=0
)

# 基本使用示例
add_executable(basic_example
    examples/basic_usage.cpp
)

target_include_directories(basic_example PRIVATE
    include
)

target_link_libraries(basic_example
    micro_sd
    pico_stdlib
    pico_stdio_usb      # USB串口输出
    pico_stdio_uart     # UART串口输出 (可选)
)

# 启用USB和UART输出
pico_enable_stdio_usb(basic_example 1)   # 启用USB串口
pico_enable_stdio_uart(basic_example 0)  # 禁用UART串口 (避免冲突)

# 生成UF2文件
pico_add_extra_outputs(basic_example)

# 高级使用示例
add_executable(advanced_example
    examples/advanced_usage.cpp
)

target_include_directories(advanced_example PRIVATE
    include
)

target_link_libraries(advanced_example
    micro_sd
    pico_stdlib
    pico_stdio_usb      # USB串口输出
    pico_stdio_uart     # UART串口输出 (可选)
)

# 启用USB和UART输出
pico_enable_stdio_usb(advanced_example 1)   # 启用USB串口
pico_enable_stdio_uart(advanced_example 0)  # 禁用UART串口

# 生成UF2文件
pico_add_extra_outputs(advanced_example)

# 添加编译定义
target_compile_definitions(basic_example PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
    PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1  # 支持通过波特率重置
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000  # USB连接等待时间
)

target_compile_definitions(advanced_example PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
    PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000
)

# 添加调试定义
target_compile_definitions(micro_sd PRIVATE
    MICRO_SD_DEBUG=1  # 启用调试输出
)

# 如果需要调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(micro_sd PRIVATE DEBUG=1)
    target_compile_definitions(basic_example PRIVATE DEBUG=1)
    target_compile_definitions(advanced_example PRIVATE DEBUG=1)
    
    target_compile_options(micro_sd PRIVATE -g -O0)
    target_compile_options(basic_example PRIVATE -g -O0)
    target_compile_options(advanced_example PRIVATE -g -O0)
else()
    target_compile_options(micro_sd PRIVATE -O2)
    target_compile_options(basic_example PRIVATE -O2)
    target_compile_options(advanced_example PRIVATE -O2)
endif()

# 创建一个简化的调试示例 (只测试SD卡基本功能)
add_executable(debug_example
    examples/debug_simple.cpp
)

target_include_directories(debug_example PRIVATE
    include
)

target_link_libraries(debug_example
    micro_sd
    pico_stdlib
    pico_stdio_usb
)

pico_enable_stdio_usb(debug_example 1)
pico_enable_stdio_uart(debug_example 0)
pico_add_extra_outputs(debug_example)

target_compile_definitions(debug_example PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
    PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000
    DEBUG=1
)

# 显示构建信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# 添加串口测试程序
add_executable(serial_test
    examples/serial_test.cpp
)

target_link_libraries(serial_test
    pico_stdlib
    pico_stdio_usb
    hardware_gpio
)

pico_enable_stdio_usb(serial_test 1)
pico_enable_stdio_uart(serial_test 0)
pico_add_extra_outputs(serial_test)

target_compile_definitions(serial_test PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
    PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=5000  # 更长的等待时间
)

# 显示目标信息
message(STATUS "Targets to build:")
message(STATUS "  - serial_test.uf2 (串口测试程序)")
message(STATUS "  - basic_example.uf2 (基本功能示例)")
message(STATUS "  - advanced_example.uf2 (高级功能示例)")
message(STATUS "  - debug_example.uf2 (调试示例)")

# 添加自定义目标用于清理
add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMENT "清理所有构建文件"
)

# 添加自定义目标显示连接信息
add_custom_target(show-pinout
    COMMAND ${CMAKE_COMMAND} -E echo "=================================="
    COMMAND ${CMAKE_COMMAND} -E echo "MicroSD模块连接:"
    COMMAND ${CMAKE_COMMAND} -E echo "  VCC  -> 3.3V (Pin 36)"
    COMMAND ${CMAKE_COMMAND} -E echo "  GND  -> GND  (Pin 38)"
    COMMAND ${CMAKE_COMMAND} -E echo "  MISO -> GPIO11 (Pin 15)"
    COMMAND ${CMAKE_COMMAND} -E echo "  MOSI -> GPIO12 (Pin 16)"
    COMMAND ${CMAKE_COMMAND} -E echo "  SCK  -> GPIO10 (Pin 14)"
    COMMAND ${CMAKE_COMMAND} -E echo "  CS   -> GPIO13 (Pin 17)"
    COMMAND ${CMAKE_COMMAND} -E echo "=================================="
    COMMENT "显示硬件连接信息"
)

# 在构建后显示信息
add_custom_command(TARGET basic_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "UF2文件已生成: basic_example.uf2"
    COMMAND ${CMAKE_COMMAND} -E echo "调试: 连接USB后打开串口监视器 (115200波特率)"
    VERBATIM
) 