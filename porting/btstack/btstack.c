#include "os_port.h"

#include "hci_transport_h4.h"
#include "hci_transport.h"
#include "btstack_debug.h"
#include "btstack_uart.h"

static struct rt_hci_transport_h4_config g_h4_config;
static void (*packet_handler)(int type, uint8_t *buf, size_t size);

static void hci_transport_h4_init(const void *transport_config)
{
    // check for hci_transport_config_uart_t
    if (!transport_config) {
        log_error("hci_transport_h4: no config!");
        return;
    }
    if (((hci_transport_config_t*)transport_config)->type != HCI_TRANSPORT_CONFIG_UART) {
        log_error("hci_transport_h4: config not of type != HCI_TRANSPORT_CONFIG_UART!");
        return;
    }

    // extract UART config from transport config
    hci_transport_config_uart_t * hci_transport_config_uart = (hci_transport_config_uart_t*) transport_config;

    g_h4_config.uart_config.baudrate    = hci_transport_config_uart->baudrate_init;
    g_h4_config.uart_config.flowcontrol = hci_transport_config_uart->flowcontrol;
    g_h4_config.uart_config.parity      = hci_transport_config_uart->parity;
    g_h4_config.uart_config.device_name = hci_transport_config_uart->device_name;
    g_h4_config.uart_config.databit     = OS_UART_DATABIT_8_BIT;
    g_h4_config.uart_config.stopbit     = OS_UART_STOPBIT_1_BIT;
    rt_hci_transport_h4_init(&g_h4_config);
}

static int hci_transport_h4_open(void)
{
    int err = rt_hci_transport_h4_open();
    if (err) {
        log_error("Open hci transport h4 fail err(%d)", err);
        return err;
    }

    return 0;
}

static int hci_transport_h4_close(void)
{
    rt_hci_transport_h4_close();

    return 0;
}

static void hci_transport_h4_register_packet_handler(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size))
{
    packet_handler = handler;
    rt_hci_transport_h4_register_packet_handler(handler);
}

static int hci_transport_h4_can_send_packet_now(uint8_t packet_type)
{
    //TODO Now is always true.
    return 1;
}

static int hci_transport_h4_send_packet(uint8_t packet_type, uint8_t *packet, int size)
{   
    rt_hci_transport_h4_send(packet_type, packet, size);

    static const uint8_t packet_sent_event[] = { HCI_EVENT_TRANSPORT_PACKET_SENT, 0}; 
    packet_handler(HCI_TRANSPORT_H4_EVENT, (uint8_t *) &packet_sent_event[0], sizeof(packet_sent_event));

    return 0;
}

static int hci_transport_h4_set_baudrate(uint32_t baudrate)
{
    return os_uart_set_baudrate(baudrate);
}


static const hci_transport_t hci_transport_h4 = {
    .name   =   "H4",
    .init   =   hci_transport_h4_init,
    .open   =   hci_transport_h4_open,
    .close  =   hci_transport_h4_close,
    .register_packet_handler    =   hci_transport_h4_register_packet_handler,
    .can_send_packet_now    =   hci_transport_h4_can_send_packet_now,
    .send_packet    =   hci_transport_h4_send_packet,
    .set_baudrate   =   hci_transport_h4_set_baudrate,
};


const hci_transport_t * hci_transport_h4_instance_for_uart(const btstack_uart_t * uart_driver)
{
    (void)(uart_driver);
    return &hci_transport_h4;
}


#include "btstack_chipset.h"

static void chipset_init(const void * transport_config)
{
    return ;
}

static btstack_chipset_result_t chipset_next_command(uint8_t * hci_cmd_buffer)
{
    return BTSTACK_CHIPSET_DONE;
}

static const btstack_chipset_t btstack_chipset_hci_middleware = {
    .name = "hci_middleware",
    .init = chipset_init,
    .next_command = chipset_next_command,
};

const btstack_chipset_t * btstack_chipset_hci_middleware_instance(void){
    return &btstack_chipset_hci_middleware;
}