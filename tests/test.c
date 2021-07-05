#include "os_port.h"
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>

#define ARRAY_SIZE(array)   ((sizeof(array) / sizeof(array[0])))
#define CU_ASSERT_ARRAY_EQUAL   CU_ASSERT_NSTRING_EQUAL


static struct os_uart_config config = {
    .device_name = "/dev/ttyACM0",
    .parity      = OS_UART_PARITY_NONE,
    .stopbit     = OS_UART_STOPBIT_1_BIT,
    .databit     = OS_UART_DATABIT_8_BIT,
    .baudrate    = 1000000,
    .flowcontrol = true,
};

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
static int init_suite(void)
{
    os_uart_init(&config);
    os_uart_open();
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
static int clean_suite(void)
{
    os_uart_close();
    return 0;
}


static void test_os_uart(void)
{
    uint8_t send_buf[] = {0x01, 0x03, 0x0C, 0x00};  // HCI Reset.
    int len = os_uart_send(send_buf, ARRAY_SIZE(send_buf));
    CU_ASSERT_EQUAL(len, ARRAY_SIZE(send_buf));

    uint8_t recv_buf[7];
    uint8_t recv_buf_expect[] = {0x04, 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00}; // Command complete event.
    len = os_uart_recv(recv_buf, ARRAY_SIZE(recv_buf));
    CU_ASSERT_EQUAL(len, ARRAY_SIZE(recv_buf_expect));
    CU_ASSERT_ARRAY_EQUAL(recv_buf, recv_buf_expect, ARRAY_SIZE(recv_buf_expect));
}


extern int test_hci_transport_h4_init(void);
/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

    CU_TestInfo test_array[] = {
        {"test hci transport uart", test_os_uart},
        // {"test slip receive frame", test_slip_receive_frame},
        CU_TEST_INFO_NULL,
    };

    CU_SuiteInfo suites[] = {
        {"suite1", init_suite, clean_suite, NULL, NULL, test_array},
        CU_SUITE_INFO_NULL,
    };

    if (CUE_SUCCESS != CU_register_suites(suites)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    test_hci_transport_h4_init();

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}