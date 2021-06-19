我测试用的 Zephyr Controller 开发板为 nRF52840DK ：

![](images/image-20210620004642191.png)

通过将 Zephyr 的例程 [Bluetooth: HCI UART](https://docs.zephyrproject.org/latest/samples/bluetooth/hci_uart/README.html) 编译下载，nRF52840 就成为了一个 Zephyr Controller，波特率为 1000000 的 H4 Transport 。可以在 BTstack 里已有的移植环境 [BTstack Port for POSIX Systems with Zephyr-based Controller](http://bluekitchen-gmbh.com/btstack/#ports/existing_ports/#sec:posix-h4-zephyrPort) 里直接运行。

Zephyr Controller 初始化流程比较简单，发送一条命令即可：

```
0x00 0x09 0xfc 0x00
```

初始化的 HCI 包为 `packages/zephyr.pklg` ，可以通过 Wireshark 打开。