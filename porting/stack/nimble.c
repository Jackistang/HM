/* hci middle header */
#include "hm_hci_transport_h4.h"
#include "hm_chipset.h"

/* nimble header */
#include "nimble/ble.h"
#include "nimble/ble_hci_trans.h"

/* rt-thread header */
#include <rtthread.h>

static ble_hci_trans_rx_cmd_fn *ble_hci_uart_rx_cmd_cb;
static void *ble_hci_uart_rx_cmd_arg;

static ble_hci_trans_rx_acl_fn *ble_hci_uart_rx_acl_cb;
static void *ble_hci_uart_rx_acl_arg;

int ble_hci_trans_hs_cmd_tx(uint8_t *cmd)
{
    if (hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, cmd)) {
        hci_trans_h4_send_free(cmd);
        return BLE_ERR_UNKNOWN_HCI_CMD;
    }
    
    hci_trans_h4_send_free(cmd);
    return BLE_ERR_SUCCESS;
}

int ble_hci_trans_hs_acl_tx(struct os_mbuf *om)
{
    uint8_t pkt_type = HCI_TRANS_H4_TYPE_ACL;
    struct os_mbuf *om_next;

    hci_trans_h4_uart_send(&pkt_type, sizeof(pkt_type));

    while (om) {
        om_next = SLIST_NEXT(om, om_next);

        hci_trans_h4_uart_send(om->om_data, om->om_len);

        os_mbuf_free(om);
        om = om->om_next;
    }

    return 0;
}

uint8_t *ble_hci_trans_buf_alloc(int type)
{
    /* In this port, NimBLE only support Host. */
    uint8_t *buf = NULL;

    switch (type) {
    case BLE_HCI_TRANS_BUF_CMD:
        buf = hci_trans_h4_send_alloc(HCI_TRANS_H4_TYPE_CMD);
        break;
    default:
        RT_ASSERT(0);
        buf = NULL;
        break;
    }

    return buf;
}

/*
 * Called by NimBLE host to free buffer allocated for HCI Event packet.
 * Called by HCI transport to free buffer allocated for HCI Command packet.
 */
void ble_hci_trans_buf_free(uint8_t *buf)
{
    /* In this port, only called by NimBLE host to free buffer allocated for HCI Evnet packet */
    hci_trans_h4_recv_free(buf);
}

int ble_hci_trans_set_acl_free_cb(os_mempool_put_fn *cb, void *arg)
{
    RT_UNUSED(cb);
    RT_UNUSED(arg);

    return BLE_ERR_UNSUPPORTED;
}


void ble_hci_trans_cfg_hs(ble_hci_trans_rx_cmd_fn *cmd_cb,
                          void *cmd_arg,
                          ble_hci_trans_rx_acl_fn *acl_cb,
                          void *acl_arg)
{
    ble_hci_uart_rx_cmd_cb  = cmd_cb;
    ble_hci_uart_rx_cmd_arg = cmd_arg;
    ble_hci_uart_rx_acl_cb  = acl_cb;
    ble_hci_uart_rx_acl_arg = acl_arg;
}

int ble_hci_trans_reset(void)
{
    return BLE_ERR_SUCCESS;
}

/* TODO Create a thread to receive packet. */

/* Not supported now. */
int ble_hci_trans_ll_evt_tx(uint8_t *hci_ev)
{
    RT_ASSERT(0);

    return HM_NOT_SUPPORT;
}

int ble_hci_trans_ll_acl_tx(struct os_mbuf *om)
{
    RT_ASSERT(0);

    return HM_NOT_SUPPORT;
}

void ble_hci_trans_cfg_ll(ble_hci_trans_rx_cmd_fn *cmd_cb,
                          void *cmd_arg,
                          ble_hci_trans_rx_acl_fn *acl_cb,
                          void *acl_arg)
{
    RT_ASSERT(0);

    return HM_NOT_SUPPORT;
}
