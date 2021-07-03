#include "hci_transport_h4.h"
#include "os_port.h"
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>


#define ARRAY_SIZE(array)   ((sizeof(array) / sizeof(array[0])))
#define CU_ASSERT_ARRAY_EQUAL   CU_ASSERT_NSTRING_EQUAL

static struct os_sem sync_sem;

static int init_suite(void)
{
    if (os_sem_init(&sync_sem, 0))
        return -1;
    
    return 0;
}

static int clean_suite(void)
{
    return 0;
}

static uint8_t command1[] = { 0x03, 0x0C, 0x00 };
static uint8_t event1[] = { 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00 };
static void h4_package_callback(int type, uint8_t *buf, size_t size)
{
    CU_ASSERT_EQUAL(type, HCI_TRANSPORT_H4_EVENT);
    CU_ASSERT_EQUAL(size, ARRAY_SIZE(event1));
    CU_ASSERT_ARRAY_EQUAL(buf, event1, size);

    os_sem_release(&sync_sem);
}

static struct rt_hci_transport_h4_config config = {
    .package_callback = h4_package_callback,
};

/* For mock test */
#include "../src/h4_inner.h"

static void test_hci_transport_h4(void)
{
    rt_hci_transport_h4_init(&config);

    int n = rt_hci_transport_h4_send(HCI_TRANSPORT_H4_COMMAND, command1, ARRAY_SIZE(command1));
    CU_ASSERT_EQUAL(n, ARRAY_SIZE(command1));

    /* For mock test */
    uint8_t recv_buf[] = {0x04, 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00};
    int err = _hci_transport_h4_recv(recv_buf, ARRAY_SIZE(recv_buf));
    CU_ASSERT_EQUAL(err, 0);

    err = os_sem_take(&sync_sem, 1000);
    CU_ASSERT_EQUAL(err, 0);
}

int test_hci_transport_h4_init(void)
{
    CU_TestInfo test_array[] = {
        {"test hci transport h4", test_hci_transport_h4},
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