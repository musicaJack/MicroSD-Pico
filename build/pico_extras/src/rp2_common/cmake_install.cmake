# Install script for directory: C:/Users/sh_mu/Documents/Pico-v1.5.1/pico-extras/src/rp2_common

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/MicroSD-Pico")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Program Files/Raspberry Pi/Pico SDK v1.5.1/gcc-arm-none-eabi/bin/arm-none-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/hardware_rosc/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/pico_sleep/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/pico_audio_i2s/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/pico_audio_pwm/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/pico_audio_spdif/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/pico_sd_card/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/pico_scanvideo_dpi/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/usb_common/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/usb_device/cmake_install.cmake")
  include("C:/Users/sh_mu/code/MicroSD-Pico/build/pico_extras/src/rp2_common/usb_device_msc/cmake_install.cmake")

endif()

