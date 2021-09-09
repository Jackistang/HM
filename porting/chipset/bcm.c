#include "hm_chipset.h"
#include "hm_hci_transport_h4.h"
#include "hm_config.h"
#include <stdio.h>
#include <unistd.h>
#include <rtthread.h>


#define BCM_PATH_NAME    "/dev/bt_image"
#define HCI_COMMAND_HEADER  3

static uint8_t download_commands[] = {
    0x2e, 0xfc, 0x00,
};

int chipset_bcm_init(void)
{
    int err = HM_SUCCESS;
    int res = 0;
    uint8_t event_buf[20];

    chip_send_hci_reset_cmd_until_ack();

    int fd = open(BCM_PATH_NAME, O_RDONLY);
    if (fd < 0) {
        rt_kprintf("Open bcm file %d error\n", BCM_PATH_NAME);
        return HM_OPEN_FILE_ERROR;
    }

    rt_kprintf("bcm init start\n");

    chip_hci_cmd_send(download_commands, ARRAY_SIZE(download_commands));
    chip_hci_event_read(event_buf, ARRAY_SIZE(event_buf), RT_WAITING_FOREVER);
    if (event_buf[0] != 0x0E) {
        err = HM_CHIPSET_INIT_ERROR;
        goto err_download_cmd;
    }

    uint16_t next_read = 0;
    uint8_t *p = hci_trans_h4_send_alloc(HCI_TRANS_H4_TYPE_CMD);
    RT_ASSERT(p);

    rt_kprintf("bcm start download init script\n");

    do {
        res = read(fd, p, 3);
        if (res == 0) {
            rt_kprintf("bcm chipset init file read end\n");
            break;
        }
        if (res < 0) {
            rt_kprintf("bcm chipset init file read error (%d)\n", res);
            err = HM_CHIPSET_INIT_FILE_ERROR;
            goto err_download;
        }

        /* Unknown error, test find it, is file end. */
        if (p[0] == 0xFF && p[1] == 0xFF && p[2] == 0xFF) {
            break;
        }

        next_read = p[2];
        res = read(fd, p + 3, next_read);
        if (res < 0) {
            rt_kprintf("bcm chipset init file read error (%d)\n", res);
            err = HM_CHIPSET_INIT_FILE_ERROR;
            goto err_download;
        }

        hci_trans_h4_send(HCI_TRANS_H4_TYPE_CMD, p);

        chip_hci_event_read(event_buf, ARRAY_SIZE(event_buf), RT_WAITING_FOREVER);
        if (event_buf[0] != 0x0E) {
            err = HM_CHIPSET_INIT_ERROR;
            goto err_download;
        }

    } while(1);

    close(fd);
    hci_trans_h4_send_free(p);
    rt_kprintf("bcm init script download success\n");

    chip_send_hci_reset_cmd_until_ack();

    rt_kprintf("bcm init success\n");
    return HM_SUCCESS;

err_download:
    hci_trans_h4_send_free(p);

err_download_cmd:
    close(fd);

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


#ifdef HM_USING_BOARD_ART_PI

#define BT_FIRMWARE_PARTITION_NAME "bt_image"
static int bt_firmware_create(void)
{
    rt_kprintf("bt firmware start create.\n");

    rt_device_t bt_firmware = RT_NULL;
    const struct fal_partition *bt_partition = RT_NULL;

    bt_partition = fal_partition_find(BT_FIRMWARE_PARTITION_NAME);
    if (bt_partition == NULL)
    {
        rt_kprintf("%s partition is not exist, please check your configuration!\n", BT_FIRMWARE_PARTITION_NAME);
        return -1;
    }

    //create bt device fs
    bt_firmware = fal_char_device_create(BT_FIRMWARE_PARTITION_NAME);
    if (bt_firmware == NULL)
    {
        rt_kprintf("bt firmware device create failed\n");
        return -1;
    }

    return 0;
}

#include <spi_flash.h>
#include <drv_spi.h>
static int rt_flash_init(void)
{
    extern rt_spi_flash_device_t rt_sfud_flash_probe(const char *spi_flash_dev_name, const char *spi_dev_name);
    extern int fal_init(void);

    rt_hw_spi_device_attach("spi1", "spi10", GPIOA, GPIO_PIN_4);

    /* initialize SPI Flash device */
    rt_sfud_flash_probe("norflash0", "spi10");

    fal_init();

    bt_firmware_create();

    return 0;
}
INIT_ENV_EXPORT(rt_flash_init);

#endif