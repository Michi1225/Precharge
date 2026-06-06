#ifndef COMM_H
#define COMM_H


#include "main.h"
#include "stm32g4xx_hal_i2c.h"


#define I2C_HANDLER hi2c2

#define MAX_READ_ADDR 0x07 // Maximum valid register address for reading, adjust as needed
#define MAX_WRITE_ADDR 0x04 // Maximum valid register address for writing, adjust as needed



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
        
            //0x01 Last PC time
            uint8_t last_pc_time;
        
            //0x02 PC Max Current
            uint8_t pc_max_current;
        
            //0x03 BP Max Current
            uint8_t bp_max_current;
        
            //0x04 PC Setpoint
            uint8_t pc_setpoint;
        
            //0x05 PC Timeout
            uint8_t pc_timeout;
        
            //0x06 Voltage
            uint8_t voltage;
        
            //0x07 Temperature
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
            //0x00-0x01 Tracking Current
            uint16_t tracking_current;
        
            //0x02 WD Timeout
            uint8_t wd_timeout;
        
            //0x03 Fault Clear
            uint8_t fault_clear;
        
            //0x04 OC Threshold PC
            uint8_t oc_threshold_pc;
        
            //0x05 OC Threshold BP
            uint8_t oc_threshold_bp;
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