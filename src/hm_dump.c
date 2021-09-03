#include <rtthread.h>

static void hm_dump(uint8_t in, uint8_t type, uint8_t *data, uint16_t len)
{
    rt_kprintf("[0000-00-00 00:00:00.000] ");

    switch (type) {
    case 1:
        rt_kprintf("CMD ");
        break;
    case 2:
        rt_kprintf("ACL ");
        break;
    case 3:
        rt_kprintf("SCO ");
        break;
    case 4:
        rt_kprintf("EVT ");
        break;
    default:
        break;
    }

    if (in) {
        rt_kprintf("<= ");
    } else {
        rt_kprintf("=> ");
    }

    uint8_t *cur = data;
    uint8_t *end = data + len-1;
    while (cur <= end) {
        rt_kprintf("%02x ", *cur);
        cur++;
    }
    rt_kprintf("\n");
}

void hm_dump_out(uint8_t type, uint8_t *data)
{
    uint16_t len = 0;

    switch (type) {
    case 1:
        len = data[2] + 3;
        break;
    case 2:
        len = ((uint16_t)data[2] | (uint16_t)data[3] << 8) + 4;
        break;
    default:
        break;
    }
    hm_dump(0, type, data, len);
}

void hm_dump_in(uint8_t type, uint8_t *data)
{
    uint16_t len = 0;

    switch (type) {
    case 2:
        len = ((uint16_t)data[2] | (uint16_t)data[3] << 8) + 4;
        break;
    case 4:
        len = data[1] + 2;
    default:
        break;
    }

    hm_dump(1, type, data, len);
}

