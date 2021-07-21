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


/**
 * @brief HCI transport h4 receive a byte from uart, used for hci_transport_h4_uart.c .
 * 
 * @param byte A byte coming from uart.
 * 
 * @return int 
 * @retval  HM_SUCCESS       Receive byte success.
 * @retval  -HM_NOT_SUPPORT  H4 sync loss, or this package not support now.
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
 * @retval  HM_SUCCESS       Send success.
 * @retval  -HM_NOT_SUPPORT  This type package not support now.
 */
extern int hci_trans_h4_send(uint8_t type, uint8_t *data);

/**
 * @brief HCI transport h4 receive a hci event.
 * 
 * @param buf   A pointer to hci event buffer when receive event successfully.
 * @param ms    Waitting time in ms. Specially, 
 *          RT_WAITING_NO means no wait, 
 *          RT_WAITING_FOREVER means wait forever.
 * 
 * @return int 
 * @retval  HM_SUCCESS      Read hci event success.
 * @retval  -HM_TIMEOUT     Timeout.
 * 
 * @note If this function return successfully, `buf` should be 
 *      freed with `hci_trans_h4_recv_free()` when it's not needed.
 */
int hci_trans_h4_recv_event(uint8_t **buf, int ms);

/**
 * @brief HCI transport h4 receive a hci acl packet.
 * 
 * @param buf   A pointer to hci acl packet buffer when receive acl packet successfully.
 * @param ms    Waitting time in ms. Specially, 
 *          RT_WAITING_NO means no wait, 
 *          RT_WAITING_FOREVER means wait forever.
 * 
 * @return int 
 * @retval  HM_SUCCESS      Read hci acl packet success.
 * @retval  -HM_TIMEOUT     Timeout.
 * 
 * @note If this function return successfully, `buf` should be 
 *      freed with `hci_trans_h4_recv_free()` when it's not needed.
 */
int hci_trans_h4_recv_acl(uint8_t **buf, int ms);

/**
 * @brief HCI transport h4 receive a packet, which type is not limited.
 * 
 * @param buf   A pointer to hci packet buffer when receive successfully.
 * @param ms    Waitting time in ms. Specially, RT_WAITING_NO means no wait,
 * @param type  A pointer to restore hci packet type.
 * 
 * @return int 
 * @retval  HM_SUCCESS      Read hci acl packet success.
 * @retval  -HM_TIMEOUT     Timeout.
 * 
 * @note If this function return successfully, `buf` should be 
 *      freed with `hci_trans_h4_recv_free()` when it's not needed.
 * 
 * @note ms shouldn't be RT_WAITING_FOREVER.
 */
int hci_trans_h4_recv_all(uint8_t **buf, int ms, uint8_t *type);

/**
 * @brief Free memory buffer, which is alloc by `hci_trans_h4_recv_*` API.
 * 
 * @param p     Memory buffer.
 */
void hci_trans_h4_recv_free(uint8_t *p);

#ifdef __cplusplus
}
#endif

#endif /* HM_TRANS_H4_H */
