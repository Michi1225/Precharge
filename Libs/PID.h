#pragma once

#include "main.h"
#include "CurrentSense.h"

#define ENABLE_PID

#define HANDOVER_THRESHOLD 1000.0f // Threshold to switch to bypass mode

#define PERIOD 1E-5f //10us
#define KP 0.01f
#define KI 1.0f
#define KD 0.00f
#define OUTPUT_MIN_I 0.0f
#define OUTPUT_MAX_I 1.0f
#define CURRENT_SETPOINT 0.50f

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

float PID_Compute(PIDController* pid, float measurement, float dt);
void PID_SetSetpoint(PIDController* pid, float setpoint);
void PID_Reset(PIDController* pid);

void controller_init();
void controller_start();
void controller_run();
void controller_stop();

extern PIDController current_controller;
extern uint8_t run;
extern uint8_t bypass;
