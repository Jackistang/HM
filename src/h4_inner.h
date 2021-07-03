#ifndef HCI_TRANSPORT_H4_INNER_
#define HCI_TRANSPORT_H4_INNER_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int _hci_transport_h4_pack(uint8_t *buf, uint16_t size);

extern int _receiver_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HCI_TRANSPORT_H4_INNER_ */
