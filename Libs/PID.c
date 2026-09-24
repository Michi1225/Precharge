#include "PID.h"
#include "CurrentSense.h"
#include "comm.h"
#include "stm32g4a1xx.h"



PIDController current_controller =
{
    .Kp = KP,
    .Ki = KI,
    .Kd = KD,
    .setpoint = 0.0f,
    .integral = 0.0f,
    .prevError = 0.0f,
    .outputMin = OUTPUT_MIN_I,
    .outputMax = OUTPUT_MAX_I,
    .handover_counter = 0,
    .wd_counter = 0
};

uint8_t run = 0;
uint8_t bypass = 0;

static uint16_t oc_to_DAC(float oc)
{
    float voc = CS_SENSITIVITY * oc / 2.5f;
    float vdac = voc *(3.3f + 1.4f) / 1.4f;
    uint16_t ret = (uint16_t)(vdac / get_vrefint() * 4096);
    if(ret > 4095) return 4095;
    return ret;
}


float PID_Compute(PIDController *pid, float measurement, float dt)
{
#ifdef ENABLE_PID
    float error = pid->setpoint - measurement;
    pid->integral += error * dt;
    float derivative = (error - pid->prevError) / dt;

    float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

    // Clamp output to min/max
    if (output > pid->outputMax) {
        output = pid->outputMax;
        // Anti-windup: prevent integral from increasing further
        if (error > 0) {
            pid->integral -= error * dt; // Undo last integral addition
        }
    } else if (output < pid->outputMin) {
        output = pid->outputMin;
        // Anti-windup: prevent integral from decreasing further
        if (error < 0) {
            pid->integral -= error * dt; // Undo last integral addition
        }
    }

    pid->prevError = error;
    return output;
#else
    return 0.0f;
#endif
}

void PID_SetSetpoint(PIDController *pid, float setpoint)
{
    pid->setpoint = setpoint;
    
}

void PID_Reset(PIDController *pid)
{
    pid->integral = 0.0f;
    pid->prevError = 0.0f;
    pid->wd_counter = 0;
    pid->handover_counter = 0;
}


HAL_StatusTypeDef controller_init()
{
    run = 0;
    bypass = 0;
    PID_SetSetpoint(&current_controller, commHandler.inputMemMap.input_registers.tracking_current);
    current_controller.wd_timeout = commHandler.inputMemMap.input_registers.wd_timeout;

    HAL_StatusTypeDef error = HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    TIM3->CCR4 = 0;
    
    error |= HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
    error |= HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R
            , oc_to_DAC(commHandler.inputMemMap.input_registers.oc_threshold_pc));

    HAL_Delay(0);
    //Reset OC latch
    HAL_GPIO_WritePin(nCLR_OC_GPIO_Port, nCLR_OC_Pin, GPIO_PIN_RESET);
    HAL_Delay(9);
    HAL_GPIO_WritePin(nCLR_OC_GPIO_Port, nCLR_OC_Pin, GPIO_PIN_SET);


    //Controller ready

    commHandler.outputMemMap.output_registers.hw_oc_fault = HAL_GPIO_ReadPin(OC_GPIO_Port, OC_Pin);
    commHandler.outputMemMap.output_registers.estop = HAL_GPIO_ReadPin(ESTOP_GPIO_Port, ESTOP_Pin);

    return error;
    
}

