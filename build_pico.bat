@echo off
echo Starting build process...

rem ========== Configure Pico SDK Environment Variables ==========
set PICO_SDK_VERSION=1.5.1
set "PICO_INSTALL_PATH=C:\Program Files\Raspberry Pi\Pico SDK v1.5.1"
set PICO_REG_KEY=Software\Raspberry Pi\Pico SDK v1.5.1
set "PICO_repos_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1"
set "PICO_examples_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1\pico-examples"
set "PICO_extras_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1\pico-extras"
set "PICO_playground_PATH=C:\Users\sh_mu\Documents\Pico-v1.5.1\pico-playground"
set "OPENOCD_SCRIPTS=%PICO_INSTALL_PATH%\openocd\scripts"

rem Set core SDK path
set "PICO_SDK_PATH=%PICO_INSTALL_PATH%\pico-sdk"
echo Setting PICO_SDK_PATH to %PICO_SDK_PATH%

rem ========== Configure ARM Toolchain ==========
set "ARM_TOOLCHAIN_PATH=%PICO_INSTALL_PATH%\gcc-arm-none-eabi\bin"
set "CMAKE_PATH=%PICO_INSTALL_PATH%\cmake\bin"
set "NINJA_PATH=%PICO_INSTALL_PATH%\ninja"
set "PYTHON_PATH=%PICO_INSTALL_PATH%\python"
set "GIT_PATH=%PICO_INSTALL_PATH%\git\bin"

rem Add toolchain to PATH
set "PATH=%ARM_TOOLCHAIN_PATH%;%CMAKE_PATH%;%NINJA_PATH%;%PYTHON_PATH%;%GIT_PATH%;%PATH%"

echo Configuring ARM toolchain path: %ARM_TOOLCHAIN_PATH%
echo Configuring Ninja path: %NINJA_PATH%
echo Configuring CMake path: %CMAKE_PATH%

rem Verify toolchain availability
echo Verifying toolchain...
arm-none-eabi-gcc --version >nul 2>&1
if %ERRORLEVEL%==0 (
    echo ✓ ARM toolchain available
) else (
    echo ✗ ARM toolchain not available
    goto :show_solutions
)

ninja --version >nul 2>&1
if %ERRORLEVEL%==0 (
    echo ✓ Ninja build system available
) else (
    echo ✗ Ninja build system not available
    goto :show_solutions
)

cmake --version >nul 2>&1
if %ERRORLEVEL%==0 (
    echo ✓ CMake available
) else (
    echo ✗ CMake not available
    goto :show_solutions
)

rem Copy pico_sdk_import.cmake file if exists
if exist "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" (
    echo Copying pico_sdk_import.cmake from SDK external directory...
    copy "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" .
) else if exist "%PICO_SDK_PATH%\pico_sdk_import.cmake" (
    echo Copying pico_sdk_import.cmake from SDK root directory...
    copy "%PICO_SDK_PATH%\pico_sdk_import.cmake" .
)

if exist build (
    echo Cleaning old build files...
    rd /s /q build
)

mkdir build
cd build

rem ========== Start Build Process ==========
echo Configuring project with cmake...
echo Using ARM toolchain: %ARM_TOOLCHAIN_PATH%

rem Use Ninja generator with ARM toolchain (use short path names to avoid space issues)
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
    echo CMake configuration failed!
    goto :show_solutions
)

echo Building with ninja...
ninja -j8

if %ERRORLEVEL% NEQ 0 (
    echo Ninja build failed!
    echo Checking detailed error information...
    goto :show_solutions
)

:check_output
echo Checking generated files...

rem Check for UF2 files
set UF2_FOUND=0
if exist "basic_example.uf2" (
    echo ✓ Found: basic_example.uf2
    set UF2_FOUND=1
)
if exist "advanced_example.uf2" (
    echo ✓ Found: advanced_example.uf2
    set UF2_FOUND=1
)
if exist "debug_example.uf2" (
    echo ✓ Found: debug_example.uf2
    set UF2_FOUND=1
)
if exist "serial_test.uf2" (
    echo ✓ Found: serial_test.uf2
    set UF2_FOUND=1
)

if %UF2_FOUND%==1 (
    echo.
    echo ========== BUILD SUCCESSFUL! ==========
    echo UF2 files generated in build directory
    echo.
    echo Usage Instructions:
    echo 1. Hold the BOOTSEL button on your Pico
    echo 2. Connect Pico to computer via USB
    echo 3. Release BOOTSEL button (RPI-RP2 drive should appear)
    echo 4. Drag any .uf2 file to the drive
    echo 5. Pico will restart and run the program
    echo.
    echo Debug Output:
    echo - USB serial output enabled (115200 baud rate)
    echo - Use serial monitor to view debug information
    echo.
    echo Hardware Connections (MicroSD Module):
    echo VCC  - 3.3V  (Pin 36)
    echo GND  - GND   (Pin 38)
    echo MISO - GPIO11 (Pin 15)
    echo MOSI - GPIO12 (Pin 16)
    echo SCK  - GPIO10 (Pin 14)
    echo CS   - GPIO13 (Pin 17)
    goto :success_end
) else (
    echo Build completed but no UF2 files found
    echo This may be due to errors during the build process
    goto :show_solutions
)

:show_solutions
echo.
echo ==== TROUBLESHOOTING ====
echo.
echo 1. Check environment variable configuration:
echo    PICO_SDK_PATH=%PICO_SDK_PATH%
echo    ARM_TOOLCHAIN_PATH=%ARM_TOOLCHAIN_PATH%
echo.
echo 2. Verify toolchain:
echo    arm-none-eabi-gcc --version
echo    ninja --version
echo    cmake --version
echo.
echo 3. Check Pico SDK installation:
echo    Ensure the following directories exist:
echo    - %PICO_SDK_PATH%
echo    - %ARM_TOOLCHAIN_PATH%
echo    - %NINJA_PATH%
echo.
echo 4. If problems persist, check:
echo    - Pico SDK is completely installed
echo    - Environment variables are correctly set
echo    - You have permissions to access installation directory
echo.

:success_end
echo.
echo Output files in build directory:
for %%f in (*.uf2) do echo   %%f
for %%f in (*.elf) do echo   %%f

cd ..

if %UF2_FOUND%==1 (
    echo.
    set /p OPEN_FOLDER=Open build folder to view UF2 files? (y/n): 
    if /i "%OPEN_FOLDER%"=="y" (
        explorer build
    )
)

pause 