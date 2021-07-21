Chipset 注册结构体：

名称，mid, pid, proto 协议，初始波特率，工作波特率，bdaddr，init,  post 。

工作波特率：有些 Chipset 支持更改波特率

bdaddr：有些 Chipset 支持设置静态地址。





需要将 HCI_Ttransport_h4 封装成哪些接口供 Chipset 使用？

```C
/* Send a command */
int chip_hci_cmd_send(uint8_t *buf, uint16_t size);

/* Wait until read a event. */
int chip_hci_event_read(uint8_t *buf, uint16_t size);
```

