#ifndef HM_DUMP_H
#define HM_DUMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void hm_dump_out(uint8_t type, uint8_t *data);

extern void hm_dump_in(uint8_t type, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* HM_DUMP_H */
