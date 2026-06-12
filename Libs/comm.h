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


#define PC_TRACKING_CURRENT_MAX_VAL 5.0f
#define PC_TRACKING_CURRENT_MIN_VAL 0.0f

#define PC_WD_TIMEOUT_MAX_VAL 5E6
#define PC_WD_TIMEOUT_MIN_VAL 0

#define PC_CLR_FLT_VAL 0x5D

#define PC_OC_THRESHOLD_PC_MAX_VAL 15.0f
#define PC_OC_THRESHOLD_PC_MIN_VAL 5.0f

#define PC_OC_THRESHOLD_BP_MAX_VAL 60.0f
#define PC_OC_THRESHOLD_BP_MIN_VAL 0.0f

// Function prototypes for communication functions

typedef enum 
{
    PC_TRACKING_CURRENT = 0x00,
    PC_WD_TIMEOUT = 0x04,
    PC_CLR_FLT = 0x08,
    PC_OC_THRESHOLD_PC = 0x09,
    PC_OC_THRESHOLD_BP = 0x0D
}PC_InputRegister_t;

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
            uint32_t pc_timeout;
        
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
void comm_reset_faults();


/**
 * @brief  I2C Address Match callback. Called when the slave address is matched.
 *         This occurrs on START/REPEATED START conditions.
 * @param  hi2c Pointer to I2C handle
 * @param  TransferDirection Master to Slave or Slave to Master
 * @param  AddrMatchCode Address match code
 */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode);


/**
 * @brief  I2C Slave Receive Complete callback. Called when the slave has received one byte of data from the master.
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);

/**
 * @brief  I2C Slave Transmit Complete callback. Called when the slave has transmitted one byte of data to the master.
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);

/**
 * @brief  I2C Listen Complete callback. Called when the slave has completed listening for I2C communication.
 *         This happens on a STOP condition
 * @param  hi2c Pointer to I2C handle
 */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c);

/*
 * Example Master register write sequence:
 * START => Register Address => master_tx_data[n] => LSB of CRC32(data)
 *
 * Example Master register read sequence:
 * START => Register Address => REPEATED START => slave_tx_data[n]
 */




#endif // COMM_H