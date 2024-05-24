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
} can_rx_frame_s;

typedef struct
{
    CAN_TxHeaderTypeDef header;
    uint8_t data[8];
} can_tx_frame_s;

#define HEARTBEAT_ID (0x00A)
#define HEARTBEAT_DLC (sizeof(heartbeat_msg_s))
typedef struct
{
    uint16_t seqnum;
    uint16_t _reserved0;
    uint32_t timestamp;
} __attribute__((packed)) heartbeat_msg_s;
STATIC_SIZE(heartbeat_msg_s, 8);

#define DO_WORK0_ID (0x00B)

#ifdef __cplusplus
}
#endif

#endif /* CAN_H */
