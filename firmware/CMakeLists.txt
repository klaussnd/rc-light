cmake_minimum_required(VERSION 3.11)

set(MCU_FREQ 1000000UL)
set(AVR_MCU atmega168p)
set(CMAKE_TOOLCHAIN_FILE "../hal/cmake-avr/generic-gcc-avr.cmake")
add_compile_definitions(
   F_CPU=${MCU_FREQ}
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(rc_light_firmware)

set(HAL_DIR ../hal)

add_avr_executable(rc_light
    rc_light.cpp
    decode_servo.cpp
)
avr_target_include_directories(rc_light
    PUBLIC
        ${HAL_DIR}/include
)
if(DEBUG)
    get_target_property(TARGET_LIST rc_light OUTPUT_NAME)
    target_sources(${TARGET_LIST}
        PRIVATE
            ${HAL_DIR}/src/hal/avr/usart_irq.cpp
            ${HAL_DIR}/src/hal/avr/usart_stdout.c
    )
    set_property(
        SOURCE rc_light.cpp
        APPEND
        PROPERTY COMPILE_DEFINITIONS DEBUG
)
endif()
