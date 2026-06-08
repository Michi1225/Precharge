#ifndef COMM_H
#define COMM_H


#include "main.h"
#include "stm32g4xx_hal_i2c.h"


#define I2C_HANDLER hi2c2

#define MAX_READ_ADDR 0x19 // Maximum valid register address for reading, adjust as needed
#define MAX_WRITE_ADDR 0x10 // Maximum valid register address for writing, adjust as needed

#define PC_STATUS_REG 0x00
#define PC_LAST_PC_TIME_REG 0x01
#define PC_MAX_PC_CURRENT 0x05
#define PC_MAX_BP_CURRENT 0x09
#define PC_SETPOINT_REG 0x0D
#define PC_TIMEOUT_REG 0x11
#define PC_VOLTAGE_REG 0x15
#define PC_TEMPERATURE_REG 0x19

// Function prototypes for communication functions

typedef struct
{
    union 
    {
        uint8_t raw_output_data[256];

        struct __attribute__((packed))
        {
            //0x00 Status Register
            uint8_t precharging :1;
            uint8_t ready :1;
            uint8_t done :1;
            uint8_t hw_oc_fault :1;
            uint8_t sw_oc_fault :1;
            uint8_t wd_timeout_fault :1;
            uint8_t init_failed :1;
            uint8_t estop :1;
        
            //0x01 Last PC time (us)
            uint32_t last_pc_time;
        
            //0x05 PC Max Current
            float pc_max_current;
        
            //0x09 BP Max Current
            float bp_max_current;
        
            //0x0D PC Setpoint
            float pc_setpoint;
        
            //0x11 PC Timeout
            float pc_timeout;
        
            //0x15 Voltage
            float voltage;
        
            //0x19 Temperature
            uint8_t temperature;
        } output_registers;


    };
}Output_MemMap_t;


typedef struct 
{
    union 
    {
        uint8_t raw_input_data[256];

        struct __attribute__((packed))
        {
            //0x00-0x03 Tracking Current
            float tracking_current;
        
            //0x04 WD Timeout (us)
            uint32_t wd_timeout;
        
            //0x08 Fault Clear
            uint8_t fault_clear;
        
            //0x09 OC Threshold PC
            float oc_threshold_pc;
        
            //0x0D OC Threshold BP
            float oc_threshold_bp;
        } input_registers;
    };
} Input_MemMap_t;


typedef struct
{
    /* data */
    Output_MemMap_t outputMemMap;
    Input_MemMap_t inputMemMap;

}Communication_Handler_t;

extern Communication_Handler_t commHandler;

HAL_StatusTypeDef comm_init();

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c);




#endif // COMM_H