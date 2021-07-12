#include "chipset.h"
#include <assert.h>
#include <string.h>

#define ARRAY_SIZE(array)   ((sizeof(array) / sizeof(array[0])))

static const uint8_t g_init_command[] = {
    // OGF: 0x3F, OCF: 0x09, Paramter Total Length: 0x00. 
    // Zephyr Controller 
    0x09, 0xfc, 0x00, 
};

static uint16_t g_index;

static void init(void *args)
{
    g_index = 0;
    return ;
}

static int next_hci_command(uint8_t *buf)
{
    assert(buf);

    if (g_index >= ARRAY_SIZE(g_init_command))
        return CHIPSET_ITER_STOP;
    
    // Copy command header.
    memcpy(&buf[0], &g_init_command[g_index], 3);
    g_index += 3;

    // Copy command payload.
    uint16_t payload_len = buf[2];
    memcpy(&buf[3], &g_init_command[g_index], payload_len);
    g_index += payload_len;
 
    return CHIPSET_ITER_CONTINUE;
}


static struct rt_chipset chipset_zephyr = {
    .name             = "Zephyr",
    .init             = init,
    .next_hci_command = next_hci_command,
};

rt_chipset_t* rt_chipset_get_instance(void)
{
    return &chipset_zephyr;
}




