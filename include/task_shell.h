#ifndef TASK_SHELL_H
#define TASK_SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

void task_shell_start(void);

/* NOTE: only for the hooks to use, interrupts will be disabled */
void task_shell_unsafe_printf(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif /* TASK_SHELL_H */
