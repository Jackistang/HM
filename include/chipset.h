#ifndef HCI_TRANSPORT_CHIPSET_H
#define HCI_TRANSPORT_CHIPSET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CHIPSET_ITER_STOP = 0,
    CHIPSET_ITER_CONTINUE,
};

typedef struct rt_chipset {
    char *name;

    /**
     * @brief User default init.
     * 
     * @return void
    */
    void (*init)(void *args);

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
    int (*next_hci_command)(uint8_t *buf);

} rt_chipset_t;

/**
 * @brief Get a chipset instance.
 * 
 * @return rt_chipset_t *
 * @retval  Non-NULL on success.
 * @retval  NULL on fail.
*/
extern rt_chipset_t* rt_chipset_get_instance(void);

/**
 * @brief Init chipset.
 * 
 * @return int
 * @retval  0   Success
 * @retval  -1  Fail
*/
extern int rt_chipset_init_start(void);


#ifdef __cplusplus
}
#endif

#endif /* HCI_TRANSPORT_CHIPSET_H */
