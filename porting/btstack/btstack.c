#include "os_port.h"
#include "hci_transport_h4.h"
#include "hci_transport.h"
#include "btstack_debug.h"

// UART Config
static btstack_uart_config_t uart_config;

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
    uart_config.baudrate    = hci_transport_config_uart->baudrate_init;
    uart_config.flowcontrol = hci_transport_config_uart->flowcontrol;
    uart_config.parity      = hci_transport_config_uart->parity;
    uart_config.device_name = hci_transport_config_uart->device_name;
}

static int hci_transport_h4_open(void)
{
    struct os_uart_config config = {
        .device_name    =   uart_config.device_name,
        .baudrate       =   uart_config.baudrate,
        .flowcontrol    =   uart_config.flowcontrol,
        .parity         =   uart_config.parity,

        //  default
        .databit        =   OS_UART_DATABIT_8_BIT,
        .stopbit        =   OS_UART_STOPBIT_1_BIT,
    };
    
    int err = os_uart_init(&config);
    if (err) {
        log_error("Open uart %s error!", config.device_name);
        return -1;
    }
}

static int hci_transport_h4_close(void)
{
    os_uart_deinit();
}

static void hci_transport_h4_register_packet_handler(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size))
{
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