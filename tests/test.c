#include "hci_transport_uart.h"
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>

#define ARRAY_SIZE(array)   ((sizeof(array) / sizeof(array[0])))
#define CU_ASSERT_ARRAY_EQUAL   CU_ASSERT_NSTRING_EQUAL


struct hci_transport_uart_config config = {
    .device_name = "/dev/ttyUSB0",
    .parity      = UART_PARITY_NONE,
    .stopbit     = UART_STOPBIT_1_BIT,
    .databit     = UART_DATABIT_8_BIT,
    .baudrate    = 115200,
    .flowcontrol = true,
};

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite(void)
{
    rt_hci_transport_uart_init(&config);
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite(void)
{
    return 0;
}


void test_rt_hci_transport_uart(void)
{
    uint8_t send_buf[] = {0x1, 0x2, 0x3};
    int len = rt_hci_transport_uart_send(send_buf, ARRAY_SIZE(send_buf));
    CU_ASSERT_EQUAL(len, 3);

    uint8_t recv_buf[6];
    len = rt_hci_transport_uart_recv(recv_buf, ARRAY_SIZE(recv_buf));
    CU_ASSERT_EQUAL(len, 3);
    CU_ASSERT_ARRAY_EQUAL(recv_buf, send_buf, len);
}


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
        {"test hci transport uart", test_rt_hci_transport_uart},
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

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}