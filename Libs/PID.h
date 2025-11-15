#pragma once

#include "main.h"
#include "CurrentSense.h"

#define ENABLE_PID

#define HANDOVER_THRESHOLD 1000.0f // Threshold to switch to bypass mode

#define PERIOD 1E-5f //10us
#define KP 0.01f
#define KI 5.0f
#define KD 0.00f
#define OUTPUT_MIN_I 0.0f
#define OUTPUT_MAX_I 1.0f
#define CURRENT_SETPOINT 3.0f // Target precharge current in Amperes

typedef struct {
    float Kp;       // Proportional gain
    float Ki;       // Integral gain
    float Kd;       // Derivative gain
    float setpoint; // Desired target value
    float integral; // Integral term
    float prevError;// Previous error value
    float outputMin;// Minimum output limit
    float outputMax;// Maximum output limit
} PIDController;


/**
 * @brief  Compute the PID controller output.
 * @param  pid Pointer to PIDController structure
 * @param  measurement Current measured value
 * @param  dt Time delta since last computation
 * @retval Control output
 */
float PID_Compute(PIDController* pid, float measurement, float dt);

/**
 * @brief  Set the PID controller setpoint.
 * @param  pid Pointer to PIDController structure
 * @param  setpoint Desired target value
 * @retval None
 */
void PID_SetSetpoint(PIDController* pid, float setpoint);

/**
 * @brief  Reset the PID controller internal state.
 * @param  pid Pointer to PIDController structure
 * @retval None
 */
void PID_Reset(PIDController* pid);

/**
 * @brief  Initialize the controller.
 * @param  None
 * @retval None
 */
void controller_init();

/**
 * @brief  Start the controller. This starts the controller interrup routine.
 * @param  None
 * @retval None
 */
void controller_start();

/**
 * @brief  Stop the controller and bypass mode if active.
 * @param  None
 * @retval None
 */
void controller_stop();

/**
 * @brief  Run the controller loop. This function is called periodically by the controller interrupt routine.
 * @param  None
 * @retval None
 */
void controller_run();

extern PIDController current_controller;
extern uint8_t run;
extern uint8_t bypass;
