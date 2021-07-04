#ifndef HCI_MIDDLEWARE_TRANSPORT_H4_H
#define HCI_MIDDLEWARE_TRANSPORT_H4_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    HCI_TRANSPORT_H4_COMMAND = 1,
    HCI_TRANSPORT_H4_ACL,
    HCI_TRANSPORT_H4_SYNC,
    HCI_TRANSPORT_H4_EVENT,
    HCI_TRANSPORT_H4_ISO,
};

struct rt_hci_transport_h4_config {
    /**
     * @brief Receive package callback.
     * 
     * @param type      Controller to Host package type, ACL, SYNC, EVENT, or ISO.
     * @param buf       Package buffer.
     * @param length    Package length
    */
    void (*package_callback)(int type, uint8_t *buf, size_t length);
};

/**
 * @brief Init hci transport h4 .
 * 
 * @param config    Init configure.
 * 
 * @return void
*/
extern void rt_hci_transport_h4_init(struct rt_hci_transport_h4_config *config);

/**
 * @brief HCI transport h4 send package.
 * 
 * @param type      HCI package type.
 * @param buf       HCI package buffer.
 * @param length    HCI package length.
 * 
 * @return int
 * @retval  >=0     Sended data length.
 * @retval  -1      Send fail.
*/
extern int rt_hci_transport_h4_send(int type, uint8_t *buf, size_t length);

/**
 * @brief Register a packet callback handler, is similar with `rt_hci_transport_h4_init()`
 * 
 * @param handler   Callback function pointer.
*/
extern void rt_hci_transport_h4_register_packet_handler(void (*handler)(int packet_type, uint8_t *packet, uint16_t size));

#ifdef __cplusplus
}
#endif

#endif /* HCI_MIDDLEWARE_TRANSPORT_H4_H */
