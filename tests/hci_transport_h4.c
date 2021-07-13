#include <rtthread.h>
#include "utest.h"


extern void hci_trans_h4_register_packge_callback(void (*callback)(int package_type, uint8_t *packge, uint16_t size));
extern int hci_trans_h4_recv_byte(uint8_t byte);

static struct rt_semaphore sync_sem;

static rt_err_t utest_tc_init(void)
{
    rt_sem_init(&sync_sem, "sync_sem", 0, RT_IPC_FLAG_PRIO);
    
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

hci_trans_h4_recv_byte_test_t hci_trans_h4_recv_byte_test_table[] = {
    {HCI_TRANS_H4_TYPE_CMD, 4, {0x01, 0x3c, 0x0f, 0x00}},
    {HCI_TRANS_H4_TYPE_CMD, 6, {0x01, 0x3c, 0x0f, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ACL, 5, {0x02, 0x01, 0x00, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_ACL, 7, {0x02, 0x01, 0x00, 0x01, 0x00, 0x06, 0x06}},
    {HCI_TRANS_H4_TYPE_SCO, 4, {0x03, 0x01, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_SCO, 6, {0x03, 0x01, 0x00, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_EVT, 3, {0x04, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_EVT, 5, {0x04, 0x80, 0x02, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ISO, 5, {0x05, 0x01, 0x00, 0x00, 0x00}},
    {HCI_TRANS_H4_TYPE_ISO, 7, {0x05, 0x01, 0x00, 0x02, 0x00, 0xfc, 0xfc}},
    {HCI_TRANS_H4_TYPE_ISO, 7, {0x05, 0x01, 0x00, 0x02, 0xc0, 0xfc, 0xfc}},
};

void test_hci_trans_h4_recv_byte_callback(int package_type, uint8_t *packge, uint16_t size)
{

}

static void test_hci_trans_h4_recv_byte(void)
{
    hci_trans_h4_register_packge_callback(test_hci_trans_h4_recv_byte_callback);


    uassert_true(1);
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_hci_trans_h4_recv_byte);
}
UTEST_TC_EXPORT(testcase, "hm.hci_transport_h4", utest_tc_init, utest_tc_cleanup, 10);