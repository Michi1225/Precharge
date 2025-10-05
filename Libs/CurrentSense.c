#include "CurrentSense.h"



uint32_t cs_pc_raw = 0;
uint32_t cd_bp_raw = 0;
uint32_t vrefint_raw = 0;

uint32_t zero_offset_pc = 0;
uint32_t zero_offset_bp = 0;


volatile uint8_t adc_toggle = 0;

HAL_StatusTypeDef cs_init()
{
    if(HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    if(HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    if(HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    HAL_Delay(1); // Wait for ADC to stabilize after calibration
    if(HAL_ADC_Start_DMA(&hadc1, &cs_pc_raw, 1) != HAL_OK) return HAL_ERROR;
    if(HAL_ADC_Start_DMA(&hadc2, &cd_bp_raw, 1) != HAL_OK) return HAL_ERROR;
    if(HAL_ADC_Start_DMA(&hadc3, &vrefint_raw, 1) != HAL_OK) return HAL_ERROR;
    HAL_Delay(10); // Allow some time for initial readings
    zero_offset_pc = cs_pc_raw; // Capture zero-current offset
    zero_offset_bp = cd_bp_raw; // Capture zero-current offset

    return HAL_OK;
}

// Only used in discountinuous mode
void cs_run()
{
    adc_toggle ^= 1;
    if (adc_toggle)
    {
        HAL_ADC_Start_DMA(&hadc1, &cs_pc_raw, 1);
    }
    
}

float cs_get_pc_current()
{
    uint32_t cs_pc_raw_corrected = (cs_pc_raw > zero_offset_pc) ? (cs_pc_raw - zero_offset_pc) : 0;
    // cs_pc_raw_corrected &= 0xFFFF; // Ensure 16-bit value
    float adc_voltage = ((float)cs_pc_raw_corrected) * 3.3f / 65535.0f; 
    return adc_voltage / PC_SENSITIVITY + 0.35f; // Add offset to account for weird behaviour
}

float cs_get_bp_current()
{
    uint32_t cs_bp_corrected = (cd_bp_raw > zero_offset_bp) ? (cd_bp_raw - zero_offset_bp) : 0;
    float adc_voltage = ((float)(cs_bp_corrected)) * 3.3f / 65535.0f; 
    return adc_voltage / BP_SENSITIVITY;
}

float get_vrefint()
{
    float adc_voltage = ((float)vrefint_raw) * 3.3f / 65535.0f; 
    return adc_voltage;
}

uint32_t cs_get_pc_raw()
{
    return cs_pc_raw;
}