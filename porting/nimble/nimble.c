/* hci middle header */
#include "hm_hci_transport_h4.h"
#include "hm_hci_transport_h4_uart.h"
#include "hm_chipset.h"

/* nimble header */
#include "nimble/ble.h"
#include "nimble/ble_hci_trans.h"
#include "nimble/hci_common.h"
#include "os/os_mempool.h"
#include "os/os_mbuf.h"

/* rt-thread header */
#include <rtthread.h>

static ble_hci_trans_rx_cmd_fn *ble_hci_uart_rx_cmd_cb;
static void *ble_hci_uart_rx_cmd_arg;

static ble_hci_trans_rx_acl_fn *ble_hci_uart_rx_acl_cb;
static void *ble_hci_uart_rx_acl_arg;

static struct os_mbuf_pool ble_hci_uart_acl_mbuf_pool;
static struct os_mempool_ext ble_hci_uart_acl_pool;

#define ACL_BLOCK_SIZE  OS_ALIGN(MYNEWT_VAL(BLE_ACL_BUF_SIZE) \
                                 + BLE_MBUF_MEMBLOCK_OVERHEAD \
                                 + BLE_HCI_DATA_HDR_SZ, OS_ALIGNMENT)

static os_membuf_t ble_hci_uart_acl_buf[
	OS_MEMPOOL_SIZE(MYNEWT_VAL(BLE_ACL_BUF_COUNT), 
                    ACL_BLOCK_SIZE)  /* Now is only 1 block in this pool. */
];

static rt_thread_t nimble_tid;

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
        om = om_next;
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

    return ;
}

static struct os_mbuf * ble_hci_trans_acl_buf_alloc(void)
{
    struct os_mbuf *m;
    uint8_t usrhdr_len;

#if MYNEWT_VAL(BLE_DEVICE)
    usrhdr_len = sizeof(struct ble_mbuf_hdr);
#elif MYNEWT_VAL(BLE_HS_FLOW_CTRL)
    usrhdr_len = BLE_MBUF_HS_HDR_LEN;
#else
    usrhdr_len = 0;
#endif

    m = os_mbuf_get_pkthdr(&ble_hci_uart_acl_mbuf_pool, usrhdr_len);
    return m;
}

static void hm_nimble_thread_entry(void *args)
{
    uint8_t *recv = NULL;
    uint8_t type;
    int err;

    while (1) {
        err = hci_trans_h4_recv_all(&recv, 100, &type);
        if (err) {
            continue;
        }

        switch (type) {
        case HCI_TRANS_H4_TYPE_EVT: {
            ble_hci_uart_rx_cmd_cb(recv, ble_hci_uart_rx_cmd_arg);
            break;
        }
        case HCI_TRANS_H4_TYPE_ACL: {
            struct os_mbuf *om = ble_hci_trans_acl_buf_alloc();
            uint16_t packet_len = 4 + ((uint16_t)recv[2] | (uint16_t)recv[3] << 8);
            RT_ASSERT(packet_len <= 255);

            rt_memcpy(om->om_data, recv, packet_len);
            hci_trans_h4_recv_free(recv);

            om->om_len = packet_len;
            OS_MBUF_PKTLEN(om) = packet_len;
            ble_hci_uart_rx_acl_cb(om, ble_hci_uart_rx_acl_arg);
            break;
        }
        }
    }
}

int hm_nimble_init(void)
{
    int rc;

    rc = os_mempool_ext_init(&ble_hci_uart_acl_pool,
                             MYNEWT_VAL(BLE_ACL_BUF_COUNT),
                             ACL_BLOCK_SIZE,
                             ble_hci_uart_acl_buf,
                             "ble_hci_uart_acl_pool");
    RT_ASSERT(rc == 0);

    rc = os_mbuf_pool_init(&ble_hci_uart_acl_mbuf_pool,
                           &ble_hci_uart_acl_pool.mpe_mp,
                           ACL_BLOCK_SIZE,
                           MYNEWT_VAL(BLE_ACL_BUF_COUNT));
    RT_ASSERT(rc == 0);

    nimble_tid = rt_thread_create("hm.nimble", hm_nimble_thread_entry, NULL,
                                512, 10, 10);
    RT_ASSERT(nimble_tid != NULL);

    rt_thread_startup(nimble_tid);

    return RT_EOK;
}
