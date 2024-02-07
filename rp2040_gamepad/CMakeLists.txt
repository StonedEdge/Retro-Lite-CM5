cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(rp2040_gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

add_executable(rp2040_gamepad
        src/main.cpp
        src/keyboard.cpp
        src/mouse.cpp
        src/consumer.cpp
        src/gamepad.cpp
        src/usb_descriptors.cpp
        src/imu/I2Cdev.cpp
        src/imu/MPU6050.cpp
        src/imu/MPU6050_6Axis_MotionApps612.cpp
        src/imu/calibration.cpp
        src/tracking/calibration_data.cc
        src/tracking/main_loop.cc
        src/tracking/orientation_tracker.cc
        src/tracking/sensors/gyroscope_bias_estimator.cc
        src/tracking/sensors/lowpass_filter.cc
        src/tracking/sensors/mean_filter.cc
        src/tracking/sensors/median_filter.cc
        src/tracking/sensors/pose_prediction.cc
        src/tracking/sensors/sensor_fusion_ekf.cc
        src/tracking/util/matrixutils.cc
        src/tracking/util/matrix_3x3.cc
        src/tracking/util/matrix_4x4.cc
        src/tracking/util/rotation.cc
        src/tracking/util/vectorutils.cc
        )

target_compile_definitions(rp2040_gamepad PRIVATE
        PICO_DEFAULT_UART=1
        PICO_DEFAULT_UART_TX_PIN=4
        PICO_DEFAULT_UART_RX_PIN=5
        PICO_DEFAULT_I2C=1
        PICO_DEFAULT_I2C_SDA_PIN=18
        PICO_DEFAULT_I2C_SCL_PIN=19
        PICO_FLASH_SIZE_BYTES=8388608
)

target_include_directories(rp2040_gamepad PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# pull in common dependencies and additional i2c hardware support
target_link_libraries(rp2040_gamepad pico_stdlib pico_unique_id hardware_i2c hardware_adc hardware_flash tinyusb_device tinyusb_board)

# create map/bin/hex file etc.
pico_add_extra_outputs(rp2040_gamepad)
