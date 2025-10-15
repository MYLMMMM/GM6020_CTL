cmake_minimum_required(VERSION 3.10)
project(gm6020_ctl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_library(headers INTERFACE)

target_include_directories(headers INTERFACE 
    ${WORKING_DIRECTORY}/can_mod/Inc
    ${WORKING_DIRECTORY}/cli_mod/Inc
    ${WORKING_DIRECTORY}/drive_mod/Inc
    ${WORKING_DIRECTORY}/drive_mod/alth
    ${WORKING_DIRECTORY}/error_struct     
)

add_executable(hearder_test ${WORKING_DIRECTORY}/unit_test/headers_test.cpp)

target_link_libraries(hearder_test headers)

add_executable(gm6020_ctl ${WORKING_DIRECTORY}/cli_mod/Src/main.cpp)

target_link_libraries(gm6020_ctl headers)

add_executable(can_test ${WORKING_DIRECTORY}/unit_test/can_send.cpp)

target_link_libraries(can_test headers)
