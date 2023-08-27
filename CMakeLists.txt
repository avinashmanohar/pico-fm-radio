cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(project_name)

pico_sdk_init()

add_executable(main
    main.c ssd1306.c button.c
)

# Add pico_stdlib library, add more if used
target_link_libraries(main pico_stdlib hardware_i2c)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# Need to generate UF2 file for upload to RP2040
pico_add_extra_outputs(main)