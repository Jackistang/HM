#include <rtthread.h>
#include <rtdevice.h>
#include "utest.h"
#include "hm_hci_transport_h4.h"

static struct rt_semaphore sync_sem;

static rt_err_t utest_tc_init(void)
{
    rt_sem_init(&sync_sem, "sync_sem", 0, RT_IPC_FLAG_PRIO);

    hci_trans_h4_open();

    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    rt_sem_detach(&sync_sem);
    
    hci_trans_h4_close();

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

static void test_hci_trans_h4_recv_byte_callback(uint8_t package_type, uint8_t *package, uint16_t size)
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

    hci_trans_h4_register_callback(test_hci_trans_h4_recv_byte_callback);

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

    hci_trans_h4_remove_callback(test_hci_trans_h4_recv_byte_callback);
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
//RT_WEAK int hci_trans_h4_uart_send(uint8_t *data, uint16_t len)
//{
//    hci_trans_h4_send_test_t *p = &test_send_table[test_send_table_index];
//
//    uassert_not_null(data);
//    uassert_int_equal(len, p->length);
//    uassert_buf_equal(data, p->buf, len);
//
//    return len;
//}

#define SEND_MOCK_TEST(n) do {    \
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

static void mock_test_hci_trans_h4_send(void)
{
    int err = 0;
    hci_trans_h4_send_test_t *p = RT_NULL;
    uint8_t *data;
    hci_trans_h4_open();

    // Command
    SEND_MOCK_TEST(0);
    SEND_MOCK_TEST(1);

    // ACL
    SEND_MOCK_TEST(2);
    SEND_MOCK_TEST(3);

    // // SCO
    // SEND_MOCK_TEST(4);
    // SEND_MOCK_TEST(5);

    // // ISO
    // SEND_MOCK_TEST(6);
    // SEND_MOCK_TEST(7);
    // SEND_MOCK_TEST(8);

    hci_trans_h4_close();
}

struct test_h4_send_object {
    uint8_t send_pkg_type;
    uint8_t *send;
    uint16_t send_len;
    uint8_t expected_recv_pkg_type;
    uint8_t *expected_recv;
    uint16_t expected_recv_len;
};
struct test_h4_send_object h4_send_object;

static uint8_t h4_send1[] = { 0x03, 0x0C, 0x00 };
static uint8_t h4_recv1[] = { 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00 };

/* Used for CSR8311 test */
static uint8_t h4_send2[] = {0x01,0x10,0x00};
static uint8_t h4_recv2[] = {0x0E,0x0C,0x01,0x01,0x10,0x00,0x06,0x31,0x20,0x06,0x0A,0x00,0x31,0x20};
static uint8_t h4_send3[] = {0x14,0x0C,0x00};
static uint8_t h4_recv3[] = {0x0E,0xFC,0x01,0x14,0x0C,0x00,0x43,0x53,0x52,0x20,0x2D,0x20,0x62,0x63,0x37,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00};

#define SEND_TEST(send_type, recv_type, idx)    do {    \
    h4_send_object.send_pkg_type = send_type;   \
    h4_send_object.send = h4_send##idx; \
    h4_send_object.send_len = ARRAY_SIZE(h4_send##idx); \
    h4_send_object.expected_recv_pkg_type = recv_type;  \
    h4_send_object.expected_recv = h4_recv##idx;    \
    h4_send_object.expected_recv_len = ARRAY_SIZE(h4_recv##idx);    \
    \
    p = (uint8_t *)hci_trans_h4_send_alloc(h4_send_object.send_pkg_type);   \
    uassert_not_null(p);    \
    rt_memcpy(p, h4_send_object.send, h4_send_object.send_len); \
    \
    err = hci_trans_h4_send(h4_send_object.send_pkg_type, p);   \
    uassert_int_equal(err, HM_SUCCESS); \
    err = rt_sem_take(&sync_sem, 2000);   \
    uassert_int_equal(err, RT_EOK); \
    \
    hci_trans_h4_send_free(p);  \
    p = NULL;   \
} while (0)

static void test_hci_trans_h4_send_callback(uint8_t package_type, uint8_t *package, uint16_t size)
{
    uassert_int_equal(package_type, h4_send_object.expected_recv_pkg_type);
    uassert_int_equal(size, h4_send_object.expected_recv_len);
    uassert_buf_equal(package, h4_send_object.expected_recv, size);

    rt_thread_mdelay(10);

    rt_sem_release(&sync_sem);
}

static void test_hci_trans_h4_send(void)
{
    int err;
    uint8_t *p = NULL;
//    hci_trans_h4_open();
    hci_trans_h4_register_callback(test_hci_trans_h4_send_callback);

/*     h4_send_object.send_pkg_type = HCI_TRANS_H4_TYPE_CMD;
    h4_send_object.send = h4_send1;
    h4_send_object.send_len = ARRAY_SIZE(h4_send1);
    h4_send_object.expected_recv_pkg_type = HCI_TRANS_H4_TYPE_EVT;
    h4_send_object.expected_recv = h4_recv1;
    h4_send_object.expected_recv_len = ARRAY_SIZE(h4_recv1);

    p = (uint8_t *)hci_trans_h4_send_alloc(h4_send_object.send_pkg_type);
    uassert_not_null(p);
    rt_memcpy(p, h4_send_object.send, h4_send_object.send_len);

    err = hci_trans_h4_send(h4_send_object.send_pkg_type, p);
    uassert_int_equal(err, HM_SUCCESS);
    err = rt_sem_take(&sync_sem, 10);
    uassert_int_equal(err, RT_EOK);

    hci_trans_h4_send_free(p);
    p = NULL; */
    
    hci_reset_cmd_send();
    rt_thread_mdelay(100);

    hci_reset_cmd_send();
    rt_thread_mdelay(100);

    hci_reset_cmd_send();
    rt_thread_mdelay(100);

    SEND_TEST(HCI_TRANS_H4_TYPE_CMD, HCI_TRANS_H4_TYPE_EVT, 1);
    SEND_TEST(HCI_TRANS_H4_TYPE_CMD, HCI_TRANS_H4_TYPE_EVT, 2);
    SEND_TEST(HCI_TRANS_H4_TYPE_CMD, HCI_TRANS_H4_TYPE_EVT, 3);

    hci_trans_h4_remove_callback(test_hci_trans_h4_send_callback);
//    hci_trans_h4_close();
}

static uint16_t test_hci_trans_h4_callback_count = 0;

static void test_hci_trans_h4_callback_recv_pkg()
{
    int err;
    uint8_t pkg[] = { 0x04, 0x00, 0x00};
    for (size_t i = 0; i < ARRAY_SIZE(pkg); i++) {
        err = hci_trans_h4_recv_byte(pkg[i]);
        uassert_int_equal(err, HM_SUCCESS);
    }
}

static void test_hci_trans_h4_callback1(uint8_t package_type, uint8_t *package, uint16_t size)
{
    test_hci_trans_h4_callback_count += 1;
}

static void test_hci_trans_h4_callback2(uint8_t package_type, uint8_t *package, uint16_t size)
{
    test_hci_trans_h4_callback_count += 2;
}

static void test_hci_trans_h4_callback(void)
{
    test_hci_trans_h4_callback_count = 0;
    hci_trans_h4_register_callback(test_hci_trans_h4_callback1);
    hci_trans_h4_register_callback(test_hci_trans_h4_callback2);
    test_hci_trans_h4_callback_recv_pkg();
    uassert_int_equal(test_hci_trans_h4_callback_count, 3);

    test_hci_trans_h4_callback_count = 0;
    hci_trans_h4_remove_callback(test_hci_trans_h4_callback2);
    test_hci_trans_h4_callback_recv_pkg();
    uassert_int_equal(test_hci_trans_h4_callback_count, 1);

    test_hci_trans_h4_callback_count = 0;
    hci_trans_h4_remove_callback(test_hci_trans_h4_callback1);
    test_hci_trans_h4_callback_recv_pkg();
    uassert_int_equal(test_hci_trans_h4_callback_count, 0);
}

static uint8_t hci_cmd_send1[] = { 0x03, 0x0C, 0x00 };
static uint8_t hci_evt_recv1[] = { 0x0E, 0x04, 0x01, 0x03, 0x0C, 0x00 };
static int test_hci_cmd_send_sync_count = 0;

static void hci_vendor_evt_callback(uint8_t *hci_evt, uint16_t len)
{
    uassert_int_equal(len, ARRAY_SIZE(hci_evt_recv1));
    uassert_buf_equal(hci_evt, hci_evt_recv1, len);

    test_hci_cmd_send_sync_count += 2;
}

static void test_hci_cmd_send_sync_h4_callback(uint8_t package_type, uint8_t *package, uint16_t size)
{
    test_hci_cmd_send_sync_count += 1;
}

static void test_hci_cmd_send_sync(void)
{
    int err;
//    hci_trans_h4_open();
    hci_trans_h4_register_callback(test_hci_cmd_send_sync_h4_callback);

    err = hci_vendor_cmd_send_sync(hci_cmd_send1, ARRAY_SIZE(hci_cmd_send1), 2000, NULL);
    uassert_int_equal(err, HM_SUCCESS);
    uassert_int_equal(test_hci_cmd_send_sync_count, 0);

    err = hci_vendor_cmd_send_sync(hci_cmd_send1, ARRAY_SIZE(hci_cmd_send1), 2000, hci_vendor_evt_callback);
    uassert_int_equal(err, HM_SUCCESS);
    uassert_int_equal(test_hci_cmd_send_sync_count, 2);

    err = hci_cmd_send_sync(hci_cmd_send1, ARRAY_SIZE(hci_cmd_send1), 2000);
    uassert_int_equal(err, HM_SUCCESS);

    err = hci_reset_cmd_send();
    uassert_int_equal(err, HM_SUCCESS);

    test_hci_cmd_send_sync_count = 0;
    hci_trans_h4_remove_callback(test_hci_cmd_send_sync_h4_callback);
//    hci_trans_h4_close();
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_hci_trans_h4_recv_byte);
    UTEST_UNIT_RUN(test_hci_trans_h4_send);
    UTEST_UNIT_RUN(test_hci_trans_h4_callback);
    UTEST_UNIT_RUN(test_hci_cmd_send_sync);

    /* For mock test hci_trans_h4_send() */
    // UTEST_UNIT_RUN(mock_test_hci_trans_h4_send);
}
UTEST_TC_EXPORT(testcase, "hm.hci_transport_h4", utest_tc_init, utest_tc_cleanup, 1000);
