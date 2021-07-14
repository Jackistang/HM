#ifndef HM_TRANS_H4_H
#define HM_TRANS_H4_H

#include "error.h"
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


// enum {
//     HCI_TRANSPORT_H4_COMMAND = 1,
//     HCI_TRANSPORT_H4_ACL,
//     HCI_TRANSPORT_H4_SYNC,
//     HCI_TRANSPORT_H4_EVENT,
//     HCI_TRANSPORT_H4_ISO,
// };

// struct rt_hci_transport_h4_config {
//     struct os_uart_config uart_config;
// };

// /**
//  * @brief Init hci transport h4 .
//  * 
//  * @param config    Init configure.
//  * 
//  * @return void
// */
// extern void rt_hci_transport_h4_init(struct rt_hci_transport_h4_config *config);

// /**
//  * @brief Open hci h4 transport, including open os_uart.
//  * 
//  * @return int
//  * @retval  0   Success
//  * @retval  -1  Fail
// */
// extern int rt_hci_transport_h4_open(void);

// /**
//  * @brief Close hci h4 transport, including close os_uart.
//  * 
//  * @return int
//  * @retval  0   Success
//  * @retval  -1  Fail
// */
// extern int rt_hci_transport_h4_close(void);

// /**
//  * @brief HCI transport h4 send package.
//  * 
//  * @param type      HCI package type.
//  * @param buf       HCI package buffer.
//  * @param length    HCI package length.
//  * 
//  * @return int
//  * @retval  >=0     Sended data length.
//  * @retval  -1      Send fail.
// */
// extern int rt_hci_transport_h4_send(int type, uint8_t *buf, size_t length);

// /**
//  * @brief Register a packet callback handler, is similar with `rt_hci_transport_h4_init()`
//  * 
//  * @param handler   Callback function pointer.
// */
// extern void rt_hci_transport_h4_register_packet_handler(void (*handler)(int packet_type, uint8_t *packet, uint16_t size));

#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))

extern void hci_trans_h4_init(void);
extern int hci_trans_h4_open(void);
extern int hci_trans_h4_close(void);

/**
 * @brief Register a callback for incoming package.
 * 
 * @param callback A callback function pointer.
 */
extern void hci_trans_h4_register_packge_callback(void (*callback)(uint8_t package_type, uint8_t *packge, uint16_t size));

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

#ifdef __cplusplus
}
#endif

#endif /* HM_TRANS_H4_H */
