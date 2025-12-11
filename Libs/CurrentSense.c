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
    HAL_Delay(100); // Allow some time for initial readings


    //Average multiple samples to determine zero-current offset
    uint64_t offset_pc_avg = 0;
    uint64_t offset_bp_avg = 0;
    const uint32_t samples = 100;
    for(uint32_t i = 0; i < samples; i++)
    {
        offset_pc_avg += cs_pc_raw & 0xFFFF;
        offset_bp_avg += cd_bp_raw & 0xFFFF;
        HAL_Delay(0);
    }
    offset_pc_avg /= samples;
    offset_bp_avg /= samples;
    zero_offset_pc = offset_pc_avg; // Capture zero-current offset
    zero_offset_bp = offset_bp_avg; // Capture zero-current offset


    // Set initial DAC output to zero
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 0);
    //Start DAC
    if(HAL_DAC_Start(&hdac1, DAC_CHANNEL_2) != HAL_OK) return HAL_ERROR;

    return HAL_OK;
}


float cs_get_pc_current()
{
    uint32_t cs_pc_raw_corrected = cs_pc_raw & 0xFFFF; // 16-bit ADC
    cs_pc_raw_corrected = (cs_pc_raw_corrected > zero_offset_pc) ? (cs_pc_raw_corrected - zero_offset_pc) : 0;
    float adc_voltage = ((float)cs_pc_raw_corrected) * 3.3f / 65535.0f; 
    return adc_voltage / PC_SENSITIVITY;
}                                                                                    

float cs_get_bp_current()
{
    uint32_t cs_bp_corrected = cd_bp_raw & 0xFFFF; // 16-bit ADC
    cs_bp_corrected = (cs_bp_corrected > zero_offset_bp) ? (cs_bp_corrected - zero_offset_bp) : 0;
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

void set_Imon()
{
    // --- constants ---
    const uint32_t ADC_60A = 59605u;  // ADC code ≈ 3.0V at 0.05 V/A with Vref=3.3V
    const uint32_t DAC_MIN = 250u;
    const uint32_t DAC_MAX = 4095u - 250u;   // 12-bit DAC full scale

    //ADC read processing
    uint32_t cs_bp_corrected = cd_bp_raw & 0xFFFF;  // 16-bit raw ADC
    cs_bp_corrected = (cs_bp_corrected > zero_offset_bp)
                        ? (cs_bp_corrected - zero_offset_bp)
                        : 0; // zero-offset correction

    // --- scaling 16-bit corrected ADC to 12-bit DAC (0..60A maps to 0..4095) ---
    uint32_t dac_val = cs_bp_corrected * (DAC_MAX - DAC_MIN) / ADC_60A + DAC_MIN;
    if(dac_val > DAC_MAX) dac_val = DAC_MAX;
    if(dac_val < DAC_MIN) dac_val = DAC_MIN;

    // --- output to DAC channel 1 (right aligned 12-bit) ---
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_val);
}

