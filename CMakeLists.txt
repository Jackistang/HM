cmake_minimum_required(VERSION 3.12)

project(hci_middleware)

set(CMAKE_BUILD_TYPE "Debug")

set(SOURCES
    src/hci_transport_uart_linux.c
    
    tests/test.c)

add_executable(hci_middleware
    ${SOURCES})

target_include_directories(hci_middleware
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(hci_middleware
    PRIVATE
    cunit)
