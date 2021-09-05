#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "hm_config.h"
#include "hm_hci_transport_h4.h"
#include "hm_hci_transport_h4_uart.h"

typedef enum {
    H4_RECV_STATE_NONE,
    H4_RECV_STATE_CMD = 1,
    H4_RECV_STATE_ACL,
    H4_RECV_STATE_SCO,
    H4_RECV_STATE_EVT,
    H4_RECV_STATE_ISO,
} h4_recv_state_t;

#define HM_MEMPOOL_ALLOC_TIMEOUT    10

#define HCI_COMMAND_BUF_SIZE RT_ALIGN(255, RT_ALIGN_SIZE) 
#define HCI_EVENT_BUF_SIZE   RT_ALIGN(255,  RT_ALIGN_SIZE)
#define HCI_ACL_BUF_SIZE     RT_ALIGN(255, RT_ALIGN_SIZE)

struct h4_rx_evt {
    uint8_t *data;
    uint16_t cur;
    uint16_t len;
};

struct h4_rx_acl {
    uint8_t *data;
    uint16_t cur;
    uint16_t len;
};

struct h4_rx {
    h4_recv_state_t state;
    union {
        struct h4_rx_evt evt;
        struct h4_rx_acl acl;
    };
};

struct h4_tx {

};

struct h4_object {
    struct h4_rx rx;
    struct h4_tx tx;

    rt_mailbox_t mb;
};

static struct h4_object h4_object;

static int hci_trans_h4_alloc(uint8_t type, uint8_t **ptr);
static void hci_trans_h4_free(void *ptr);

static struct rt_mempool cmd_pool;
ALIGN(RT_ALIGN_SIZE)
static uint8_t cmd_pool_buf[MEMPOOL_SIZE(1, HCI_COMMAND_BUF_SIZE)];

static struct rt_mempool evt_pool;
ALIGN(RT_ALIGN_SIZE)
static uint8_t evt_pool_buf[MEMPOOL_SIZE(5, HCI_EVENT_BUF_SIZE)];

static struct rt_mempool acl_pool;
ALIGN(RT_ALIGN_SIZE)
static uint8_t acl_pool_buf[MEMPOOL_SIZE(2, HCI_ACL_BUF_SIZE)];

void hci_trans_h4_init(struct hci_trans_h4_config *config)
{
    rt_mp_init(&cmd_pool, "cmd_pool", cmd_pool_buf, ARRAY_SIZE(cmd_pool_buf), HCI_COMMAND_BUF_SIZE);
    rt_mp_init(&evt_pool, "evt_pool", evt_pool_buf, ARRAY_SIZE(evt_pool_buf), HCI_EVENT_BUF_SIZE);
    rt_mp_init(&acl_pool, "acl_pool", acl_pool_buf, ARRAY_SIZE(acl_pool_buf), HCI_ACL_BUF_SIZE);

    h4_object.mb = rt_mb_create("h4.mb", 10, RT_IPC_FLAG_PRIO);

    hci_trans_h4_uart_init(&config->uart_config);

    rt_kprintf("hci transport h4 init success.\n");
}

int hci_trans_h4_open(void)
{
    int err;
    h4_object.rx.state = H4_RECV_STATE_NONE;

    if ((err = hci_trans_h4_uart_open()))
        return err;

    rt_kprintf("hci transport h4 open success.\n");

    return HM_SUCCESS;
}

int hci_trans_h4_close(void)
{
    int err;
    h4_object.rx.state = H4_RECV_STATE_NONE;

    // rt_sem_detach(&h4_object.send_sync_object.sync_sem);

    if ((err = hci_trans_h4_uart_close()))
        return err;

    rt_kprintf("hci transport h4 close success.\n");

    return HM_SUCCESS;
}

static int h4_recv_type(uint8_t byte)
{
    switch (byte) {
    case H4_RECV_STATE_EVT:
        h4_object.rx.evt.data = NULL;
        h4_object.rx.evt.cur = 0;
        h4_object.rx.evt.len = 0;
        break;
    case H4_RECV_STATE_ACL:
        h4_object.rx.acl.data = NULL;
        h4_object.rx.acl.cur = 0;
        h4_object.rx.acl.len = 0;
        break;
    default:
        return HM_NOT_SUPPORT;
    }

    h4_object.rx.state = byte;
    return HM_SUCCESS;
}

static int h4_recv_acl(uint8_t byte)
{
    int err = HM_SUCCESS;
    struct h4_rx_acl *acl = &h4_object.rx.acl;

    if (!acl->data) {
        if ((err = hci_trans_h4_alloc(H4_RECV_STATE_ACL, &acl->data)))
            return err;
        rt_memset(acl->data, 0, HCI_ACL_BUF_SIZE);
        acl->data[0] = 2;       //< Add H4 ACL type.
        acl->data++;            //< Leave a byte of space for storage type
    }

    acl->data[acl->cur++] = byte;

    if (acl->cur < sizeof(struct hm_hci_acl))
        return HM_SUCCESS;

    if (acl->cur == sizeof(struct hm_hci_acl)) {
        acl->len = (uint16_t)acl->data[2] | (uint16_t)acl->data[3] << 8;    // Parameter length
        acl->len += sizeof(struct hm_hci_acl);     // ACL package header.
    }

    if (acl->cur == acl->len) {
        /* The user who receive mail from "event mailbox" is responsible for free it's memory.*/
        err = rt_mb_send(h4_object.mb, (rt_ubase_t)(acl->data-1));
        if (err) {
            rt_kprintf("acl mailbox send mail fail\n");
        }

        h4_object.rx.state = H4_RECV_STATE_NONE;
    }

    return HM_SUCCESS;
}

