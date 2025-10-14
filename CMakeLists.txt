cmake_minimum_required(VERSION 3.10)
project(gm6020_ctl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_executable(gm6020_ctl ${WORKING_DIRECTORY}/cli_mod/Src/main.cpp)

target_include_directories(gm6020_ctl PRIVATE 
            ${WORKING_DIRECTORY}/can_mod/Inc
            ${WORKING_DIRECTORY}/cli_mod/Inc
            ${WORKING_DIRECTORY}/drive_mod/Inc
)

add_executable(can_test ${WORKING_DIRECTORY}/unit_test/can_send.cpp)

target_include_directories(can_test PRIVATE 
            ${WORKING_DIRECTORY}/can_mod/Inc 
            ${WORKING_DIRECTORY}/cli_mod/Inc
            ${WORKING_DIRECTORY}/drive_mod/Inc
)