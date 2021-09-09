#ifndef HM_PORTING_INIT_H
#define HM_PORTING_INIT_H

#include "hm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HM_USING_STACK_NIMBLE
extern int hm_nimble_init(void);
#endif

#ifdef HM_USING_STACK_BTSTACK
extern int btstack_rtthread_port_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HM_PORTING_INIT_H */