#ifndef HCI_TRANSPORT_H4
#define HCI_TRANSPORT_H4

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HCI_TRANSPORT_H4_COMMAND = 1,
    HCI_TRANSPORT_H4_ACL,
    HCI_TRANSPORT_H4_SYNC,
    HCI_TRANSPORT_H4_EVENT,
    HCI_TRANSPORT_H4_ISO,
} HCI_TRANSPORT_H4_PACKAGE_TYPE;

int rt_hci_transport_h4_send_command(HCI_TRANSPORT_H4_PACKAGE_TYPE type, uint8_t *buf, size_t size);

void rt_hci_transport_h4_register_callback(void (*package_callback)(uint8_t type, uint8_t *buf, size_t size));


#ifdef __cplusplus
}
#endif

#endif /* HCI_TRANSPORT_H4 */
