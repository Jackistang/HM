
VPATH += HCI_Middleware/src
VPATH += HCI_Middleware/porting/btstack
VPATH += HCI_Middleware/porting/os/linux

CORE += \
    hci_transport_h4.c   \
    receiver.c   \
    btstack.c    \
    os_mutex.c  \
    os_sem.c    \
    os_task.c   \
    os_uart.c   \

CFLAGS  +=  \
    -IHCI_Middleware/include    \
    -IHCI_Middleware/porting/os/include \
    -IHCI_Middleware/porting/os/linux/include   \
    -lpthread

