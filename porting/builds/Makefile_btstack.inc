HCI_MIDDLEWARE_ROOT ?= ../..

CORE += \
    $(HCI_MIDDLEWARE_ROOT)/src/hci_transport_h4.c   \
    $(HCI_MIDDLEWARE_ROOT)/src/receiver.c   \
    $(HCI_MIDDLEWARE_ROOT)/porting/os/linux/src/os_mutex.c  \
    $(HCI_MIDDLEWARE_ROOT)/porting/os/linux/src/os_sem.c    \
    $(HCI_MIDDLEWARE_ROOT)/porting/os/linux/src/os_task.c   \
    $(HCI_MIDDLEWARE_ROOT)/porting/os/linux/src/os_uart.c   

CFLAGS  +=  \
    -I$(HCI_MIDDLEWARE_ROOT)/include    \
    -I$(HCI_MIDDLEWARE_ROOT)/porting/os/include \
    -I$(HCI_MIDDLEWARE_ROOT)/porting/os/linux/include   \
    -lpthread

