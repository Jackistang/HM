#include "chipset.h"
#include "hci_transport_h4.h"



static uint8_t init_commands[] = {
    // OGF: 0x3F, OCF: 0x09, Paramter Total Length: 0x00. 
    // Zephyr Controller 
    0x09, 0xfc, 0x00, 
};

int chipset_zephyr_init(void)
{
    int err = hci_cmd_send_sync(init_commands, ARRAY_SIZE(init_commands), 1000, NULL);
    if (err) {
        rt_kprintf("Zephyr chipset init fail err(%d)\n", err);
        return HM_CHIPSET_INIT_ERROR;
    }

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