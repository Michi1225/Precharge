#pragma once    

#include "main.h"

#define PC_SENSITIVITY 0.250f // V/A
#define BP_SENSITIVITY 0.05f   // V/A
#define CS_OFFSET 0.33f       // V


/**
 * @brief  Initialize the current sensing ADCs and perform zero-current offset calibration. Also, initializes the Imon DAC.
 * @param  None
 * @retval HAL status
 */
HAL_StatusTypeDef cs_init();

/**
 * @brief  Get the precharge current measurement.
 * @param  None
 * @retval Current in Amperes
 */
float cs_get_pc_current();

/**
 * @brief  Get the bypass current measurement.
 * @param  None
 * @retval Current in Amperes
 */
float cs_get_bp_current();

/**
 * @brief  Get the raw ADC measurement for precharge current.
 * @param  None
 * @retval Raw ADC value (16-bit ADC value)
 */
uint32_t cs_get_pc_raw();


/**
 * @brief  Get the internal reference voltage measurement.
 * @param  None
 * @retval Vrefint voltage in Volts
 */
float get_vrefint();

/**
 * @brief  Set the Imon DAC output based on the measured current. Full scale is 40A.
 * @param  None
 * @retval None
 */
void set_Imon();


