#ifndef HCI_MIDDLEWARE_PORTING_BTSTACK_H
#define HCI_MIDDLEWARE_PORTING_BTSTACK_H

#include "hci_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Setup H4 instance with btstack_uart implementation
 * @param btstack_uart_block_driver to use
 */
const hci_transport_t * hci_transport_h4_instance_for_uart(const btstack_uart_t * uart_driver);

const btstack_chipset_t * btstack_chipset_hci_middleware_instance(void);

#ifdef __cplusplus
}
#endif

#endif /* HCI_MIDDLEWARE_PORTING_BTSTACK_H */
