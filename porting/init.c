#include "hm_chipset.h"
#include "hm_config.h"
#include "hm_hci_transport_h4.h"
#include "init.h"

#if HM_CONFIG_NIMBLE
#include "host/ble_hs_id.h"
#endif

#include <rtthread.h>

static struct hci_trans_h4_config h4_config = {
    .uart_config = {
        .device_name = "uart1",             /* Default value */
        .databit     = DATA_BITS_8,
        .stopbit     = STOP_BITS_1,
        .parity      = PARITY_NONE,
        .baudrate    = BAUD_RATE_115200,    /* Default value */
        .flowcontrol = 1,                   /* Default value */
    },
};

static void hm_start_advertisement(uint8_t *data, uint8_t data_len)
{
    uint8_t *cmd = (uint8_t *)hci_trans_h4_send_alloc(HCI_TRANS_H4_TYPE_CMD);
    RT_ASSERT(cmd != NULL);
    uint8_t *evt;

    // LE Set Advertising Data command
    // OCF 0x0008, OGF 0x08, - Opcode 0x2008
    cmd[0] = 0x08;
    cmd[1] = 0x20;          // opcode
    cmd[2] = data_len+1;  // parameters length
    cmd[3] = data_len;
    rt_memcpy(&cmd[4], data, data_len);
    hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, cmd);
    hci_trans_h4_recv_event(&evt, RT_WAITING_FOREVER);
    hci_trans_h4_recv_free(evt);

    // LE Set Advertising Enable command
    // OCF 0x000A, OGF 0x08, - Opcode 0x200A
    cmd[0] = 0x0A;
    cmd[1] = 0x20;          // opcode
    cmd[2] = 1;             // parameters length
    cmd[3] = 1;
    hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, cmd);
    hci_trans_h4_recv_event(&evt, RT_WAITING_FOREVER);
    hci_trans_h4_recv_free(evt);

    hci_trans_h4_send_free(cmd);
}

static void hm_thread_entry(void *args)
{
    hci_trans_h4_init(&h4_config);
    hci_trans_h4_open();

    hm_chipset_t *chipset_instance = hm_chipset_get_instance();
    chipset_instance->init();

#if HM_CONFIG_NIMBLE
    // uint8_t random_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x06};
    // ble_hs_id_set_rnd(random_addr);
    hm_nimble_init();
#endif

#if HM_CONFIG_BTSTACK
    btstack_rtthread_port_init();
#endif

}

static int hm_init(void)
{
    rt_thread_t tid = rt_thread_create("hm", hm_thread_entry, NULL, 2048, 10, 10);
    
    rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(hm_init);
