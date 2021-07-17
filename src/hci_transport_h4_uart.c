#include "hm_config.h"
#include "hm_hci_transport_h4.h"
#include "hm_hci_transport_h4_uart.h"
#include "hm_error.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define H4_UART_STACK_SIZE     512
#define H4_UART_PRIORITY       10
#define H4_UART_TICKS          10

struct h4_uart_config{
    const char *name;
    int flowcontrol;        // RT-Thread not support CTS/RTS flowcontrol now, default is true.
    struct serial_configure rt_config;
};

static struct h4_uart_config uart_config = {
    .rt_config = RT_SERIAL_CONFIG_DEFAULT,
};
static rt_thread_t h4_uart_tid;
static rt_device_t h4_uart;

void hci_trans_h4_uart_init(struct hci_trans_h4_uart_config *config)
{
    RT_ASSERT(config);

    uart_config.name        = config->device_name;
    uart_config.flowcontrol = config->flowcontrol;

    uart_config.rt_config.baud_rate = config->baudrate;
    uart_config.rt_config.data_bits = config->databit;
    uart_config.rt_config.stop_bits = config->stopbit;
    uart_config.rt_config.parity    = config->parity;

    /* PD11 - CTS, PD12 - RTS */
    rt_pin_mode(59, PIN_MODE_OUTPUT);
    rt_pin_mode(60, PIN_MODE_OUTPUT);
    rt_pin_write(59, PIN_HIGH);
    rt_pin_write(60, PIN_HIGH);
}

static void h4_uart_thread(void *args)
{
    uint8_t buf[32];
    uint16_t read = 0;

    while (1) {
        read = rt_device_read(h4_uart, 0, buf, ARRAY_SIZE(buf));
        for (uint16_t i = 0; i < read; i++) {
            hci_trans_h4_recv_byte(buf[i]);
        }

        rt_thread_mdelay(1);
    };
}


int hci_trans_h4_uart_open(void)
{
    RT_ASSERT(uart_config.name);

    h4_uart = rt_device_find(uart_config.name);
    RT_ASSERT(h4_uart);

    rt_err_t err;

    if ((err = rt_device_open(h4_uart, RT_DEVICE_FLAG_INT_RX))) {
        rt_kprintf("Open h4_uart error\n");
        return HM_NOT_OPEN;
    }
    if ((err = rt_device_control(h4_uart, RT_DEVICE_CTRL_CONFIG, &uart_config.rt_config))) {
        rt_kprintf("Control h4_uart error\n");
        return HM_NOT_OPEN;
    }

    rt_thread_mdelay(100);

    h4_uart_tid = rt_thread_create("h4.uart", h4_uart_thread, RT_NULL, 
                        H4_UART_STACK_SIZE, H4_UART_PRIORITY, H4_UART_TICKS);
    if (h4_uart_tid == RT_NULL)
        return HM_THREAD_ERROR;
    
    rt_thread_startup(h4_uart_tid);

    return HM_SUCCESS;
}

int hci_trans_h4_uart_close(void)
{
    rt_device_close(h4_uart);
    h4_uart = RT_NULL;
    
    rt_thread_delete(h4_uart_tid);
    h4_uart_tid = RT_NULL;

    return HM_SUCCESS;
}

int hci_trans_h4_uart_send(uint8_t *data, uint16_t len)
{
    RT_ASSERT(data);
    RT_ASSERT(len > 0);

    uint16_t remain = len;
    uint16_t write;

    while (remain > 0) {
        write = rt_device_write(h4_uart, 0, data, remain);
        if (write == 0)
            return HM_UART_SEND_ERR;

        remain -= write;
    }

    return HM_SUCCESS;
}
