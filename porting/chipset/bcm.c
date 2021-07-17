#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"
#include <stdio.h>
#include <unistd.h>
#include <rtthread.h>

#define BCM_PATH_NAME    "/dev/bt_image"
#define HCI_COMMAND_HEADER  3

static uint8_t download_commands[] = {
    0x2e, 0xfc, 0x00,
};

static struct rt_semaphore sync_sem;

static void hci_trans_h4_package_callback(uint8_t pkg_type, uint8_t *pkg, uint16_t size)
{
    RT_ASSERT(pkg_type == HCI_TRANS_H4_TYPE_EVT);

    rt_sem_release(&sync_sem);
}

int chipset_bcm_init(void)
{
    int err = HM_SUCCESS;

    rt_sem_init(&sync_sem, "bcm sync sem", 0, RT_IPC_FLAG_PRIO);

    int fd = open(BCM_PATH_NAME, O_RDONLY);
    if (fd < 0) {
        rt_kprintf("Open bcm file %d error\n", BCM_PATH_NAME);
        err = HM_OPEN_FILE_ERROR;
        goto err_open;
    }

    err = hci_vendor_cmd_send_sync(download_commands, ARRAY_SIZE(download_commands), 1000, NULL);
    if (err) {
        rt_kprintf("bcm chipset init fail err(%d)\n", err);
        err = HM_CHIPSET_INIT_ERROR;
        goto err_download_cmd;
    }

    hci_trans_h4_register_callback(hci_trans_h4_package_callback);
    uint8_t *p = hci_trans_h4_send_alloc(HCI_TRANS_H4_TYPE_CMD);
    RT_ASSERT(p);

    int count = 0;
    uint16_t next_read = 0;
    while ((count = read(fd, p, HCI_COMMAND_HEADER)) == HCI_COMMAND_HEADER) {
        next_read = p[2];

        count = read(fd, p + HCI_COMMAND_HEADER, next_read);
        if (count != next_read) {
            err = HM_CHIPSET_INIT_FILE_ERROR;
            goto err_download;
        }

        hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, p);
        err = rt_sem_take(&sync_sem, 1000);
        if (err) {
            err = HM_CHIPSET_INIT_ERROR;
            goto err_download;
        }
    }

err_download:
    hci_trans_h4_send_free(p);
    hci_trans_h4_remove_callback(hci_trans_h4_package_callback);

err_download_cmd:
    close(fd);

err_open:
    rt_sem_detach(&sync_sem);
    return err;
}

static hm_chipset_t chipset_bcm = {
    .name = "bcm",
    .init = chipset_bcm_init,
};

hm_chipset_t* hm_chipset_get_instance(void)
{
    return &chipset_bcm;
}
