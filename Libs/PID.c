#include "PID.h"

PIDController current_controller =
{
    .Kp = KP,
    .Ki = KI,
    .Kd = KD,
    .setpoint = 0.0f,
    .integral = 0.0f,
    .prevError = 0.0f,
    .outputMin = OUTPUT_MIN_I,
    .outputMax = OUTPUT_MAX_I
};

uint8_t run = 0;
uint8_t bypass = 0;


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
}


void controller_init()
{
    run = 0;
    bypass = 0;
    PID_SetSetpoint(&current_controller, CURRENT_SETPOINT);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    TIM3->CCR4 = 0;

    //Reset OC latch
    HAL_GPIO_WritePin(nCLR_OC_GPIO_Port, nCLR_OC_Pin, GPIO_PIN_RESET);
    HAL_Delay(9);
    HAL_GPIO_WritePin(nCLR_OC_GPIO_Port, nCLR_OC_Pin, GPIO_PIN_SET);

    //Controller ready
    HAL_GPIO_WritePin(RDY_GPIO_Port, RDY_Pin, GPIO_PIN_RESET); //TODO: invert logic (LED ON = DONE LOW)
    HAL_GPIO_WritePin(DONE_GPIO_Port, DONE_Pin, GPIO_PIN_SET); //TODO: invert logic (LED ON = DONE LOW)
  
    
}

void controller_start()
{
    if(run == 0 && bypass == 0) // Start the controller if not already running
    {
        run = 1; // Set run flag
        HAL_TIM_Base_Start_IT(&htim1); // Start timer interrupt for control loop
    }
}

void controller_run()
{




    if(bypass == 0) // Only run if not in bypass mode
    {
        float d = PID_Compute(&current_controller, cs_get_pc_current(), PERIOD);
        TIM3->CCR4 = (uint32_t)(d * 169.0f);
        //TODO: Handover to bypass logic
        // if(d > HANDOVER_THRESHOLD) // Enter bypass mode if output exceeds threshold
        // {
        //     //Set bypass mode
        //     bypass = 1;
        //     HAL_GPIO_WritePin(DRV_BP_GPIO_Port, DRV_BP_Pin, GPIO_PIN_SET);
        //     HAL_GPIO_WritePin(DONE_GPIO_Port, DONE_Pin, GPIO_PIN_RESET); //TODO: invert logic (LED ON = DONE LOW)

        //     //Disable controller
        //     HAL_TIM_Base_Stop_IT(&htim1);
        //     TIM3->CCR4 = 0;
        //     PID_Reset(&current_controller);
        // }
        return;
    }
}

void controller_stop()
{
    if(run == 1 || bypass == 1) // Stop the controller if running or in bypass mode
    {
        //Stop Precharge
        run = 0;
        HAL_TIM_Base_Stop_IT(&htim1);
        TIM3->CCR4 = 0;
        PID_Reset(&current_controller);

        //Stop Bypass
        bypass = 0;
        HAL_GPIO_WritePin(DRV_BP_GPIO_Port, DRV_BP_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(DONE_GPIO_Port, DONE_Pin, GPIO_PIN_SET); //TODO: invert logic (LED ON = DONE LOW)
    }
}
