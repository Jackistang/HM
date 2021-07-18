#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"
#include <rtthread.h>


int chip_init(hm_chipset_t* chip)
{

    return HM_SUCCESS;
}


/* Send a command */
int chip_hci_cmd_send(uint8_t *buf, uint16_t size)
{
    uint8_t *p = hci_trans_h4_send_alloc(HCI_TRANS_H4_TYPE_CMD);
    if (p == NULL)
        return HM_NO_MEMORY;
    
    rt_memcpy(p, buf, size);
    
    if (hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, p) != HM_SUCCESS) {
        hci_trans_h4_send_free(p);
        return HM_UART_SEND_ERR;
    }

    hci_trans_h4_send_free(p);
    return HM_SUCCESS;
}

/* Wait until read a event. */
int chip_hci_event_read(uint8_t *buf, uint16_t size)
{
    uint8_t *p = NULL;
    int err;

    hci_trans_h4_recv_event(&p, RT_WAITING_FOREVER);
    RT_ASSERT(p);

    uint16_t len = 2 + p[1];    /* HCI Event length */
    len = (len > size) ? size : len;

    rt_memcpy(buf, p, len);
    hci_trans_h4_recv_free(p);

    return HM_SUCCESS;
}
