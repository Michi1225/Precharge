#include "CurrentSense.h"


// Raw ADC values 
uint32_t cs_raw = 0;
uint32_t vrefint_raw = 0;

uint32_t zero_offset_cs = 0;

/* Cached values to avoid repeated work at runtime */
static float cs_scale_factor = 0.0f; /* multiplies ADC counts -> current */
static float vref_cached = 0.0f;


HAL_StatusTypeDef cs_init()
{
    // Run ADC Calibration
    if(HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    if(HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED) != HAL_OK) return HAL_ERROR;
    HAL_Delay(1); // Wait for ADC to stabilize after calibration

    // Start ADC in DMA mode
    if(HAL_ADC_Start_DMA(&hadc2, &cs_raw, 1) != HAL_OK) return HAL_ERROR;
    if(HAL_ADC_Start_DMA(&hadc3, &vrefint_raw, 1) != HAL_OK) return HAL_ERROR;
    HAL_Delay(100); // Allow some time for initial readings


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

    /* Cache Vref and precompute scale factor to speed up conversions
       Avoid repeated divisions and function calls in the fast path. */
    vref_cached = get_vrefint();
    cs_scale_factor = vref_cached * DIV_RESOLUTION_16_BIT * DIV_CS_SENSITIVITY;

    return HAL_OK;
}


float cs_get_current()
{
    /* Fast path: use precomputed scale factor. Keep operations minimal. */
    uint32_t raw = cs_raw & 0xFFFFU; /* read DMA-updated ADC count */
    if (raw <= zero_offset_cs) return 0.0f;
    uint32_t corrected = raw - zero_offset_cs;
    /* Single multiply: converts counts -> current using cached factor */
    return ((float)corrected) * cs_scale_factor;
}                                                                                    

float get_vrefint()
{
    // Guard against divide-by-zero (DMA may not have provided a sample yet)
    if (vrefint_raw == 0) return 3.3f;
    /* Keep existing formula but avoid unnecessary temporaries */
    return (VREF_INT * RESOLUTION_16_BIT) / (float)vrefint_raw;
}


