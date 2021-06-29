cmake_minimum_required(VERSION 3.12)

project(hci_middleware)

set(CMAKE_BUILD_TYPE "Debug")

set(SOURCES
    # src/hci_transport_uart_linux.c

# porting os
    porting/os/linux/src/os_mutex.c
    porting/os/linux/src/os_sem.c
    porting/os/linux/src/os_task.c
    porting/os/linux/src/os_uart.c

    tests/test.c)

add_executable(hci_middleware
    ${SOURCES})

target_include_directories(hci_middleware
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src

# porting os
    ${PROJECT_SOURCE_DIR}/porting/os/include
    ${PROJECT_SOURCE_DIR}/porting/os/linux/include)

target_link_libraries(hci_middleware
    PRIVATE
    cunit
    pthread)
