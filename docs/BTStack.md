

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



## BTStack 构建规则

最开始运行的 makefile 在 BTStack 官方移植好的平台里，例如 port -> posix-h4-zephyr -> Makefile 。

以下都基于 posix-h4-zephyr 环境。

最开始创建了变量 `BTSTACK_ROOT` 用于指定 btstack 根目录，

```makefile
# Makefile for posix-h4 based examples
BTSTACK_ROOT ?= ../..
```



```
VPATH 
```



main.c 文件里更改头文件接口。

![](images/image-20210705093425855.png)

Makefile 文件里去除掉 hci_transport_h4.c 文件，并 include 文件 ./HCI-Middleware/porting/btstack/Makefile.inc 即可。

![](images/image-20210705093532814.png)



`hci_transport_h4_block_read()`



每次 Host 到 Controller 的数据发送完成后，都需要手动返回一个 `6E 00` 的事件，该事件代表数据发送完成（`HCI_EVENT_TRANSPORT_PACKET_SENT`），参考下述代码，最核心的是释放了 HCI 发送数据包的缓冲区。

```C
static void event_handler(uint8_t *packet, uint16_t size)
{
    ......
    switch (hci_event_packet_get_type(packet)) {
            ......
                
            case HCI_EVENT_TRANSPORT_PACKET_SENT:
                // release packet buffer only for asynchronous transport and if there are not further fragements
                if (hci_transport_synchronous()) {
                    log_error("Synchronous HCI Transport shouldn't send HCI_EVENT_TRANSPORT_PACKET_SENT");
                    return; // instead of break: to avoid re-entering hci_run()
                }
                hci_stack->acl_fragmentation_tx_active = 0;
                if (hci_stack->acl_fragmentation_total_size) break;
                hci_release_packet_buffer();
            
            ......
    }
}
```



btstack HCI 数据接收流程

以 posix-h4-zephyr 移植环境为例，`btstack_uart_posix.c` 文件实现了 posix 环境下 btstack 定义的串口接口，`hci_transport_h4.c` 文件实现了 h4 的相关功能，下面先讨论 btstack 收发数据包的具体流程。

btstack 里的程序都处于一个大的 Loop 里，而且还允许里面的模块添加属于自己的小 Loop 。

以串口收发为例：

在串口打开函数 `btstack_uart_posix_open()` 里，有下述代码：

```C
// set up data_source
btstack_run_loop_set_data_source_fd(&transport_data_source, fd);
btstack_run_loop_set_data_source_handler(&transport_data_source, &hci_uart_posix_process);
btstack_run_loop_add_data_source(&transport_data_source);
```

这三行代码就为整个串口模块创建了一个 loop，然后这个 loop 就开始周期性地执行了。之后的串口发送函数 `btstack_uart_posix_send_block()` 和串口接收函数 `btstack_uart_posix_receive_block()` 只是简单地向 loop 里面添加了一个触发事件。

- 写操作的参数 `data` 是需要写入的数据缓冲区，`size` 是写入数据的大小。
- 读操作的参数 `buffer` 是读取数据的缓冲区，**`len` 是需要读取数据的大小（注意）**。

```C
static void btstack_uart_posix_send_block(const uint8_t *data, uint16_t size){
    // setup async write
    btstack_uart_block_write_bytes_data = data;
    btstack_uart_block_write_bytes_len  = size;
    btstack_run_loop_enable_data_source_callbacks(&transport_data_source, DATA_SOURCE_CALLBACK_WRITE);
}

static void btstack_uart_posix_receive_block(uint8_t *buffer, uint16_t len){
    btstack_uart_block_read_bytes_data = buffer;
    btstack_uart_block_read_bytes_len = len;
    btstack_run_loop_enable_data_source_callbacks(&transport_data_source, DATA_SOURCE_CALLBACK_READ);
}
```

现在来看看 loop 里执行的代码：

```C
static void hci_uart_posix_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type) {
    if (ds->source.fd < 0) return;
    switch (callback_type){
        case DATA_SOURCE_CALLBACK_READ:
            {
                btstack_uart_block_posix_process_read(ds);
            }
            break;
        case DATA_SOURCE_CALLBACK_WRITE:
            {
                btstack_uart_block_posix_process_write(ds);
            }
            break;
        default:
            break;
    }
}
```

可以看见，根据 loop 的触发事件不同，我们进行相应的读写操作。

- 写操作比较简单，就是不断地向串口写入数据，如果一次没写完，就再触发一个写事件，在下次事件里继续写，直至写完，当然数据写完后还需要通知上层 HCI Transport，这是通过  `btstack_uart_posix_set_block_sent()` 函数注册的。
- 读操作也差不多，不断地从串口读取数据，直到所需的数据大小被读完，然后通知上层收到一个 HCI 数据包，这是通过 `btstack_uart_posix_set_block_received()` 函数注册的。

