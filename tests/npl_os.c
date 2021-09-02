#include <rtthread.h>
#include "nimble/nimble_npl.h"
#include "utest.h"

struct ble_npl_eventq evq;

static rt_err_t utest_tc_init(void)
{
    ble_npl_eventq_init(&evq);
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static int evt_arg = 10; 
static void ble_npl_event_cb(struct ble_npl_event *ev)
{
    uassert_int_equal(ev->arg, &evt_arg);
}

static void testBleNplEventqSendSuccess(void)
{
    struct ble_npl_event evt;
    ble_npl_event_init(&evt, ble_npl_event_cb, &evt_arg);
    ble_npl_eventq_put(&evq, &evt);
    struct ble_npl_event *recv_evt = ble_npl_eventq_get(&evq, 100);
    uassert_not_null(recv_evt);
    uassert_int_equal(recv_evt, &evt);
    ble_npl_event_run(&evt);
}

static void testBleNplEventqRecvFail(void)
{
    struct ble_npl_event *recv_evt = ble_npl_eventq_get(&evq, 100);
    uassert_null(recv_evt);
}

static void testcase_chipset(void)
{
    UTEST_UNIT_RUN(testBleNplEventqSendSuccess);
    UTEST_UNIT_RUN(testBleNplEventqRecvFail);
}
UTEST_TC_EXPORT(testcase_chipset, "npl.os", utest_tc_init, utest_tc_cleanup, 1000);
