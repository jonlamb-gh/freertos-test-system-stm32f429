#ifndef STATUS_FLAGS_H
#define STATUS_FLAGS_H

#ifdef __cplusplus
extern "C" {
#endif

void status_flags_set_can1_error(void);
uint8_t status_flags_get_can1_error(void);

void status_flags_set_can2_error(void);
uint8_t status_flags_get_can2_error(void);

void status_flags_set_caneth_error(void);
uint8_t status_flags_get_caneth_error(void);

void status_flags_set_shell_error(void);
uint8_t status_flags_get_shell_error(void);

#ifdef __cplusplus
}
#endif

#endif /* STATUS_FLAGS_H */
