

BTStack 通过自定义 HCI 事件 `HCI_EVENT_TRANSPORT_PACKET_SENT` 通知 HCI 当前数据已发送完毕，可以再次发送。完整的事件字节流为：`0x04 0x6E 0x00`，参考代码：

> hci_transport_h4.c -> hci_transport_h4_block_send() : 328 行

在 hci.c 中可以看见该事件用于 BTStack 内部使用：

```C
case HCI_EVENT_TRANSPORT_PACKET_SENT:
    // release packet buffer only for asynchronous transport and if there are not further fragements
    if (hci_transport_synchronous()) {
        log_error("Synchronous HCI Transport shouldn't send HCI_EVENT_TRANSPORT_PACKET_SENT");
        return; // instead of break: to avoid re-entering hci_run()
    }
    hci_stack->acl_fragmentation_tx_active = 0;
    if (hci_stack->acl_fragmentation_total_size) break;
    hci_release_packet_buffer();
    break;
```

