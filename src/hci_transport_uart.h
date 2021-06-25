#ifndef HCI_TRANSPORT_UART_H
#define HCI_TRANSPORT_UART_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
    UART_PARITY_NONE,
};

enum {
    UART_STOPBIT_1_BIT,
    UART_STOPBIT_2_BIT,
};

enum {
    UART_DATABIT_5_BIT,
    UART_DATABIT_6_BIT,
    UART_DATABIT_7_BIT,
    UART_DATABIT_8_BIT,
};

struct hci_transport_uart_config {
    const char *device_name;
    int parity;
    int stopbit;
    int databit;
    uint32_t baudrate;
    bool flowcontrol;
};

extern int rt_hci_transport_uart_init(struct hci_transport_uart_config *config);

extern int rt_hci_transport_uart_send(uint8_t *buffer, uint16_t length);

extern int rt_hci_transport_uart_recv(uint8_t *buffer, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* HCI_TRANSPORT_UART_H */
