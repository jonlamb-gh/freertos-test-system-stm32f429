#ifndef TP_H
#define TP_H

/* TODO docs 
 * can be defined in config file, locally, or as compiler opts */
//#define TRACEPOINT_COMPONENT caneth
//#define TRACEPOINT_CONFIG_INCLUDE "caneth/caneth_tracepoint_config.h"

#define TRACE_CFG_TRACEPOINT_ENABLED (1)
#define TRACE_CFG_PACKET_SIZE (64)
// just using led for now
#define TRACEPOINT_COMPONENT led
//#define TRACE_CFG_ENABLE_FROM_ISR (1)

#include "tracepoint.h"

#endif /* TP_H */
