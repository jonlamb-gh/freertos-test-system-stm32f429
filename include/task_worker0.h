#ifndef TASK_WORKER0_H
#define TASK_WORKER0_H

#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

void task_worker0_start(void);

void task_worker0_do_work(const can_rx_frame_s *const frame);

#ifdef __cplusplus
}
#endif

#endif /* TASK_WORKER0_H */
