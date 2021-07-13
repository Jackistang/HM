#ifndef HM_CHIPSET_H
#define HM_CHIPSET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CHIPSET_ITER_STOP = 0,
    CHIPSET_ITER_CONTINUE,
};

struct rt_chipset {
    char *name;

    /**
     * @brief User default init.
     * 
     * @return void
    */
    void (*init)(void);

    /**
     * @brief Send vendor HCI command.
     * 
     * @param[out] buf A pointer to HCI command buffer.
     * 
     * @return int
     * @retval  CHIPSET_ITER_STOP       Stop send command.
     * @retval  CHIPSET_ITER_CONTINUE   Continue send command.
     * 
    */
    int (*next_hci_command)(const uint8_t **buf);

};

/**
 * @brief Register a chipset handle.
 * 
 * @param[in] chipset   A chipset handle, should be static memory.
 * 
 * @return void
*/
extern void rt_chipset_register(struct rt_chipset *chipset);

/**
 * @brief Init chipset.
 * 
 * @return void
*/
extern void rt_chipset_init(void);

extern int hci_cmd_send_sync(uint16_t opcode, uint8_t *buf, uint8_t **rsp);

#ifdef __cplusplus
}
#endif

#endif /* HM_CHIPSET_H */
