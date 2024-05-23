#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "stm32f4xx_hal_eth.h" /* From FreeRTOS-Plus-TCP, not the HAL */

#include "logging.h"
#include "led.h"
#include "ip.h"

#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_ll_rng.h"

static const uint8_t MAC_ADDR[6] = {
    configMAC_ADDR0,
    configMAC_ADDR1,
    configMAC_ADDR2,
    configMAC_ADDR3,
    configMAC_ADDR4,
    configMAC_ADDR5
};

static const uint8_t IP_ADDR[4] = {
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};

static const uint8_t NET_MASK[4] = {
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};

static const uint8_t GW_ADDR[4] = {
    configGATEWAY_ADDR0,
    configGATEWAY_ADDR1,
    configGATEWAY_ADDR2,
    configGATEWAY_ADDR3
};

static const uint8_t DNS_ADDR[4] = {
    configDNS_SERVER_ADDR0,
    configDNS_SERVER_ADDR1,
    configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3
};

static RNG_HandleTypeDef g_rng = {0};

void ip_init(void)
{
    BaseType_t ret;
    HAL_StatusTypeDef hal_ret;

    INFO("Initializing TCP/IP stack");

    // Setup RNG for random number gen
    __HAL_RCC_RNG_CLK_ENABLE();
    g_rng.Instance = RNG;
    hal_ret = HAL_RNG_Init(&g_rng);
    configASSERT(hal_ret == HAL_OK);

    ret = FreeRTOS_IPInit(
            IP_ADDR,
            NET_MASK,
            GW_ADDR,
            DNS_ADDR,
            MAC_ADDR);
    configASSERT(ret == pdPASS);
}

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t network_event)
{
    if(network_event == eNetworkUp)
    {
        INFO("Network up");
    }
    else
    {
        INFO("Network down");
    }
}

BaseType_t xApplicationGetRandomNumber(uint32_t* num)
{
    HAL_StatusTypeDef ret;

    configASSERT(num != NULL);
    ret = HAL_RNG_GenerateRandomNumber(&g_rng, num);
    configASSERT(ret == HAL_OK);

    return pdTRUE;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t src_addr, uint16_t src_port, uint32_t dst_addr, uint16_t dst_port)
{
    uint32_t num;
    (void) src_addr;
    (void) src_port;
    (void) dst_addr;
    (void) dst_port;

    (void) xApplicationGetRandomNumber(&num);

    return num;
}

void HAL_ETH_MspInit(ETH_HandleTypeDef * heth)
{
    GPIO_InitTypeDef gpio = {0};
    (void) heth;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    gpio.Speed = GPIO_SPEED_HIGH;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Alternate = GPIO_AF11_ETH;
    gpio.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &gpio);

    gpio.Pin =  GPIO_PIN_11 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOG, &gpio);

    HAL_NVIC_SetPriority(ETH_IRQn, ipconfigMAC_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    __HAL_RCC_ETHMAC_CLK_ENABLE();
    __HAL_RCC_ETHMACTX_CLK_ENABLE();
    __HAL_RCC_ETHMACRX_CLK_ENABLE();
    __HAL_RCC_ETH_CLK_ENABLE();
}
