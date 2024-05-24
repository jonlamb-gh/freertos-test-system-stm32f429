#ifndef STATS_TIMER_H
#define STATS_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void stats_timer_init(void);

uint32_t stats_timer_read(void);

#ifdef __cplusplus
}
#endif

#endif /* STATS_TIMER_H */
