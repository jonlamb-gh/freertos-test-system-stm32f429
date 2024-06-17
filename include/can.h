// TODO use cantools to generate code from the dbc
// https://github.com/cantools/cantools?tab=readme-ov-file#the-generate-c-source-subcommand

#ifndef CAN_H
#define CAN_H

#include "stm32f4xx_hal_conf.h"
#include "canproto.h" /* Generated from the dbc file */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    CAN_RxHeaderTypeDef header;
    uint8_t data[8];
} can_rx_frame_s;

typedef struct
{
    CAN_TxHeaderTypeDef header;
    uint8_t data[8];
} can_tx_frame_s;

#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
