/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020/12/31     Bernard      Add license info
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>

static uint8_t ad_data[] = { 0x01, 0x08, 0x20, 0x20, 0x14, 0x02, 0x01, 0x06, 0x0d, 
                    0x09, 0x62, 0x6c, 0x65, 0x68, 0x72, 0x5f, 0x73, 0x65, 0x6e, 
                    0x73, 0x6f, 0x72, 0x02, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uint8_t ad_params[] = { 0x01, 0x06, 0x20, 0x0f, 0x30, 0x00, 0x60, 0x00, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00 };

static uint8_t ad_enable[] = { 0x01, 0x0a, 0x20, 0x01, 0x01 };

static void print_command(uint8_t *buf, uint16_t len)
{
    rt_kprintf("CMD => ");
    for (uint16_t i = 0; i < len; i++) {
        rt_kprintf("%02X ", buf[i]);
    }
    rt_kprintf("\n");
}

static void print_event(uint8_t *buf, uint16_t len)
{
    rt_kprintf("EVT <= ");
    for (uint16_t i = 0; i < len; i++) {
        rt_kprintf("%02X ", buf[i]);
    }
    rt_kprintf("\n");
}

static int start_advertisement(int argc, char *argv[])
{
    uint8_t recv[50];
    uint16_t recv_len;

    rt_device_t ble = rt_device_find("uart1");
    rt_device_open(ble, RT_DEVICE_FLAG_INT_RX);;

    // Set advertisement data
    print_command(ad_data, sizeof(ad_data));
    rt_device_write(ble, 0, ad_data, sizeof(ad_data));
    rt_thread_mdelay(10);
    recv_len = rt_device_read(ble, 0, recv, sizeof(recv));
    print_event(recv, recv_len);

    // Set advertisement parameters
    print_command(ad_params, sizeof(ad_params));
    rt_device_write(ble, 0, ad_params, sizeof(ad_params));
    rt_thread_mdelay(10);
    recv_len = rt_device_read(ble, 0, recv, sizeof(recv));
    print_event(recv, recv_len);

    // Enable advertisement
    print_command(ad_enable, sizeof(ad_enable));
    rt_device_write(ble, 0, ad_enable, sizeof(ad_enable));
    rt_thread_mdelay(10);
    recv_len = rt_device_read(ble, 0, recv, sizeof(recv));
    print_event(recv, recv_len);

    return 0;
}
MSH_CMD_EXPORT(start_advertisement, "HM sample");
