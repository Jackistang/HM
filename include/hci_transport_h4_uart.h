#ifndef HM_TRANS_H4_UART_H
#define HM_TRANS_H4_UART_H

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

int hci_trans_h4_uart_send(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* HM_TRANS_H4_UART_H */
