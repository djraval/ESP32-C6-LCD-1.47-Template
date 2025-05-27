# Waveshare ESP32-C6-LCD-1.47 Project
# CMakeLists.txt for the main project
cmake_minimum_required(VERSION 3.16)

# Add components directory to the component search path
set(EXTRA_COMPONENT_DIRS "components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32c6_lcd_board)
