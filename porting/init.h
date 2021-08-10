#ifndef HM_PORTING_INIT_H
#define HM_PORTING_INIT_H

#include "hm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if HM_CONFIG_NIMBLE
extern int hm_nimble_init(void);
#endif

#if HM_CONFIG_BTSTACK
extern int btstack_rtthread_port_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HM_PORTING_INIT_H */