void controller_start()
{
    if(commHandler.outputMemMap.output_registers.ready == 0) return;
    
    if(run == 0 && bypass == 0) // Start the controller if not already running
    {
        //Set OCP Threshold
        HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R
                , oc_to_DAC(commHandler.inputMemMap.input_registers.oc_threshold_pc));
    
        // Wait for DAC to settle
        HAL_Delay(0);
        
        // Only update setpoint, WD Timeout, OC Threshold on controller start
        // Does not allow changing those parameters on the fly, which could cause instability
        PID_SetSetpoint(&current_controller, commHandler.inputMemMap.input_registers.tracking_current);
        current_controller.wd_timeout = commHandler.inputMemMap.input_registers.wd_timeout;
        current_controller.oc_threshold = commHandler.inputMemMap.input_registers.oc_threshold_pc;

        // Set status flags
        run = 1; // Internal Run flag
        commHandler.outputMemMap.output_registers.precharging = 1;

        // Start Control loop
        HAL_TIM_Base_Start_IT(&htim1);

        // Reset Max recorded PC current
        commHandler.outputMemMap.output_registers.pc_max_current = 0.0f;

        // HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }
}

void controller_run()
{
    float current = cs_get_current();

    /* Cache pointers to reduce repeated indirections */
    Output_MemMap_t *out = &commHandler.outputMemMap;
    Input_MemMap_t *in = &commHandler.inputMemMap;

    if (bypass == 1)
    {
        if (current > out->output_registers.bp_max_current)
            out->output_registers.bp_max_current = current;
    }
    else if (bypass == 0) /* Only run if not in bypass mode */
    {
        /* Store Maximum Precharge Current */
        if (current > out->output_registers.pc_max_current)
            out->output_registers.pc_max_current = current;

        if (current > current_controller.oc_threshold)
        {
            out->output_registers.sw_oc_fault = 1;
            out->output_registers.ready = 0;
            HAL_GPIO_WritePin(INT_GPIO_Port, INT_Pin, GPIO_PIN_SET);
            controller_stop();
            return;
        }

        /* Calculate Duty */
        float d = PID_Compute(&current_controller, current, PERIOD);

        /* Set Duty */
        TIM3->CCR4 = (uint32_t)(d * 169.0f);

        /* D = 1, wait for handover */
        if (TIM3->CCR4 == 169) ++current_controller.handover_counter;
        else current_controller.handover_counter = 0;

        /* D = 1 for long enough, handover to Bypass */
        if (current_controller.handover_counter > HANDOVER_THRESHOLD)
        {
            /* Set bypass mode */
            HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
                oc_to_DAC(in->input_registers.oc_threshold_bp));
            bypass = 1;
            HAL_GPIO_WritePin(DRV_BP_GPIO_Port, DRV_BP_Pin, GPIO_PIN_SET);
            out->output_registers.done = 1;
            out->output_registers.last_pc_time = PERIOD_US * current_controller.wd_counter;
            out->output_registers.precharging = 0;
            HAL_GPIO_WritePin(INT_GPIO_Port, INT_Pin, GPIO_PIN_SET);

            /* Disable controller */
            // HAL_TIM_Base_Stop_IT(&htim1);
            // TIM3->CCR4 = 0;
            PID_Reset(&current_controller);
            return;
        }

        /* Check for WD */
        ++current_controller.wd_counter;
        if (current_controller.wd_counter > current_controller.wd_timeout * FREQ_MHZ)
        {
            /* Disable controller */
            HAL_TIM_Base_Stop_IT(&htim1);
            TIM3->CCR4 = 0;
            PID_Reset(&current_controller);

            out->output_registers.ready = 0;
            out->output_registers.wd_timeout_fault = 1;
            out->output_registers.precharging = 0;
            HAL_GPIO_WritePin(INT_GPIO_Port, INT_Pin, GPIO_PIN_SET);
            return;
        }
    }
}

void controller_stop()
{
    
    //Stop Precharge
    run = 0;
    HAL_TIM_Base_Stop_IT(&htim1);
    TIM3->CCR4 = 0;
    PID_Reset(&current_controller);

    //Stop Bypass
    bypass = 0;
    HAL_GPIO_WritePin(DRV_BP_GPIO_Port, DRV_BP_Pin, GPIO_PIN_RESET);
    commHandler.outputMemMap.output_registers.precharging = 0;
    commHandler.outputMemMap.output_registers.done = 0;

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}
