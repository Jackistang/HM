#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
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


#define HCI_COMMAND_BUF_SIZE RT_ALIGN(260, RT_ALIGN_SIZE) 
#define HCI_EVENT_BUF_SIZE   RT_ALIGN(70,  RT_ALIGN_SIZE)
#define HCI_ACL_BUF_SIZE     RT_ALIGN(255, RT_ALIGN_SIZE)

/**
 * @param n             The count for the block.
 * @param block_size    One block size in memory pool.
*/
#define MEMPOOL_SIZE(n, block_size) (RT_ALIGN(((block_size) + 4), RT_ALIGN_SIZE) * (n))

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
typedef struct package_callback {
    hci_trans_h4_package_callback_t cb;
    rt_list_t list;
} package_callback_t;

typedef struct hci_send_sync_object {
    int cc_evt; /* Receive command complete event or not. */
    hci_cmd_send_sync_callback_t cb;
    struct rt_semaphore sync_sem;
} hci_send_sync_object_t;

struct h4_object {
    struct h4_rx rx;
    struct h4_tx tx;

    hci_send_sync_object_t send_sync_object; 

    rt_list_t   callback_list;
};

static struct h4_object h4_object;

static int hci_trans_h4_alloc(uint8_t type, uint8_t **ptr);
static void hci_trans_h4_free(void *ptr);

static struct rt_mempool cmd_pool;
ALIGN(RT_ALIGN_SIZE)
static uint8_t cmd_pool_buf[MEMPOOL_SIZE(1, HCI_COMMAND_BUF_SIZE)];

static struct rt_mempool evt_pool;
ALIGN(RT_ALIGN_SIZE)
static uint8_t evt_pool_buf[MEMPOOL_SIZE(1, HCI_EVENT_BUF_SIZE)];

static struct rt_mempool acl_pool;
ALIGN(RT_ALIGN_SIZE)
static uint8_t acl_pool_buf[MEMPOOL_SIZE(1, HCI_ACL_BUF_SIZE)];

void hci_trans_h4_init(struct hci_trans_h4_config *config)
{
    rt_mp_init(&cmd_pool, "cmd_pool", cmd_pool_buf, ARRAY_SIZE(cmd_pool_buf), HCI_COMMAND_BUF_SIZE);
    rt_mp_init(&evt_pool, "evt_pool", evt_pool_buf, ARRAY_SIZE(evt_pool_buf), HCI_EVENT_BUF_SIZE);
    rt_mp_init(&acl_pool, "acl_pool", acl_pool_buf, ARRAY_SIZE(acl_pool_buf), HCI_ACL_BUF_SIZE);

    hci_trans_h4_uart_init(&config->uart_config);

    h4_object.send_sync_object.cb = NULL;
    rt_sem_init(&h4_object.send_sync_object.sync_sem, "send sync sem", 0, RT_IPC_FLAG_PRIO);

    rt_list_init(&h4_object.callback_list);
}

int hci_trans_h4_open(void)
{
    int err;
    h4_object.rx.state = H4_RECV_STATE_NONE;

    if ((err = hci_trans_h4_uart_open()))
        return err;

    /* For test */
#define BT_AP6212_PIN GET_PIN(I, 11)
    rt_pin_mode(BT_AP6212_PIN, PIN_MODE_OUTPUT);

    rt_pin_write(BT_AP6212_PIN, PIN_LOW);
    HAL_Delay(1000);
    rt_pin_write(BT_AP6212_PIN, PIN_HIGH);
    HAL_Delay(1000);

    return HM_SUCCESS;
}

int hci_trans_h4_close(void)
{
    int err;
    h4_object.rx.state = H4_RECV_STATE_NONE;

    if ((err = hci_trans_h4_uart_close()))
        return err;

    return HM_SUCCESS;
}

int hci_trans_h4_register_callback(hci_trans_h4_package_callback_t callback)
{
    package_callback_t *pkg_callback;
    rt_list_for_each_entry(pkg_callback, &h4_object.callback_list, list) {
        if (pkg_callback->cb == callback)   /* This callback function has been registered. */
            return HM_SUCCESS;
    }

    pkg_callback = rt_malloc(sizeof(package_callback_t));
    if (pkg_callback == NULL)
        return HM_NO_MEMORY;
    
    pkg_callback->cb = callback;
    rt_list_insert_after(&h4_object.callback_list, &pkg_callback->list);

    return HM_SUCCESS;
}

void hci_trans_h4_remove_callback(hci_trans_h4_package_callback_t callback)
{
    package_callback_t *pkg_callback;
    rt_list_for_each_entry(pkg_callback, &h4_object.callback_list, list) {
        if (pkg_callback->cb == callback)
            break;
    }
    rt_list_remove(&pkg_callback->list);
    rt_free(pkg_callback);
}

