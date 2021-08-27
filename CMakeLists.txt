cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

# This is set in settings.json and is the name of your folder.
set(projname $ENV{projectName})
set(PICO_DEOPTIMIZED_DEBUG 1)
# I added this for interfacing with my libraries... 
add_compile_definitions(RASPBERRY_PI_PICO)

project(${projname} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
# For exceptions
set(PICO_CXX_ENABLE_EXCEPTIONS 0)

set(LIB_PATH "C:/pico/my-pico/Mylibs")
#variable_watch(LIB_PATH)

#include_directories(C/pico/my-pico/Mylibs/OLED C/pico/my-pico/Mylibs/I2C)

add_executable(${projname})

pico_sdk_init()

target_include_directories 	(	${projname} PUBLIC 
								${LIB_PATH}
							)

target_sources( ${projname} PRIVATE 
					main.cpp
					menu.cpp
					gpio.cpp
					${LIB_PATH}/OLED/OneBitDisplay.cpp 
					${LIB_PATH}/OLED/i2c_wrapper.cpp
					${LIB_PATH}/OLED/SPI_wrapper.cpp
				)
#				${LIB_PATH}/i2c/mi2c.cpp
#				${LIB_PATH}/PID/PID_v1.cpp
#				${LIB_PATH}/flash/flashwrapper.cpp

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(	${projname} PRIVATE 
						pico_stdlib
						hardware_pwm
						hardware_i2c
						hardware_spi
						hardware_irq
					)
# 						pico_multicore
#						pico_malloc
#						pico_mem_ops
#						hardware_adc
#						hardware_watchdog
#						hardware_flash
#						hardware_sync


pico_enable_stdio_usb(${projname} 0)
pico_enable_stdio_uart(${projname} 1)
#include(example_auto_set_url.cmake)

# create map/bin/hex file etc.
pico_add_extra_outputs(${projname})

# add url via pico_set_program_url
#example_auto_set_url(${projname})