关键就在于这两个注册的函数，在 `hci_transport_h4.c` 文件里注册的是这两个函数 `hci_transport_h4_block_sent()` 和  `hci_transport_h4_block_read()` 。

**在 sent 函数里，通过 `packet_handler()` 函数给上层 HCI 传输了一个自定义事件 `6E 00`，代表 HCI 数据由 Host 成功发送给 Controller 。**

```C
static void hci_transport_h4_block_sent(void){

    static const uint8_t packet_sent_event[] = { HCI_EVENT_TRANSPORT_PACKET_SENT, 0};

    switch (tx_state){
        case TX_W4_PACKET_SENT:
            // packet fully sent, reset state
            tx_state = TX_IDLE;
            // notify upper stack that it can send again
            packet_handler(HCI_EVENT_PACKET, (uint8_t *) &packet_sent_event[0], sizeof(packet_sent_event));
            break;
        default:
            break;
    }
}
```

在 read 函数里实现了一个收包的状态机，并且每次进入该函数，在最末尾都会调用 `hci_transport_h4_trigger_next_read()` 接口触发下一次的读操作，最终会激活串口模块里的读取事件，感兴趣地可以参考下述代码。

```C
static void hci_transport_h4_block_read(void){

    read_pos += bytes_to_read;

    switch (h4_state) {
        case H4_W4_PACKET_TYPE:
            switch (hci_packet[0]){
                case HCI_EVENT_PACKET:
                    bytes_to_read = HCI_EVENT_HEADER_SIZE;
                    h4_state = H4_W4_EVENT_HEADER;
                    break;
                case HCI_ACL_DATA_PACKET:
                    bytes_to_read = HCI_ACL_HEADER_SIZE;
                    h4_state = H4_W4_ACL_HEADER;
                    break;
                case HCI_SCO_DATA_PACKET:
                    bytes_to_read = HCI_SCO_HEADER_SIZE;
                    h4_state = H4_W4_SCO_HEADER;
                    break;
                default:
                    log_error("hci_transport_h4: invalid packet type 0x%02x", hci_packet[0]);
                    hci_transport_h4_reset_statemachine();
                    break;
            }
            break;
            
        case H4_W4_EVENT_HEADER:
            bytes_to_read = hci_packet[2];
            // check Event length
            if (bytes_to_read > (HCI_INCOMING_PACKET_BUFFER_SIZE - HCI_EVENT_HEADER_SIZE)){
                log_error("hci_transport_h4: invalid Event len %d - only space for %u", bytes_to_read, HCI_INCOMING_PACKET_BUFFER_SIZE - HCI_EVENT_HEADER_SIZE);
                hci_transport_h4_reset_statemachine();
                break;
            }
            h4_state = H4_W4_PAYLOAD;
            break;
            
        case H4_W4_ACL_HEADER:
            bytes_to_read = little_endian_read_16( hci_packet, 3);
            // check ACL length
            if (bytes_to_read > (HCI_INCOMING_PACKET_BUFFER_SIZE - HCI_ACL_HEADER_SIZE)){
                log_error("hci_transport_h4: invalid ACL payload len %d - only space for %u", bytes_to_read, HCI_INCOMING_PACKET_BUFFER_SIZE - HCI_ACL_HEADER_SIZE);
                hci_transport_h4_reset_statemachine();
                break;
            }
            h4_state = H4_W4_PAYLOAD;
            break;
            
        case H4_W4_SCO_HEADER:
            bytes_to_read = hci_packet[3];
            // check SCO length
            if (bytes_to_read > (HCI_INCOMING_PACKET_BUFFER_SIZE - HCI_SCO_HEADER_SIZE)){
                log_error("hci_transport_h4: invalid SCO payload len %d - only space for %u", bytes_to_read, HCI_INCOMING_PACKET_BUFFER_SIZE - HCI_SCO_HEADER_SIZE);
                hci_transport_h4_reset_statemachine();
                break;
            }
            h4_state = H4_W4_PAYLOAD;
            break;

        case H4_W4_PAYLOAD:
            hci_transport_h4_packet_complete();
            break;

        case H4_OFF:
            bytes_to_read = 0;
            break;
        default:
            btstack_assert(false);
            break; 
    }
    // forward packet if payload size == 0
    if (h4_state == H4_W4_PAYLOAD && bytes_to_read == 0u) {
        hci_transport_h4_packet_complete();
    }

    if (h4_state != H4_OFF) {
        hci_transport_h4_trigger_next_read();
    }
}
```



btstack 芯片初始化

