#include "hm_chipset.h"
#include "hm_config.h"
#include "hm_hci_transport_h4.h"
#include "init.h"

#ifdef HM_USING_STACK_NIMBLE
#include "host/ble_hs_id.h"
#endif

#include <rtthread.h>

static struct hci_trans_h4_config h4_config = {
    .uart_config = {
        .device_name = "uart3",             /* Default value */
        .databit     = DATA_BITS_8,
        .stopbit     = STOP_BITS_1,
        .parity      = PARITY_NONE,
        .baudrate    = BAUD_RATE_115200,    /* Default value */
        .flowcontrol = 1,                   /* Default value */
    },
};

static void hm_thread_entry(void *args)
{
    hci_trans_h4_init(&h4_config);
    hci_trans_h4_open();

    hm_chipset_t *chipset_instance = hm_chipset_get_instance();
    chipset_instance->init();

#ifdef HM_USING_STACK_NIMBLE
    hm_nimble_init();
#endif

#ifdef HM_USING_STACK_BTSTACK
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
