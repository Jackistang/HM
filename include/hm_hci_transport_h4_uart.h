#ifndef HM_TRANS_H4_UART_H
#define HM_TRANS_H4_UART_H

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hci_trans_h4_uart_config {
    const char *device_name;    /* Uart device name, i.e. "uart1" */
    int parity;                 /* Parity,           i.e. PARITY_NONE */
    int stopbit;                /* Stop bit,         i.e. STOP_BITS_1 */
    int databit;                /* Data bit,         i.e. DATA_BITS_8 */
    uint32_t baudrate;          /* Bautdate,         i.e. BAUD_RATE_115200 */
    int flowcontrol;            /* Flow Control,     i.e. 1 */
};

/**
 * @brief Init H4 uart.
 * 
 * @param config H4 uart configure structure pointer.
 */
extern void hci_trans_h4_uart_init(struct hci_trans_h4_uart_config *config);

/**
 * @brief Open H4 uart.
 * 
 * @return int 
 * @retval  HM_SUCCESS          Open success.
 * @retval  -HM_THREAD_ERROR     Create h4 uart thread error.
 */
extern int hci_trans_h4_uart_open(void);

/**
 * @brief Close H4 uart.
 * 
 * @return int 
 * @retval  HM_SUCCESS      Close success.
 */
extern int hci_trans_h4_uart_close(void);

/**
 * @brief H4 uart send package data.
 * 
 * @param data Data to be sent.
 * @param len Data size needed to be sent.
 * 
 * @return int 
 * @retval  HM_SUCCESS           Send success.
 * @retval  -HM_UART_SEND_ERR    H4 uart send error.
 */
int hci_trans_h4_uart_send(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* HM_TRANS_H4_UART_H */
