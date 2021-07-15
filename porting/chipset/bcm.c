#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"

#define BCM_PATH    "BCM43430A1.hcd"

static uint8_t download_commands[] = {
    0x2e, 0xfc, 0x00,
};

int chipset_bcm_init(void)
{
    int err = hci_cmd_send_sync(download_commands, ARRAY_SIZE(download_commands), 1000, NULL);
    if (err) {
        rt_kprintf("bcm chipset init fail err(%d)\n", err);
        return HM_CHIPSET_INIT_ERROR;
    }

    // TODO open BCM_PATH, and read HCI Command to send until all command send.

    return HM_SUCCESS;
}

static hm_chipset_t chipset_bcm = {
    .name = "bcm",
    .init = chipset_bcm_init,
};

hm_chipset_t* hm_chipset_get_instance(void)
{
    return &chipset_bcm;
}