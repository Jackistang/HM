#include <rtthread.h>
#include "utest.h"
#include "hci_transport_h4.h"

static struct rt_semaphore sync_sem;

static rt_err_t utest_tc_init(void)
{
    rt_sem_init(&sync_sem, "sync_sem", 0, RT_IPC_FLAG_PRIO);
    hci_trans_h4_init();

    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    rt_sem_detach(&sync_sem);
    
    return RT_EOK;
}



typedef struct {
    int expected_package_type;
    uint16_t length;
    uint8_t buf[10];
} hci_trans_h4_recv_byte_test_t ;

uint8_t test_recv_byte_table_index = 0;
hci_trans_h4_recv_byte_test_t test_recv_byte_table[] = {
    {HCI_TRANS_H4_TYPE_CMD, 4, {0x01, 0x3c, 0x0f, 0x00}},
    {HCI_TRANS_H4_TYPE_CMD, 6, {0x01, 0x3c, 0x0f, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ACL, 5, {0x02, 0x01, 0x00, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_ACL, 6, {0x02, 0x01, 0x00, 0x01, 0x00, 0x06}},
    {HCI_TRANS_H4_TYPE_SCO, 4, {0x03, 0x01, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_SCO, 6, {0x03, 0x01, 0x00, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_EVT, 3, {0x04, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_EVT, 5, {0x04, 0x80, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ISO, 5, {0x05, 0x01, 0x00, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_ISO, 7, {0x05, 0x01, 0x00, 0x02, 0x00, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ISO, 7, {0x05, 0x01, 0x00, 0x02, 0xc0, 0xfc, 0xfc}},
};

void test_hci_trans_h4_recv_byte_callback(uint8_t package_type, uint8_t *package, uint16_t size)
{
    hci_trans_h4_recv_byte_test_t *p = &test_recv_byte_table[test_recv_byte_table_index];
    uassert_int_equal(package_type, p->expected_package_type);
    uassert_int_equal(size, p->length - 1);
    uassert_buf_equal(package, &p->buf[1], size);

    rt_sem_release(&sync_sem);
}

#define RECV_BYTE_TEST(n) do {    \
    test_recv_byte_table_index = n; \
    p = &test_recv_byte_table[test_recv_byte_table_index];  \
    for (uint16_t i = 0; i < p->length; i++) {  \
        err = hci_trans_h4_recv_byte(p->buf[i]);    \
        uassert_int_equal(err, HM_SUCCESS); \
    }   \
    err = rt_sem_take(&sync_sem, 10);   \
    uassert_int_equal(err, RT_EOK); \
} while (0)

static void test_hci_trans_h4_recv_byte(void)
{
    int err = 0;
    hci_trans_h4_recv_byte_test_t *p = RT_NULL;
    hci_trans_h4_open();

    hci_trans_h4_register_packge_callback(test_hci_trans_h4_recv_byte_callback);

    // // Command
    // RECV_BYTE_TEST(0);
    // RECV_BYTE_TEST(1);

    // // SCO
    // RECV_BYTE_TEST(4);
    // RECV_BYTE_TEST(5);

    // Event
    RECV_BYTE_TEST(6);
    RECV_BYTE_TEST(7);

    // ACL
    RECV_BYTE_TEST(2);
    RECV_BYTE_TEST(3);

    // // ISO
    // RECV_BYTE_TEST(8);
    // RECV_BYTE_TEST(9);
    // RECV_BYTE_TEST(10);

    hci_trans_h4_close();
}

typedef struct {
    int expected_package_type;
    uint16_t length;
    uint8_t buf[20];
} hci_trans_h4_send_test_t ;

uint8_t test_send_table_index = 0;
hci_trans_h4_send_test_t test_send_table[] = {
    {HCI_TRANS_H4_TYPE_CMD, 4, {0x01, 0x3c, 0x0f, 0x00}},
    {HCI_TRANS_H4_TYPE_CMD, 6, {0x01, 0x3c, 0x0f, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ACL, 5, {0x02, 0x01, 0x00, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_ACL, 6, {0x02, 0x01, 0x00, 0x01, 0x00, 0x06}},
    {HCI_TRANS_H4_TYPE_SCO, 4, {0x03, 0x01, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_SCO, 6, {0x03, 0x01, 0x00, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ISO, 5, {0x05, 0x01, 0x00, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_ISO, 7, {0x05, 0x01, 0x00, 0x02, 0x00, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ISO, 7, {0x05, 0x01, 0x00, 0x02, 0xc0, 0xfc, 0xfc}},
};

/* Mock function */
int hci_trans_h4_uart_send(uint8_t *data, uint16_t len)
{
    hci_trans_h4_send_test_t *p = &test_send_table[test_send_table_index];

    uassert_not_null(data);
    uassert_int_equal(len, p->length);
    uassert_buf_equal(data, p->buf, len);

    return len;
}

#define SEND_TEST(n) do {    \
    test_send_table_index = n; \
    p = &test_send_table[test_send_table_index];  \
    data = hci_trans_h4_send_alloc(p->expected_package_type);   \
    uassert_not_null(data); \
    rt_memcpy(data, p->buf+1, p->length-1); \
    err = hci_trans_h4_send(p->expected_package_type, data);   \
    uassert_int_equal(err, HM_SUCCESS); \
    hci_trans_h4_send_free(data);   \
    data = NULL;    \
} while (0)

static void test_hci_trans_h4_send(void)
{
    int err = 0;
    hci_trans_h4_send_test_t *p = RT_NULL;
    uint8_t *data;
    hci_trans_h4_open();

    // Command
    SEND_TEST(0);
    SEND_TEST(1);

    // ACL
    SEND_TEST(2);
    SEND_TEST(3);

    // // SCO
    // SEND_TEST(4);
    // SEND_TEST(5);

    // // ISO
    // SEND_TEST(6);
    // SEND_TEST(7);
    // SEND_TEST(8);

    hci_trans_h4_close();
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_hci_trans_h4_recv_byte);
    UTEST_UNIT_RUN(test_hci_trans_h4_send);
}
UTEST_TC_EXPORT(testcase, "hm.hci_transport_h4", utest_tc_init, utest_tc_cleanup, 10);