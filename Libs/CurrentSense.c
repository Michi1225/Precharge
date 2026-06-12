#include "CurrentSense.h"


// Raw ADC values 
uint32_t cs_raw = 0;
uint32_t vrefint_raw = 0;

uint32_t zero_offset_cs = 0;


HAL_StatusTypeDef cs_init()
{
    // Run ADC Calibration
    if(HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    if(HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    HAL_Delay(1); // Wait for ADC to stabilize after calibration

    // Start ADC in DMA mode
    if(HAL_ADC_Start_DMA(&hadc2, &cs_raw, 1) != HAL_OK) return HAL_ERROR;
    if(HAL_ADC_Start_DMA(&hadc3, &vrefint_raw, 1) != HAL_OK) return HAL_ERROR;
    HAL_Delay(1); // Allow some time for initial readings


    //Average multiple samples to determine zero-current offset
    uint64_t offset_avg = 0;
    const uint32_t samples = 100;
    for(uint32_t i = 0; i < samples; i++)
    {
        offset_avg += cs_raw & 0xFFFF;
        HAL_Delay(0);
    }
    offset_avg /= samples;
    zero_offset_cs = offset_avg;

    return HAL_OK;
}


float cs_get_current()
{
    // TODO: Validate this
    // Formula: I = (Vadc - Voffset) / Sensitivity
    // Get raw ADC value and apply zero-current offset correction
    uint32_t cs_raw_corrected = cs_raw & 0xFFFF; // 16-bit ADC
    cs_raw_corrected = (cs_raw_corrected > zero_offset_cs) ? (cs_raw_corrected - zero_offset_cs) : 0;

    // Get ADC reference
    float vref = get_vrefint();

    // Convert to measured current
    float adc_voltage = ((float)cs_raw_corrected) * vref / RESOLUTION_16_BIT; 
    return adc_voltage / CS_SENSITIVITY;
}                                                                                    

float get_vrefint()
{
    //TODO: Validate this!
    // Formula: Vref(3.3V) = (ADC_VAL * VREF_INT(1.21V)) / 2^16
    float adc_voltage = ((float)vrefint_raw) * VREF_INT / RESOLUTION_16_BIT; 
    return adc_voltage;
}


