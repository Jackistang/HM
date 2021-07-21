#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"



static uint8_t init_commands[] = {
    // OGF: 0x3F, OCF: 0x09, Paramter Total Length: 0x00. 
    // Zephyr Controller 
    0x09, 0xfc, 0x00, 
};

int chipset_zephyr_init(void)
{
    uint8_t event_buf[20];

    chip_send_hci_reset_cmd_until_ack();

    rt_kprintf("Zephyr controller start init\n");

    chip_hci_cmd_send(init_commands, ARRAY_SIZE(init_commands));

    chip_hci_event_read(event_buf, ARRAY_SIZE(event_buf), RT_WAITING_FOREVER);
    if (event_buf[0] != 0x0E) {
        rt_kprintf("Zephyr controller init fail\n");
        return HM_CHIPSET_INIT_ERROR;
    }

    chip_send_hci_reset_cmd_until_ack();

    rt_kprintf("Zephyr controller init success\n");

    return HM_SUCCESS;
}

static hm_chipset_t chipset_zephyr = {
    .name = "Zephyr",
    .init = chipset_zephyr_init,
};

hm_chipset_t* hm_chipset_get_instance(void)
{
    return &chipset_zephyr;
}