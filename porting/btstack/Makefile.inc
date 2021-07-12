
VPATH += HCI-Middleware/src
VPATH += HCI-Middleware/chipset/zephyr
VPATH += HCI-Middleware/porting/btstack
VPATH += HCI-Middleware/porting/os/linux/src

CORE += \
    hci_transport_h4.c   \
    receiver.c   \
    btstack.c    \
    chipset.c \
    chipset_zephyr.c \
    os_mutex.c  \
    os_sem.c    \
    os_task.c   \
    os_uart.c   \

CFLAGS  +=  \
    -IHCI-Middleware/include    \
    -IHCI-Middleware/porting/os/include \
    -IHCI-Middleware/porting/os/linux/include   \
    -Wno-incompatible-pointer-types \
    -Wimplicit-function-declaration \

LDFLAGS += -lpthread