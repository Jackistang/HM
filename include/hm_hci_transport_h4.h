#ifndef HM_TRANS_H4_H
#define HM_TRANS_H4_H

#include "hm_error.h"
#include "hm_hci_transport_h4_uart.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HCI_TRANS_H4_TYPE_CMD   (0x01)
#define HCI_TRANS_H4_TYPE_ACL   (0x02)
#define HCI_TRANS_H4_TYPE_SCO   (0x03)
#define HCI_TRANS_H4_TYPE_EVT   (0x04)
#define HCI_TRANS_H4_TYPE_ISO   (0x05)

/**
 * @param n             The count for the block.
 * @param block_size    One block size in memory pool.
*/
#define MEMPOOL_SIZE(n, block_size) (RT_ALIGN(((block_size) + 4), RT_ALIGN_SIZE) * (n))


struct hm_hci_cmd {
    uint16_t opcode;
    uint8_t length;
    uint8_t data[0];
} __attribute__((packed));

struct hm_hci_acl {
    uint16_t handle;    // Include PB, BC flag.
    uint16_t length;
    uint8_t data[0];
} __attribute__((packed));

struct hm_hci_sco {
    uint16_t handle;    // Include Package Status flag, RFU.
    uint8_t length;
    uint8_t data[0];
} __attribute__((packed));

struct hm_hci_evt {
    uint8_t evt_code;
    uint8_t length;
    uint8_t data[0];
} __attribute__((packed));

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

struct hci_trans_h4_config {
    struct hci_trans_h4_uart_config uart_config;
};

extern void hci_trans_h4_init(struct hci_trans_h4_config *config);
extern int hci_trans_h4_open(void);
extern int hci_trans_h4_close(void);


// typedef void (*hci_trans_h4_package_callback_t)(uint8_t pkg_type, uint8_t *pkg, uint16_t size);

// /**
//  * @brief Register a callback for incoming package.
//  * 
//  * @param callback A callback function pointer.
//  * 
//  * @return int
//  * @retval  HM_SUCCESS      Register success.
//  * @retval  HM_NO_MEMORY    Malloc memory fail.
//  * 
//  * @note If use btstack, this callback is resposibility to free package buffer with `rt_mp_free()`.
//  */
// extern int hci_trans_h4_register_callback(hci_trans_h4_package_callback_t callback);


// extern void hci_trans_h4_remove_callback(hci_trans_h4_package_callback_t callback);

/**
 * @brief HCI transport h4 receive a byte from uart, used for hci_transport_h4.c .
 * 
 * @param byte A byte coming from uart.
 * 
 * @return int 
 * @retval  HM_SUCCESS      Receive byte success.
 * @retval  HM_NOT_SUPPORT  H4 sync loss, or this package not support now.
 */
extern int hci_trans_h4_recv_byte(uint8_t byte);

/**
 * @brief Alloc a enough memory to send data.
 * 
 * @param type The H4 package type, HCI_TRANS_H4_TYPE_CMD, HCI_TRANS_H4_TYPE_ACL, ...
 * 
 * @return void* 
 * @retval Non-NULL A memory to storage HCI data.
 * @retval NULL     Alloc fail. Memory pool is mot enough or this type package not support send_alloc.
 * 
 * @note    If memory alloc success, need to free it use `hci_trans_h4_send_free` .
 */
extern void *hci_trans_h4_send_alloc(uint8_t type);

/**
 * @brief Free memory buffer alloc by `hci_trans_h4_send_alloc` .
 * 
 * @param buf Memory buffer.
 */
extern void hci_trans_h4_send_free(uint8_t *buf);

/**
 * @brief HCI transport h4 send package to uart.
 * 
 * @param type The H4 package type, HCI_TRANS_H4_TYPE_CMD, HCI_TRANS_H4_TYPE_ACL, ...
 * @param data HCI package data.
 * 
 * @return int 
 * @retval  HM_SUCCESS  Send success.
 * @retval  HM_NOT_SUPPORT  This type package not support now.
 */
extern int hci_trans_h4_send(uint8_t type, uint8_t *data);

// typedef void (*hci_vendor_evt_callback_t)(uint8_t *hci_evt, uint16_t len);

// extern int hci_vendor_cmd_send_sync(uint8_t *hci_cmd, uint16_t len, int32_t time, hci_vendor_evt_callback_t callback);

// extern int hci_cmd_send_sync(uint8_t *hci_cmd, uint16_t len, int32_t time);

// extern int hci_reset_cmd_send(void);

int hci_trans_h4_recv_event(uint8_t **buf, int ms);
void hci_trans_h4_recv_free(uint8_t *p);

#ifdef __cplusplus
}
#endif

#endif /* HM_TRANS_H4_H */
