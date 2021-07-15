#ifndef HM_CHIPSET_H
#define HM_CHIPSET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    
} hm_chipset_t;

hm_chipset_t* hm_chipset_get_instance(void);

#ifdef __cplusplus
}
#endif

#endif /* HM_CHIPSET_H */
