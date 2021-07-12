cmake_minimum_required(VERSION 3.12)

project(hci_middleware)

set(CMAKE_BUILD_TYPE "Debug")

set(SOURCES
    # src/hci_transport_uart_linux.c
    src/chipset.c
    src/hci_transport_h4.c
    src/receiver.c

# porting os
    porting/os/linux/src/os_mutex.c
    porting/os/linux/src/os_sem.c
    porting/os/linux/src/os_task.c
    porting/os/linux/src/os_uart.c

# 3rd-party
    3rd-party/ringbuffer/ringbuffer.c
    3rd-party/mpool/mpool.c

# chipset
    chipset/zephyr/chipset_zephyr.c

# test
    tests/test.c
    tests/test_hci_transport_h4.c)

add_executable(hci_middleware
    ${SOURCES})

target_include_directories(hci_middleware
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include

# porting os
    ${PROJECT_SOURCE_DIR}/porting/os/include
    ${PROJECT_SOURCE_DIR}/porting/os/linux/include

# 3rd-party
    ${PROJECT_SOURCE_DIR}/3rd-party/ringbuffer
    ${PROJECT_SOURCE_DIR}/3rd-party/mpool
)

target_link_libraries(hci_middleware
    PRIVATE
    cunit
    pthread)
