#include "chipset.h"
#include "os_port.h"
#include "hci_transport_h4.h"
#include <assert.h>

static uint8_t g_hci_command_buf[260];
static struct os_sem sync_sem;


static void chipset_recv_handler(int packet_type, uint8_t *packet, uint16_t size)
{
    assert(packet_type == HCI_TRANSPORT_H4_EVENT); //  Event

    if (packet[0] == 0x0e)  //  Command complete
        os_sem_release(&sync_sem);
}

int rt_chipset_init_start(void)
{
    int ret = CHIPSET_ITER_CONTINUE;
    uint16_t len = 0;   // HCI command length.
    int err = 0;

    os_sem_init(&sync_sem, 0);

    rt_hci_transport_h4_register_packet_handler(chipset_recv_handler);

    rt_chipset_t *instance = rt_chipset_get_instance();
    assert(instance);

    instance->init(NULL);
    
    while (ret == CHIPSET_ITER_CONTINUE) {
        ret = instance->next_hci_command(g_hci_command_buf);    // 1 byte is for H4 type.   
        len = 3 + g_hci_command_buf[2];     // Opcode(2 bytes) + Parameter length(1 byte) + Payload length

        err = rt_hci_transport_h4_send(HCI_TRANSPORT_H4_COMMAND, g_hci_command_buf, len);
        if (err)
            return -1;

        err = os_sem_take(&sync_sem, 1000);
        if (err)
            return -1;

    }
}