static void hci_trans_h4_pkg_notify(uint8_t type, uint8_t *pkg, uint16_t len)
{
    /* hci_cmd_send_sync() function will set this, used for sync callback, 
        and don't call other registered callback. */
    if (h4_object.send_sync_object.cb) {
        RT_ASSERT(type == HCI_TRANS_H4_TYPE_EVT);
        h4_object.send_sync_object.cb(pkg, len);

        /* Command Complete event */
        if (pkg[0] == 0x0E) {
            h4_object.send_sync_object.cc_evt = 1;
        }
        rt_sem_release(&h4_object.send_sync_object.sync_sem);
        return ;
    }

    package_callback_t *pkg_callback;
    rt_list_for_each_entry(pkg_callback, &h4_object.callback_list, list) {
        pkg_callback->cb(type, pkg, len);
    }
}

static int hci_trans_h4_recv_type(uint8_t byte)
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

static int hci_trans_h4_recv_acl(uint8_t byte)
{
    int err = HM_SUCCESS;
    struct h4_rx_acl *acl = &h4_object.rx.acl;

    if (!acl->data) {
        if ((err = hci_trans_h4_alloc(H4_RECV_STATE_ACL, &acl->data)))
            return err;
    }

    acl->data[acl->cur++] = byte;

    if (acl->cur < sizeof(struct hm_hci_acl))
        return HM_SUCCESS;

    if (acl->cur == sizeof(struct hm_hci_acl)) {
        acl->len = (uint16_t)acl->data[2] | (uint16_t)acl->data[3] << 8;    // Parameter length
        acl->len += sizeof(struct hm_hci_acl);     // ACL package header.
    }

    if (acl->cur == acl->len) {
        hci_trans_h4_pkg_notify(HCI_TRANS_H4_TYPE_ACL, acl->data, acl->len);
#ifdef HM_CONFIG_BTSTACK
        /* Transfer the responsibility of free memory to btstack user.*/
#else
        hci_trans_h4_free(acl->data);
#endif
        h4_object.rx.state = H4_RECV_STATE_NONE;
    }

    return HM_SUCCESS;
}

static int hci_trans_h4_recv_evt(uint8_t byte)
{
    int err = HM_SUCCESS;
    struct h4_rx_evt *evt = &h4_object.rx.evt;

    if (!evt->data) {
        if ((err = hci_trans_h4_alloc(H4_RECV_STATE_EVT, &evt->data)))
            return err;
    }

    evt->data[evt->cur++] = byte;

    if (evt->cur < sizeof(struct hm_hci_evt))
        return HM_SUCCESS;
    
    if (evt->cur == sizeof(struct hm_hci_evt))
        evt->len = evt->data[1] + sizeof(struct hm_hci_evt);
    
    if (evt->cur == evt->len) {
        hci_trans_h4_pkg_notify(HCI_TRANS_H4_TYPE_EVT, evt->data, evt->len);
#ifdef HM_CONFIG_BTSTACK
        /* Transfer the responsibility of free memory to btstack user.*/
#else
        hci_trans_h4_free(evt->data);
#endif
        h4_object.rx.state = H4_RECV_STATE_NONE;
    }

    return HM_SUCCESS;
}

int hci_trans_h4_recv_byte(uint8_t byte)
{
    int err = HM_SUCCESS;

    switch (h4_object.rx.state) {
    case H4_RECV_STATE_NONE:
        err = hci_trans_h4_recv_type(byte);
        break;
    case H4_RECV_STATE_ACL:
        err = hci_trans_h4_recv_acl(byte);
        break;
    case H4_RECV_STATE_EVT:
        err = hci_trans_h4_recv_evt(byte);
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
        p = rt_mp_alloc(&cmd_pool, RT_WAITING_NO);
        break;
    case HCI_TRANS_H4_TYPE_EVT:
        p = rt_mp_alloc(&evt_pool, RT_WAITING_NO);
        break;
    case HCI_TRANS_H4_TYPE_ACL:
        p = rt_mp_alloc(&acl_pool, RT_WAITING_NO);
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

static void hci_cmd_send_sync_dummy_callback(uint8_t *hci_evt, uint16_t len)
{
    return ;
}

int hci_cmd_send_sync(uint8_t *hci_cmd, uint16_t len, int32_t time, hci_cmd_send_sync_callback_t callback)
{
    RT_ASSERT(hci_cmd);
    RT_ASSERT(len > 0);

    int err;
    
    if (callback == NULL)
        h4_object.send_sync_object.cb = hci_cmd_send_sync_dummy_callback;
    else
        h4_object.send_sync_object.cb = callback;

    uint8_t *p = hci_trans_h4_send_alloc(HCI_TRANS_H4_TYPE_CMD);
    if (p == NULL) {
        err = HM_NO_MEMORY;
        goto err_alloc;
    }

    rt_memcpy(p, hci_cmd, len);
    if ((err = hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, p)))
        goto err_send;

    err = rt_sem_take(&h4_object.send_sync_object.sync_sem, time);
    if (err) {
        rt_kprintf("HCI command send sync timeout.\n");
        goto err_timeout;
    }

    if (h4_object.send_sync_object.cc_evt) {
        err = HM_SUCCESS;
        h4_object.send_sync_object.cc_evt = 0;
    } else {
        err = HM_HCI_CMD_ERROR;
    }

err_timeout:
err_send:
    hci_trans_h4_send_free(p);
err_alloc:
    h4_object.send_sync_object.cb = NULL;
    return err;
}

