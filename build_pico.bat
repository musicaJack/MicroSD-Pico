@echo off
echo  开始构建...

rem ========== 配置Pico SDK环境变量 ==========
set PICO_SDK_VERSION=1.5.1
set "PICO_INSTALL_PATH=C:\Program Files\Raspberry Pi\Pico SDK v1.5.1"
set PICO_REG_KEY=Software\Raspberry Pi\Pico SDK v1.5.1
set "PICO_repos_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1"
set "PICO_examples_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1\pico-examples"
set "PICO_extras_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1\pico-extras"
set "PICO_playground_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1\pico-playground"
set "OPENOCD_SCRIPTS=%PICO_INSTALL_PATH%\openocd\scripts"

rem 设置核心SDK路径
set "PICO_SDK_PATH=%PICO_INSTALL_PATH%\pico-sdk"
echo  设置 PICO_SDK_PATH 为 %PICO_SDK_PATH%

rem ========== 配置ARM工具链 ==========
set "ARM_TOOLCHAIN_PATH=%PICO_INSTALL_PATH%\gcc-arm-none-eabi\bin"
set "CMAKE_PATH=%PICO_INSTALL_PATH%\cmake\bin"
set "NINJA_PATH=%PICO_INSTALL_PATH%\ninja"
set "PYTHON_PATH=%PICO_INSTALL_PATH%\python"
set "GIT_PATH=%PICO_INSTALL_PATH%\git\bin"

rem 添加工具链到PATH
set "PATH=%ARM_TOOLCHAIN_PATH%;%CMAKE_PATH%;%NINJA_PATH%;%PYTHON_PATH%;%GIT_PATH%;%PATH%"

echo  配置ARM工具链路径: %ARM_TOOLCHAIN_PATH%
echo  配置Ninja路径: %NINJA_PATH%
echo  配置CMake路径: %CMAKE_PATH%

rem 验证工具链是否可用
echo  验证工具链...
arm-none-eabi-gcc --version >nul 2>&1
if %ERRORLEVEL%==0 (
    echo  ✓ ARM工具链可用
) else (
    echo  ✗ ARM工具链不可用
    goto :show_solutions
)

ninja --version >nul 2>&1
if %ERRORLEVEL%==0 (
    echo  ✓ Ninja构建系统可用
) else (
    echo  ✗ Ninja构建系统不可用
    goto :show_solutions
)

cmake --version >nul 2>&1
if %ERRORLEVEL%==0 (
    echo  ✓ CMake可用
) else (
    echo  ✗ CMake不可用
    goto :show_solutions
)

rem 复制pico_sdk_import.cmake文件(如果存在)
if exist "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" (
    echo  从SDK external目录复制 pico_sdk_import.cmake...
    copy "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" .
) else if exist "%PICO_SDK_PATH%\pico_sdk_import.cmake" (
    echo  从SDK根目录复制 pico_sdk_import.cmake...
    copy "%PICO_SDK_PATH%\pico_sdk_import.cmake" .
)

if exist build (
    echo  清理旧的构建文件...
    rd /s /q build
)

mkdir build
cd build

rem ========== 开始构建 ==========
echo  使用cmake配置项目...
echo  使用ARM工具链: %ARM_TOOLCHAIN_PATH%

rem 使用Ninja生成器和ARM工具链（使用短路径名避免空格问题）
cmake .. ^
    -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_SYSTEM_NAME=Generic ^
    -DCMAKE_SYSTEM_PROCESSOR=arm ^
    -DCMAKE_C_COMPILER:FILEPATH="%ARM_TOOLCHAIN_PATH%\arm-none-eabi-gcc.exe" ^
    -DCMAKE_CXX_COMPILER:FILEPATH="%ARM_TOOLCHAIN_PATH%\arm-none-eabi-g++.exe" ^
    -DCMAKE_ASM_COMPILER:FILEPATH="%ARM_TOOLCHAIN_PATH%\arm-none-eabi-gcc.exe" ^
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY ^
    -DCMAKE_C_FLAGS="-mcpu=cortex-m0plus -mthumb" ^
    -DCMAKE_CXX_FLAGS="-mcpu=cortex-m0plus -mthumb" ^
    -DCMAKE_ASM_FLAGS="-mcpu=cortex-m0plus -mthumb"

if %ERRORLEVEL% NEQ 0 (
    echo  cmake配置失败！
    goto :show_solutions
)

echo  使用ninja构建...
ninja -j8

if %ERRORLEVEL% NEQ 0 (
    echo  ninja构建失败！
    echo  检查详细错误信息...
    goto :show_solutions
)

:check_output
echo  检查生成的文件...

rem 检查UF2文件
set UF2_FOUND=0
if exist "basic_example.uf2" (
    echo  ✓ 找到: basic_example.uf2
    set UF2_FOUND=1
)
if exist "advanced_example.uf2" (
    echo  ✓ 找到: advanced_example.uf2
    set UF2_FOUND=1
)
if exist "debug_example.uf2" (
    echo  ✓ 找到: debug_example.uf2
    set UF2_FOUND=1
)

if %UF2_FOUND%==1 (
    echo.
    echo  ========== 构建成功！ ==========
    echo  UF2文件已生成在build目录中
    echo.
    echo  使用说明：
    echo  1. 按住Pico的BOOTSEL按钮
    echo  2. 通过USB连接Pico到电脑
    echo  3. 释放BOOTSEL按钮（应该出现RPI-RP2磁盘）
    echo  4. 将.uf2文件拖拽到磁盘中
    echo  5. Pico将重启并运行程序
    echo.
    echo  调试输出：
    echo  - 已启用USB串口输出（115200波特率）
    echo  - 使用串口监视器查看调试信息
    echo.
    echo  硬件连接（MicroSD模块）：
    echo  VCC  - 3.3V （Pin 36）
    echo  GND  - GND  （Pin 38）
    echo  MISO - GPIO11（Pin 15）
    echo  MOSI - GPIO12（Pin 16）
    echo  SCK  - GPIO10（Pin 14）
    echo  CS   - GPIO13（Pin 17）
    goto :success_end
) else (
    echo  构建完成但未找到UF2文件
    echo  这可能是因为构建过程中出现了错误
    goto :show_solutions
)

:show_solutions
echo.
echo  ==== 故障排除 ====
echo.
echo  1. 检查环境变量配置：
echo     PICO_SDK_PATH=%PICO_SDK_PATH%
echo     ARM_TOOLCHAIN_PATH=%ARM_TOOLCHAIN_PATH%
echo.
echo  2. 验证工具链：
echo     arm-none-eabi-gcc --version
echo     ninja --version
echo     cmake --version
echo.
echo  3. 检查Pico SDK安装：
echo     确保以下目录存在：
echo     - %PICO_SDK_PATH%
echo     - %ARM_TOOLCHAIN_PATH%
echo     - %NINJA_PATH%
echo.
echo  4. 如果仍有问题，请检查：
echo     - Pico SDK是否完整安装
echo     - 环境变量是否正确设置
echo     - 是否有权限访问安装目录
echo.

:success_end
echo.
echo  构建目录中的输出文件：
for %%f in (*.uf2) do echo   %%f
for %%f in (*.elf) do echo   %%f

cd ..

if %UF2_FOUND%==1 (
    echo.
    set /p OPEN_FOLDER=是否打开build文件夹查看UF2文件？ (y/n): 
    if /i "%OPEN_FOLDER%"=="y" (
        explorer build
    )
)

pause 