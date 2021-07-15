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

typedef struct hm_chipset {
    char *name;

    /**
     * @brief User default init.
     * 
     * @return int
     * @retval  HM_SUCCESS              Chipset init success.
     * @retval  HM_CHIPSET_INIT_ERROR   Chipset init error.
    */
    int (*init)(void);

    // /**
    //  * @brief Send vendor HCI command.
    //  * 
    //  * @param[out] buf A pointer to HCI command buffer.
    //  * 
    //  * @return int
    //  * @retval  CHIPSET_ITER_STOP       Stop send command.
    //  * @retval  CHIPSET_ITER_CONTINUE   Continue send command.
    //  * 
    // */
    // int (*next_hci_command)(const uint8_t **buf);

} hm_chipset_t;

hm_chipset_t* hm_chipset_get_instance(void);

#ifdef __cplusplus
}
#endif

#endif /* HM_CHIPSET_H */
