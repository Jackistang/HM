# nimble

| pool        | blocks     | block size                       |
| ----------- | ---------- | -------------------------------- |
| command     | 1          | 260                              |
| event hi    | 8          | 70                               |
| event lo    | 8          | 70                               |
| acl         | 24         | 255 + 4                          |
| uart packet | 8 + 8 + 24 | sizeof (struct ble_hci_uart_pkt) |



command 内存池内有 1 个 260 字节大小的内存块。

event 有两个内存池，hi pool 里有 8 个 70 字节大小的内存块，lo pool 里也有 8 个 70 字节大小的内存块。



`hal_uart_tx_char()`：UART 驱动程序请求更多的数据发送。

`hal_uart_rx_char()`：UART 驱动程序报告数据。



```C
/* Generic command header */
struct ble_hci_cmd {
    uint16_t opcode;
    uint8_t  length;
    uint8_t  data[0];
} __attribute__((packed));

/* Generic event header */
struct ble_hci_ev {
    uint8_t opcode;
    uint8_t length;
    uint8_t  data[0];
} __attribute__((packed));
```

