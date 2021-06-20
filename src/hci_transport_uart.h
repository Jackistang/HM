#ifndef HCI_TRANSPORT_UART
#define HCI_TRANSPORT_UART

#ifdef __cplusplus
extern "C" {
#endif

struct hci_transport_uart_config {
    const char *device_name,
    uint32_t baudrate,
    bool flowcontrol,
};

int rt_hci_transport_uart_init(struct hci_transport_uart_config *config);

void rt_hci_transport_uart_send(uint8_t *buffer, uint16_t length);

void rt_hci_transport_uart_recv(uint8_t *buffer, uint16_t length)

#ifdef __cplusplus
}
#endif

#endif /* HCI_TRANSPORT_UART */
