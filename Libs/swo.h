#pragma once
#include "main.h"

#define SWO_SPEED 1E6 // 10Mhz
#define ENABLE_PORT0 1
#define ENABLE_PORT1 1
#define ENABLE_PORT2 0
#define ENABLE_PORT3 0
#define ENABLE_PORT4 0
#define ENABLE_PORT5 0
#define ENABLE_PORT6 0
#define ENABLE_PORT7 0

void SWD_Init(void)
{
    // Enable GPIOB clock (for PB3 = SWO)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // Configure PB3 as Alternate Function
    GPIOB->MODER   = (GPIOB->MODER & ~GPIO_MODER_MODE3_Msk) | (0x2U << GPIO_MODER_MODE3_Pos); // AF
    GPIOB->AFR[0]  = (GPIOB->AFR[0] & ~(0xFU << GPIO_AFRL_AFSEL3_Pos)) | (0x0U << GPIO_AFRL_AFSEL3_Pos); // AF0 = SWO
    GPIOB->OSPEEDR |= (0x3U << GPIO_OSPEEDR_OSPEED3_Pos); // very high speed

    // --- Debug block setup ---
    // On G4, DBGMCU is simple, no need to poke 0x5C00xxxx
    DBGMCU->CR |= DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY | DBGMCU_CR_DBG_SLEEP; // optional, keep debug in low-power
    DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;
    DBGMCU->CR &= ~DBGMCU_CR_TRACE_MODE;  // 00: asynchronous mode (for SWO)


    // --- Trace setup ---
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // enable tracing
    TPI->SPPR = 2;                                   // Pin protocol = NRZ / UART
    TPI->ACPR = (SystemCoreClock / SWO_SPEED) - 1;   // Async prescaler

    ITM->LAR  = 0xC5ACCE55; // unlock ITM
    ITM->TCR  = ITM_TCR_ITMENA_Msk | ITM_TCR_TSENA_Msk | ITM_TCR_SWOENA_Msk; // enable ITM + timestamp + SWO
    ITM->TCR = ITM_TCR_ITMENA_Msk | ITM_TCR_TSENA_Msk |
           ITM_TCR_SWOENA_Msk | (1 << ITM_TCR_TraceBusID_Pos);

    ITM->TER  = ENABLE_PORT0 | (ENABLE_PORT1 << 1) | (ENABLE_PORT2 << 2) | (ENABLE_PORT3 << 3) |
                 (ENABLE_PORT4 << 4) | (ENABLE_PORT5 << 5) | (ENABLE_PORT6 << 6) | (ENABLE_PORT7 << 7); // enable stimulus ports

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; // enable cycle counter (optional)
}