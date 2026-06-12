#pragma once    

#include "main.h"

#define CS_SENSITIVITY 0.05f   // V/A

#define VREF_INT 1.21f
#define RESOLUTION_16_BIT 65535.0f


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
float cs_get_current();


/**
 * @brief  Get the ADC reference Voltage.
 * @param  None
 * @retval Vadc reference voltage in Volts (typically around 3.3V, but can vary)
 */
float get_vrefint();


