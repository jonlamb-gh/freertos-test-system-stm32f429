#ifndef TASK_CANETH_H
#define TASK_CANETH_H

#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

void task_caneth_start(void);

void task_caneth_enqueue_rx_frame(const can_rx_frame_s *const frame);

#ifdef __cplusplus
}
#endif

#endif /* TASK_CANETH_H */
