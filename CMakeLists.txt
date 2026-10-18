# @file CMakeLists.txt
# @brief ESP-IDF project entry point and minimal firmware component graph.
#
cmake_minimum_required(VERSION 3.16)
set(COMPONENTS main)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tm100_tester VERSION 0.1.0)
