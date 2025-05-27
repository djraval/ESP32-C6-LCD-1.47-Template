# Example project CMakeLists.txt for board demonstration
cmake_minimum_required(VERSION 3.16)

# Add the main project components directory to the component search path
set(EXTRA_COMPONENT_DIRS "../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(board_demo)
