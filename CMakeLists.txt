cmake_minimum_required(VERSION 3.10)
project(gm6020_ctl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(gm6020_ctl ${WORKING_DIRECTORY}/core/Src/main.cpp)

target_include_directories(gm6020_ctl PRIVATE ${WORKING_DIRECTORY}/core/Inc)