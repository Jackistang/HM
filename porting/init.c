#include "hm_chipset.h"
#include "hm_config.h"
#include "hm_hci_transport_h4.h"
#include "init.h"

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

static int hm_init(void)
{
    hci_trans_h4_init(&h4_config);
    hci_trans_h4_open();

    hm_chipset_t *chipset_instance = hm_chipset_get_instance();
    chipset_instance->init();

#if HM_CONFIG_NIMBLE
    hm_nimble_init();
#endif

#if HM_CONFIG_BTSTACK
    btstack_rtthread_port_init();
#endif

    return RT_EOK;
}
INIT_APP_EXPORT(hm_init);
