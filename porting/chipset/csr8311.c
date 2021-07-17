#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"
#include "hm_config.h"

#if HM_CONFIG_BTSTACK
#include "bluetooth.h"
#include "hci_dump.h"
#endif

static uint8_t download_commands[] = {
    // 0x01fe: Set ANA_Freq to 26MHz
    0x00, 0xFC, 0x13, 0xc2, 0x02, 0x00, 0x09, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00, 0x00, 0xfe, 0x01, 0x01, 0x00, 0x08, 0x00, 0x90, 0x65,
    // 0x00f2: Set HCI_NOP_DISABLE
    0x00, 0xFC, 0x13, 0xc2, 0x02, 0x00, 0x09, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00, 0x00, 0xf2, 0x00, 0x01, 0x00, 0x08, 0x00, 0x01, 0x00,
    // 0x01bf: Enable RTS/CTS for BCSP (0806 -> 080e)
    0x00, 0xFC, 0x13, 0xc2, 0x02, 0x00, 0x09, 0x00, 0x01, 0x00, 0x03, 0x70, 0x00, 0x00, 0xbf, 0x01, 0x01, 0x00, 0x08, 0x00, 0x0e, 0x08,
    // 0x01ea: Set UART baudrate to 115200
    0x00, 0xFC, 0x15, 0xc2, 0x02, 0x00, 0x0a, 0x00, 0x02, 0x00, 0x03, 0x70, 0x00, 0x00, 0xea, 0x01, 0x02, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0xc2,
    // 0x0001: Set Bluetooth address 
    0x00, 0xFC, 0x19, 0xc2, 0x02, 0x00, 0x0A, 0x00, 0x03, 0x00, 0x03, 0x70, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x08, 0x00, 0xf3, 0x00, 0xf5, 0xf4, 0xf2, 0x00, 0xf2, 0xf1,
    //  WarmReset
    0x00, 0xFC, 0x13, 0xc2, 0x02, 0x00, 0x09, 0x00, 0x03, 0x0e, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
};

static void hci_vendor_evt_callback(uint8_t *hci_evt, uint16_t len)
{
#if HM_CONFIG_BTSTACK
    hci_dump_packet(HCI_EVENT_PACKET, 1, hci_evt, len);
#endif
}

int chipset_csr_init(void)
{
    uint8_t *p = download_commands;
    uint8_t *end = download_commands + ARRAY_SIZE(download_commands);
    uint16_t len = 0;

    while (p < end) {
        len = 3 + p[2];

#if HM_CONFIG_BTSTACK
        hci_dump_packet(HCI_COMMAND_DATA_PACKET, 0, p, len);
#endif

        int err = hci_vendor_cmd_send_sync(p, len, 1000, hci_vendor_evt_callback);
        if (err) {
            rt_kprintf("csr chipset init fail err(%d)\n", err);
            return HM_CHIPSET_INIT_ERROR;
        }

        p += len;
    }

    return HM_SUCCESS;
}

static hm_chipset_t chipset_csr = {
    .name = "csr",
    .init = chipset_csr_init,
};

hm_chipset_t* hm_chipset_get_instance(void)
{
    return &chipset_csr;
}
