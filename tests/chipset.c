#include <rtthread.h>
#include <rtdevice.h>
#include "utest.h"
#include "hm_hci_transport_h4.h"
#include "hm_chipset.h"


hm_chipset_t *chipset_instance;

static rt_err_t utest_tc_init(void)
{
    hci_trans_h4_open();

    chipset_instance = hm_chipset_get_instance();

    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{

    hci_trans_h4_close();

    chipset_instance = NULL;

    return RT_EOK;
}

void test_csr8311_init(void)
{
    int err;
    err = chipset_instance->init();
    uassert_int_equal(err, HM_SUCCESS);
}

static void testcase_chipset(void)
{
    UTEST_UNIT_RUN(test_csr8311_init);
}
UTEST_TC_EXPORT(testcase_chipset, "hm.chipset", utest_tc_init, utest_tc_cleanup, 1000);

