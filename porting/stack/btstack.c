#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"
#include "hm_hci_transport_h4_uart.h"
#include "btstack_debug.h"
#include "btstack_config.h"

#include <rtthread.h>

#define DATA_SOURCE_CALLBACK_READ   (1 << 0)

static struct hci_trans_h4_config h4_config = {
    .uart_config = {
        .device_name = "uart1",
        .databit     = DATA_BITS_8,
        .stopbit     = STOP_BITS_1,
        .parity      = PARITY_NONE,
        .baudrate    = BAUD_RATE_115200,
        .flowcontrol = 1,
    },
}

typedef struct btstack_port_sync_read {
    uint8_t *pkg;   /* package pointer */
    uint16_t size;  /* package size */
} btstack_port_sync_read_t;

static void (*packet_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size)

/* No effect if port doensn't have file descriptors */
static int fd = 10;
static btstack_data_source_t btstack_port_data_source;

static void btstack_port_process_read(btstack_data_source_t *ds) {

}

static void btstack_port_process(btstack_data_source_t *ds, btstack_data_source_callback_type_t callback_type) 
{
    if (ds->source.fd < 0) return;
    switch (callback_type){
    case DATA_SOURCE_CALLBACK_READ:
        btstack_port_process_read(ds);
        break;
    default:
        break;
    }
}

/* This function is in another thread. */
void hm_hci_trans_h4_package_callback(uint8_t pkg_type, uint8_t *pkg, uint16_t size)
{
    // btstack_port_sync_read_t *sync_read = rt_malloc(btstack_port_sync_read_t);
    // RT_ASSERT(sync_read);

    //TODO Add mailbox implement.
}

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
    h4_config.uart_config.baudrate    = hci_transport_config_uart->baudrate_init;
    h4_config.uart_config.flowcontrol = hci_transport_config_uart->flowcontrol;
    h4_config.uart_config.parity      = hci_transport_config_uart->parity;
    h4_config.uart_config.device_name = hci_transport_config_uart->device_name;

    hci_trans_h4_init(&h4_config);

    hci_trans_h4_register_callback(hm_hci_trans_h4_package_callback);
}

static int hci_transport_h4_open(void)
{
    int err = hci_trans_h4_open();
    if (err) {
        log_error("hci_transport_h4_open error");
        return -1;
    }

    // set up data_source
    btstack_run_loop_set_data_source_fd(&btstack_port_data_source, fd);
    btstack_run_loop_set_data_source_handler(&btstack_port_data_source, &btstack_port_process);
    btstack_run_loop_add_data_source(&btstack_port_data_source);

    return 0;
}

static int hci_transport_h4_close(void)
{
    int err = hci_trans_h4_close();
    if (err) {
        log_error("hci_transport_h4_close error");
        return -1;
    }

    // first remove run loop handler
    btstack_run_loop_remove_data_source(&btstack_port_data_source);
    btstack_port_data_source.source.fd = -1;

    return 0;
}

static void hci_transport_h4_register_packet_handler(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size))
{
    packet_handler = handler;
}

static int hci_transport_h4_can_send_now(uint8_t packet_type)
{
    /* Always return 1 for sending while block until send complete */
    return 1;
}

static int hci_transport_h4_send_packet(uint8_t packet_type, uint8_t *packet, int size)
{
    // store packet type before actual data and increase size
    size++;
    packet--;
    *packet = packet_type;

    int err = hci_trans_h4_uart_send(packet, size);

    // Notify btstack to release packet buffer.
    static const uint8_t packet_sent_event[] = { HCI_EVENT_TRANSPORT_PACKET_SENT, 0};
    packet_handler(HCI_EVENT_PACKET, (uint8_t *) &packet_sent_event[0], sizeof(packet_sent_event));

    // Enable run loop for receive data.
    btstack_run_loop_enable_data_source_callbacks(&transport_data_source, DATA_SOURCE_CALLBACK_READ);

    return err;
}

static int hci_transport_h4_set_baudrate(uint32_t baudrate)
{
    log_error("hci_transport_h4_set_baudrate: not support now");

    return -1;
}


static const hci_transport_t hci_transport_h4 = {
        /* const char * name; */                                        "H4",
        /* void   (*init) (const void *transport_config); */            &hci_transport_h4_init,
        /* int    (*open)(void); */                                     &hci_transport_h4_open,
        /* int    (*close)(void); */                                    &hci_transport_h4_close,
        /* void   (*register_packet_handler)(void (*handler)(...); */   &hci_transport_h4_register_packet_handler,
        /* int    (*can_send_packet_now)(uint8_t packet_type); */       &hci_transport_h4_can_send_now,
        /* int    (*send_packet)(...); */                               &hci_transport_h4_send_packet,
        /* int    (*set_baudrate)(uint32_t baudrate); */                &hci_transport_h4_set_baudrate,
        /* void   (*reset_link)(void); */                               NULL,
        /* void   (*set_sco_config)(uint16_t voice_setting, int num_connections); */ NULL,
};
