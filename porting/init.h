#ifndef HM_PORTING_INIT_H
#define HM_PORTING_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#define HM_CONFIG_BTSTACK   1   /* Use btstack or not. */
#define HM_CONFIG_NIMBLE    0   /* Use nimble or not */

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