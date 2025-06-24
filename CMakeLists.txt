cmake_minimum_required(VERSION 3.13)
# Pull in Raspberry Pi Pico SDK (must be defined before project)
# Adjust the path if your SDK is installed elsewhere
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Force the PICO_BOARD to pico_w BEFORE pico_sdk_init()
set(PICO_BOARD pico_w CACHE STRING "Target board" FORCE)

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
    src/storage_device.cpp
    src/rw_sd.cpp
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

# 添加调试定义
target_compile_definitions(micro_sd PRIVATE
    MICRO_SD_DEBUG=1  # 启用调试输出
)

# 如果需要调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(micro_sd PRIVATE DEBUG=1)
    target_compile_options(micro_sd PRIVATE -g -O0)
else()
    target_compile_options(micro_sd PRIVATE -O2)
endif()

# 添加可读写SD卡示例
add_executable(rwsd_demo
    examples/rwsd_demo.cpp
)
target_include_directories(rwsd_demo PRIVATE
    include
)
target_link_libraries(rwsd_demo
    micro_sd
    pico_stdlib
    pico_stdio_usb
    hardware_spi
    hardware_pio
    pico_fatfs
)
pico_enable_stdio_usb(rwsd_demo 1)
pico_enable_stdio_uart(rwsd_demo 0)
pico_add_extra_outputs(rwsd_demo)

target_compile_definitions(rwsd_demo PRIVATE
    PICO_DEFAULT_UART_BAUD_RATE=115200
    PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000
)

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

