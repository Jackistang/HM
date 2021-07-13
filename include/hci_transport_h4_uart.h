#ifndef HM_TRANS_H4_UART_H
#define HM_TRANS_H4_UART_H

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hci_trans_h4_uart_config {
    const char *device_name;
    int parity;
    int stopbit;
    int databit;
    uint32_t baudrate;
    int flowcontrol;
};

extern void hci_trans_h4_uart_init(struct hci_trans_h4_uart_config *config);
extern int hci_trans_h4_uart_open(void);
extern int hci_trans_h4_uart_close(void);


int hci_trans_h4_uart_send(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* HM_TRANS_H4_UART_H */