static int h4_recv_evt(uint8_t byte)
{
    int err = HM_SUCCESS;
    struct h4_rx_evt *evt = &h4_object.rx.evt;

    if (!evt->data) {
        if ((err = hci_trans_h4_alloc(H4_RECV_STATE_EVT, &evt->data)))
            return err;

        rt_memset(evt->data, 0, HCI_EVENT_BUF_SIZE);
        evt->data[0] = 4;       //< Add H4 Event type.
        evt->data++;            //< Leave a byte of space for storage type
    }

    evt->data[evt->cur++] = byte;

    if (evt->cur < sizeof(struct hm_hci_evt))
        return HM_SUCCESS;
    
    if (evt->cur == sizeof(struct hm_hci_evt))
        evt->len = evt->data[1] + sizeof(struct hm_hci_evt);
    
    if (evt->cur == evt->len) {
        /* The user who receive mail from "event mailbox" is responsible for free it's memory.*/
        err = rt_mb_send(h4_object.mb, (rt_ubase_t)(evt->data-1));
        if (err) {
            rt_kprintf("event mailbox send mail fail\n");
        }

        h4_object.rx.state = H4_RECV_STATE_NONE;
    }

    return HM_SUCCESS;
}

int hci_trans_h4_recv_byte(uint8_t byte)
{
    int err = HM_SUCCESS;

    switch (h4_object.rx.state) {
    case H4_RECV_STATE_NONE:
        err = h4_recv_type(byte);
        break;
    case H4_RECV_STATE_ACL:
        err = h4_recv_acl(byte);
        break;
    case H4_RECV_STATE_EVT:
        err = h4_recv_evt(byte);
        break;
    case H4_RECV_STATE_CMD:
    case H4_RECV_STATE_SCO:
    case H4_RECV_STATE_ISO:
    default:
        return HM_NOT_SUPPORT;
    }

    if (err != HM_SUCCESS) {
        h4_object.rx.state = H4_RECV_STATE_NONE;
    }

    return err;
}

static int hci_trans_h4_alloc(uint8_t type, uint8_t **ptr)
{
    void *p = NULL;
    switch (type) {
    case HCI_TRANS_H4_TYPE_CMD:
        p = rt_mp_alloc(&cmd_pool, HM_MEMPOOL_ALLOC_TIMEOUT);
        break;
    case HCI_TRANS_H4_TYPE_EVT:
        p = rt_mp_alloc(&evt_pool, HM_MEMPOOL_ALLOC_TIMEOUT);
        break;
    case HCI_TRANS_H4_TYPE_ACL:
        p = rt_mp_alloc(&acl_pool, HM_MEMPOOL_ALLOC_TIMEOUT);
        break;
    case HCI_TRANS_H4_TYPE_SCO:
    case HCI_TRANS_H4_TYPE_ISO:
    default:
        return HM_NOT_SUPPORT;
    }
    
    if (p == NULL) {
        rt_kprintf("Memory pool (%d) is not enough.\n", type);
        return HM_NO_MEMORY;
    }

    *ptr = p;

    return HM_SUCCESS;
}

static void hci_trans_h4_free(void *ptr)
{
    rt_mp_free(ptr);
}

/*
    uint8_t *p = hci_trans_h4_send_alloc(0x01);
    fill hci command to p.
    hci_trans_h4_send(p);
    hci_trans_h4_send_free(p);
*/
int hci_trans_h4_send(uint8_t type, uint8_t *data)
{
    int err;
    uint16_t len = 0;
    
    switch (type) {
    case HCI_TRANS_H4_TYPE_CMD:
        len = 1 + data[2] + sizeof(struct hm_hci_cmd);
        break;
    case HCI_TRANS_H4_TYPE_ACL:
        len = 1 + ((uint16_t)data[2] | (uint16_t)data[3] << 8) + sizeof(struct hm_hci_acl);
        break;
    default:
        return HM_NOT_SUPPORT;
    }

    uint8_t *p = data - 1;
    *p = type;
    
    err = hci_trans_h4_uart_send(p, len);
    if (err != HM_SUCCESS)
        return err;

    return HM_SUCCESS;
}

/**
 * 
 * @note Alloced memory begin with the second byte in memory block, the first byte is used for H4 type when send.
*/
void *hci_trans_h4_send_alloc(uint8_t type)
{
    uint8_t *p = NULL;
    int err = HM_SUCCESS;

    if ((err = hci_trans_h4_alloc(type, &p)))
        return NULL;
    
    return p+1;
}

void hci_trans_h4_send_free(uint8_t *buf)
{
    RT_ASSERT(buf);

    uint8_t *p = buf - 1;
    hci_trans_h4_free(p);
}

int hci_trans_h4_recv_all(uint8_t **buf, int ms, uint8_t *type)
{   
    uint8_t *recv = NULL;
    rt_err_t err;

    err = rt_mb_recv(h4_object.mb, (rt_ubase_t *)&recv, ms);
    if (err != RT_EOK)
        return -HM_TIMEOUT;
    
    *type = *recv;
    *buf = recv + 1;

    return RT_EOK;
}

void hci_trans_h4_recv_free(uint8_t *p)
{
    hci_trans_h4_free(p - 1);
}
