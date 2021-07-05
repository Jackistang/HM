#include "os_port.h"
#include "h4_inner.h"
#include "hci_transport_h4.h"
#include <stdio.h>

static struct os_task   recv_task;
static os_stack_t recv_stack[1024] __attribute__ ((aligned (4))) ;


static void _receiver_task(void)
{
    uint8_t buf[30];
    int size;
    int err;
    while (1) {
        size = os_uart_recv(buf, ARRAY_SIZE(buf));
        if (size <= 0)
            continue ;
        
        if ((err = _hci_transport_h4_pack(buf, size))) {
            printf("Pack h4 package fail: err(%d)\n", err);
        }
        // os_sleep(1);
    }
}

int _receiver_init(void)
{
    int err;
    err = os_task_create(&recv_task, "recv", _receiver_task, NULL, 10, 0, recv_stack, ARRAY_SIZE(recv_stack));
    if (err)
        return -1;
    
    return 0;
}

