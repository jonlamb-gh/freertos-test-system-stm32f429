#ifndef CAN_H
#define CAN_H

#include "stm32f4xx_hal_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];
} can_frame_s;

#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
