// Based on
// https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS-Plus/Demo/Common/FreeRTOS_Plus_CLI_Demos/UARTCommandConsole.c
// See https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_Command_Line_Interface.html

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "message_buffer.h"
#include "FreeRTOS_CLI.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logging.h"
#include "led.h"
#include "shell_cmds.h"
#include "task_shell.h"

#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_ll_usart.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 1)
#define TASK_STACK_SIZE (4 * configMINIMAL_STACK_SIZE)

#define SHELL_UART USART3
#define SHELL_UART_PORT GPIOD
#define SHELL_UART_TX_PIN GPIO_PIN_8
#define SHELL_UART_RX_PIN GPIO_PIN_9

#define RX_MSGBUF_SIZE (64)
#define SHELL_INPUT_BUFFER_SIZE (128)

// Control chars
#define BS (0x08)
#define DEL (0x7F)

static void init_uart(void);
static void putstr(const char* str, size_t len);
static void shell_task(void* params);

static const char WELCOME_MSG[] = "<<Test System Shell>>\nType help to view a list of registered commands.\n\n";
static const char NEWLINE[] = "\n";

static MessageBufferHandle_t g_rx_msgbuf = NULL;
static char g_input_buffer[SHELL_INPUT_BUFFER_SIZE] = {0};

void task_shell_start(void)
{
    BaseType_t ret;

    init_uart();

    shell_cmds_register();

    g_rx_msgbuf = xMessageBufferCreate(RX_MSGBUF_SIZE);
    configASSERT(g_rx_msgbuf != NULL);
    vTraceSetMessageBufferName(g_rx_msgbuf, "shell_uart_msg_buf");

    ret = xTaskCreate(
            shell_task,
            "CLI",
            TASK_STACK_SIZE,
            NULL,
            TASK_PRIO,
            NULL);
    configASSERT(ret == pdPASS);
}

// NOTE: expecting this to be called by the FreeRTOS hooks, it will corrupt
// the global input buffer, which is ok in this context
void task_shell_unsafe_printf(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vsnprintf(g_input_buffer, sizeof(g_input_buffer), fmt, arg);
    va_end(arg);
    putstr(g_input_buffer, strlen(g_input_buffer));
}

static void init_uart(void)
{
    GPIO_InitTypeDef gpio_init =
    {
        .Pin = (SHELL_UART_TX_PIN | SHELL_UART_RX_PIN),
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_MEDIUM,
        .Alternate = GPIO_AF7_USART3,
    };

    __HAL_RCC_GPIOD_CLK_ENABLE();
    HAL_GPIO_Init(SHELL_UART_PORT, &gpio_init);

    NVIC_SetPriority(USART3_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(USART3_IRQn);

    // Setup USART3, 8N1 115200, using the LL APIs
    __HAL_RCC_USART3_CLK_ENABLE();
    LL_USART_InitTypeDef usart_init =
    {
        .BaudRate = 115200,
        .DataWidth = LL_USART_DATAWIDTH_8B,
        .StopBits = LL_USART_STOPBITS_1,
        .Parity = LL_USART_PARITY_NONE,
        .TransferDirection = LL_USART_DIRECTION_TX_RX,
        .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
        .OverSampling = LL_USART_OVERSAMPLING_16,
    };
    LL_USART_Init(SHELL_UART, &usart_init);

    LL_USART_Enable(SHELL_UART);
    LL_USART_EnableIT_RXNE(SHELL_UART);
    LL_USART_EnableIT_ERROR(SHELL_UART);
}

static void putstr(const char* str, size_t len)
{
    size_t cnt;

    for(cnt = 0; cnt < len; cnt += 1)
    {
        // Wait for TXE
        while(LL_USART_IsActiveFlag_TXE(SHELL_UART) == 0);

        LL_USART_TransmitData8(SHELL_UART, str[cnt]);

        // Wait for TC
        while(LL_USART_IsActiveFlag_TC(SHELL_UART) == 0);
    }
}

static void shell_task(void* params)
{
    uint8_t rx_char;
    char* output_buf;
    BaseType_t ret;
    size_t input_idx = 0;
    (void) params;

    output_buf = FreeRTOS_CLIGetOutputBuffer();

    putstr(WELCOME_MSG, sizeof(WELCOME_MSG));

    while(1)
    {
        // Wait for data
        while(xMessageBufferReceive(g_rx_msgbuf, &rx_char, 1, portMAX_DELAY) == 0);

        // Local echo
        putstr((const char*) &rx_char, 1);

        // EOL
        if((rx_char == '\n') || (rx_char == '\r'))
        {
            putstr(NEWLINE, sizeof(NEWLINE));

            /* Pass the received command to the command interpreter.  The
             * command interpreter is called repeatedly until it returns
             * pdFALSE	(indicating there is no more output) as it might
             * generate more than one string. */
            do
            {
                // Get the next output string from the command interpreter
                ret = FreeRTOS_CLIProcessCommand(g_input_buffer, output_buf, configCOMMAND_INT_MAX_OUTPUT_SIZE);

                // Write the generated string to the UART
                putstr(output_buf, strlen(output_buf));
            }
            while(ret != pdFALSE);

            /* All the strings generated by the input command have been
             * sent.  Clear the input string ready to receive the next command. */
            input_idx = 0;
            memset(g_input_buffer, 0, SHELL_INPUT_BUFFER_SIZE);
        }
        else
        {
            if(rx_char == '\r')
            {
                // Ignore
            }
            else if((rx_char == BS) || (rx_char == DEL))
            {
                // Backspace, erase last char if any
                if(input_idx > 0)
                {
                    input_idx -= 1;
                    g_input_buffer[input_idx] = '\0';
                }
                putstr(" \b", 2);
            }
            else
            {
                /* A character was entered.  Add it to the string entered so
                 * far.  When a \n is entered the complete	string will be
                 * passed to the command interpreter. */
                if((rx_char >= ' ') && (rx_char <= '~'))
                {
                    if(input_idx < SHELL_INPUT_BUFFER_SIZE)
                    {
                        g_input_buffer[input_idx] = rx_char;
                        input_idx += 1;
                    }
                }
            }
        }
    }
}

void USART3_IRQHandler(void)
{
    BaseType_t higher_prio_task_woken;

    // Check RXNE flag value in SR register
    if(LL_USART_IsActiveFlag_RXNE(SHELL_UART) && LL_USART_IsEnabledIT_RXNE(SHELL_UART))
    {
        // Read DR, clears RXNE
        const uint8_t data = LL_USART_ReceiveData8(SHELL_UART);

        // Ignore bytes-written return
        xMessageBufferSendFromISR(
                g_rx_msgbuf,
                &data,
                1,
                &higher_prio_task_woken);
        portYIELD_FROM_ISR(higher_prio_task_woken);
    }
    else
    {
        led_on(LED_RED);
    }
}
