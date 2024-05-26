#ifndef TASK_WORKER1_H
#define TASK_WORKER1_H

#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

void task_worker1_start(void);

void task_worker1_do_work(const can_rx_frame_s *const frame);

#ifdef __cplusplus
}
#endif

#endif /* TASK_WORKER1_H */
