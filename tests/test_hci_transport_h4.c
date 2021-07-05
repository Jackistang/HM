#include "hci_transport_h4.h"
#include "../src/h4_inner.h"
#include "os_port.h"
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>


#define ARRAY_SIZE(array)   ((sizeof(array) / sizeof(array[0])))
#define CU_ASSERT_ARRAY_EQUAL   CU_ASSERT_NSTRING_EQUAL

static struct os_sem sync_sem;

static struct rt_hci_transport_h4_config config = {
    .uart_config = {
        .device_name = "/dev/ttyACM0",
        .parity      = OS_UART_PARITY_NONE,
        .stopbit     = OS_UART_STOPBIT_1_BIT,
        .databit     = OS_UART_DATABIT_8_BIT,
        .baudrate    = 1000000,
        .flowcontrol = true,
    },
};

static int init_suite(void)
{
    if (os_sem_init(&sync_sem, 0))
        return -1;
    

    rt_hci_transport_h4_init(&config);
    rt_hci_transport_h4_open();

    return 0;
}

static int clean_suite(void)
{
    rt_hci_transport_h4_close();

    return 0;
}

static uint8_t command1[] = { 0x03, 0x0C, 0x00 };
static uint8_t event1[] = { 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00 };
static void h4_package_callback(int type, uint8_t *buf, uint16_t size)
{
    CU_ASSERT_EQUAL(type, HCI_TRANSPORT_H4_EVENT);
    CU_ASSERT_EQUAL(size, ARRAY_SIZE(event1));
    CU_ASSERT_ARRAY_EQUAL(buf, event1, size);

    os_sem_release(&sync_sem);
}


/* For mock test */
#include "../src/h4_inner.h"

static void test_hci_transport_h4(void)
{
    rt_hci_transport_h4_register_packet_handler(h4_package_callback);

    int n = rt_hci_transport_h4_send(HCI_TRANSPORT_H4_COMMAND, command1, ARRAY_SIZE(command1));
    CU_ASSERT_EQUAL(n, ARRAY_SIZE(command1));
    /* Real hardware test. */
    int err = os_sem_take(&sync_sem, 1000);
    CU_ASSERT_EQUAL(err, 0);

    /* For mock test */
    uint8_t recv_buf[] = {0x04, 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00};
    err = _hci_transport_h4_pack(recv_buf, ARRAY_SIZE(recv_buf));
    CU_ASSERT_EQUAL(err, 0);
    err = os_sem_take(&sync_sem, 1000);
    CU_ASSERT_EQUAL(err, 0);

    //测试 _hci_transport_h4_recv 接口分两段接收一个包。
    for (int i = 1; i < ARRAY_SIZE(recv_buf); i+=3) {
        err = _hci_transport_h4_pack(recv_buf, i);
        CU_ASSERT_EQUAL(err, 0);
        err = _hci_transport_h4_pack(recv_buf+i, ARRAY_SIZE(recv_buf)-i);
        err = os_sem_take(&sync_sem, 1000);
        CU_ASSERT_EQUAL(err, 0);
    }
}

static uint8_t acl_data1[] = {0x02, 0x00, 0x01, 0x02, 0x00, 0x01, 0x01};
static void h4_acl_callback(int type, uint8_t *buf, uint16_t size)
{
    CU_ASSERT_EQUAL(type, HCI_TRANSPORT_H4_ACL);
    CU_ASSERT_EQUAL(size, ARRAY_SIZE(acl_data1)-1);
    CU_ASSERT_ARRAY_EQUAL(buf, acl_data1+1, size);

    os_sem_release(&sync_sem);
}

static void test_hci_transport_h4_recv_acl(void)
{
    rt_hci_transport_h4_register_packet_handler(h4_acl_callback);

    int err;
    err = _hci_transport_h4_pack(acl_data1, ARRAY_SIZE(acl_data1));
    CU_ASSERT_EQUAL(err, 0);
    err = os_sem_take(&sync_sem, 1000);
    CU_ASSERT_EQUAL(err, 0);
}


int test_hci_transport_h4_init(void)
{
    CU_TestInfo test_array[] = {
        {"test hci transport h4", test_hci_transport_h4},
        {"test hci transport h4 recv acl", test_hci_transport_h4_recv_acl},
        // {"test slip receive frame", test_slip_receive_frame},
        CU_TEST_INFO_NULL,
    };

    CU_SuiteInfo suites[] = {
        {"suite2", init_suite, clean_suite, NULL, NULL, test_array},
        CU_SUITE_INFO_NULL,
    };

    if (CUE_SUCCESS != CU_register_suites(suites)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    return CU_get_error();
}