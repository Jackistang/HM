#include "hci_transport_h4.h"
#include "h4_inner.h"
#include "chipset.h"
#include <string.h>
#include <assert.h>

static void (*g_package_cb)(int type, uint8_t *buf, uint16_t size);

void rt_hci_transport_h4_init(struct rt_hci_transport_h4_config *config)
{
    assert(config);

    os_uart_init(&config->uart_config);

    return ;
}

int rt_hci_transport_h4_open(void)
{
    int err;
    if ((err = os_uart_open()))
        return err;
    
    _receiver_init();

    rt_chipset_init_start();

    return 0;
}

int rt_hci_transport_h4_close(void)
{
    int err;
    if ((err = os_uart_close()))
        return err;
    
    return 0;
}

int rt_hci_transport_h4_send(int type, uint8_t *buf, size_t size)
{
    uint8_t h4_buf[size+1];
    h4_buf[0] = type;
    memcpy(h4_buf+1, buf, size);

    return os_uart_send(h4_buf, size+1) -1 ;    // Send buffer size, not include type.
}

struct hci_pkg_format {
    uint8_t header;
    uint8_t length;
};

/* Array index is received package type. */
static struct hci_pkg_format g_pkg_fmt[] = {
    {0, 0},
    {0, 0},
    {2, 2}, /* ACL: handle, PB PC flag is 2 bytes, Data total length is 2 bytes. */
    {2, 1}, /* SCO: handle, packet status flag, RFU is 2 bytes, Data total length is 1 bytes. */
    {1, 1}, /* Event: event code is 1 bytes, parameter total length is 1 bytes. */
    {2, 2}, /* ISO: handle, PB TS flag, RFU is 2 bytes, Data total length is 2 bytes (MSB 14bits) */
};

// TODO 接收数据待优化，利用状态机实现。
static uint8_t recv_buffer[512];
static uint16_t recv_index;
int _hci_transport_h4_pack(uint8_t *buf, uint16_t buf_len)
{
    assert(buf);
    assert(buf_len > 0);

    /* Buffer is full, need increase buffer size. */
    if (recv_index + buf_len > ARRAY_SIZE(recv_buffer))
        return -1;
    
    memcpy(recv_buffer+recv_index, buf, buf_len);
    recv_index += buf_len;

    size_t recv_hci_size = recv_index - 1;  //  H4 type is 1 byte.
    uint8_t idx = recv_buffer[0];

    if (recv_hci_size >= (g_pkg_fmt[idx].header + g_pkg_fmt[idx].length)) {
        uint8_t *ptr = recv_buffer + 1 + g_pkg_fmt[idx].header;
        uint16_t data_length = *ptr;

        if (g_pkg_fmt[idx].length > 1)
            data_length = data_length | ((uint16_t)*(ptr+1) << 8);

        size_t expect_hci_size= g_pkg_fmt[idx].header + g_pkg_fmt[idx].length + data_length;
        /* Not one complete message stored in ringbuffer. */
        if (recv_hci_size < expect_hci_size)
            return 0;
        
        //TODO 目前无法处理一个包内含有两条消息的情况。
        g_package_cb(recv_buffer[0], recv_buffer+1, expect_hci_size);

        /* Reset receive buffer, copy data from end pointer to start pointer. */
        if (recv_hci_size > expect_hci_size) {
            void *start = recv_buffer;
            void *end = recv_buffer + 1 + expect_hci_size;
            uint16_t moving_size = recv_index - (1 + expect_hci_size);
            recv_index = moving_size;   // Update receive buffer size.
            memcpy(start, end, moving_size);
        } else {
            recv_index = 0;
        }
    }

    return 0;
}

void rt_hci_transport_h4_register_packet_handler(void (*handler)(int packet_type, uint8_t *packet, uint16_t size))
{
    g_package_cb = handler;
}
