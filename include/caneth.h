/* CAN-ETH protocol, all fields are little-endian */

#ifndef CANETH_H
#define CANETH_H

#ifdef __cplusplus
extern "C" {
#endif

#define CANETH_UDP_PORT 11898

/* 8-byte identifier, ASCII characters "ISO11898" */
#define CANETH_MAGIC_ID 0x38393831314F5349UL

#define CANETH_VERSION 1

#define CANETH_MAX_COUNT 16

#define CANETH_DATAGRAM_MAX_SIZE (sizeof(caneth_header_s) + (sizeof(caneth_frame_s) * CANETH_MAX_COUNT))

/* Datagram begins with a CANETH header */
typedef struct
{
    uint64_t magic_id; /* Always CANETH_MAGIC_ID */
    uint8_t version; /* Set to CANETH_VERSION */
    uint8_t count; /* Number of CAN frames in the UDP message (1-16) */
} __attribute__((packed)) caneth_header_s;
STATIC_SIZE(caneth_header_s, 10);

typedef struct
{
    uint32_t id; /* CAN ID */
    uint8_t count; /* CAN DLC */
    uint8_t data[8]; /* CAN bytes. LSB first. Zero-filled if less than 8 bytes */
    uint8_t ext_flag; /* 0/1 flag indicates if CAN ID is 11 or 29 bit */
    uint8_t rtr_flag; /* 0/1 flag indicates a remote transmission frame */
} __attribute__((packed)) caneth_frame_s;
STATIC_SIZE(caneth_frame_s, 15);

#ifdef __cplusplus
}
#endif

#endif /* CANETH_H */
