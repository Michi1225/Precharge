#include "comm.h"  
#include "crc.h"
#include "stm32g4a1xx.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_i2c.h"


enum {
    MASTER_REG,
    MASTER_WRITE,
    MASTER_READ
} commState = MASTER_REG;


uint8_t currentRegister = 0xFF; //Invalid register address, next write must be register address
uint8_t input_data_raw[256]; // Buffer for received data, adjust size as needed
uint8_t output_data_raw[256]; // Buffer for data to send, adjust size as needed
uint8_t dummy = 0;

uint8_t rxcnt = 0; // Counter for received bytes, adjust as needed
uint8_t txcnt = 0; // Counter for transmitted bytes, adjust as needed


Communication_Handler_t commHandler = {
    .inputMemMap = {
        .input_registers.tracking_current = 1.0f,
        .input_registers.wd_timeout = 1E6,
        .input_registers.fault_clear = 0,
        .input_registers.oc_threshold_pc = 10.0f,
        .input_registers.oc_threshold_bp = 50.0f
    },
    .outputMemMap = {
        .output_registers.precharging = 0,
        .output_registers.ready = 0,
        .output_registers.done = 0,
        .output_registers.hw_oc_fault = 0,
        .output_registers.sw_oc_fault = 0,
        .output_registers.wd_timeout_fault = 0,
        .output_registers.init_failed = 0,
        .output_registers.estop = 0,

        .output_registers.last_pc_time = 0,
        .output_registers.pc_max_current = 0.0f,
        .output_registers.bp_max_current = 0.0f,
        .output_registers.pc_setpoint = 1.0f,
        .output_registers.pc_timeout = 1E6,
        .output_registers.voltage = 0.0f,
        .output_registers.temperature = 25
    }
};

HAL_StatusTypeDef comm_init()
{
    return HAL_I2C_EnableListen_IT(&hi2c2);
}

void comm_reset_faults() 
{
    // Clear Overcurrent Faults
    commHandler.outputMemMap.output_registers.sw_oc_fault = 0;
    commHandler.outputMemMap.output_registers.hw_oc_fault = 0;
    HAL_GPIO_WritePin(nCLR_OC_GPIO_Port, nCLR_OC_Pin, GPIO_PIN_RESET);
    for(int i = 0; i < 1E5; ++i) __NOP();
    HAL_GPIO_WritePin(nCLR_OC_GPIO_Port, nCLR_OC_Pin, GPIO_PIN_SET);
    
    // Clear E-Stop Fault
    commHandler.outputMemMap.output_registers.estop = 0;
    HAL_GPIO_WritePin(nCLR_ESTOP_GPIO_Port, nCLR_ESTOP_Pin, GPIO_PIN_RESET);
    for(int i = 0; i < 1E5; ++i) __NOP();
    HAL_GPIO_WritePin(nCLR_ESTOP_GPIO_Port, nCLR_ESTOP_Pin, GPIO_PIN_SET);

    // Clear Watchdog Fault
    commHandler.outputMemMap.output_registers.wd_timeout_fault = 0;
    
    // Set Ready flag unless init failed
    if(commHandler.outputMemMap.output_registers.init_failed != 1)
    {
        commHandler.outputMemMap.output_registers.ready = 1;
    }
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
                          uint16_t AddrMatchCode) {
  // Handle I2C address match event
  if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
  {
      if (TransferDirection == I2C_DIRECTION_TRANSMIT) // Master is writing to slave
      {
          
          
      switch(commState)
      {
        case MASTER_REG:
          // Expecting register address next
          // Receive one byte of data => register address
          HAL_I2C_Slave_Seq_Receive_IT(hi2c, &currentRegister, 1, I2C_NEXT_FRAME);
          break;
        case MASTER_WRITE:
          //TODO: This case should not happen, but if it does just receive into dummy variable until STOP condition
          // Master writes data for the previously specified register
          if (currentRegister + rxcnt < MAX_WRITE_ADDR) // Check if register address is valid
          {
              HAL_I2C_Slave_Seq_Receive_IT(hi2c, &input_data_raw[currentRegister + rxcnt], 1, I2C_NEXT_FRAME); 
          }
          break;
      }
    } else // Repeated START means master wants to read the specified register
    {
      // Send 1 byte of data from the specified register address
      if (currentRegister < MAX_READ_ADDR) // Check if register address is valid
      {
          commState = MASTER_READ;
          memcpy(output_data_raw, commHandler.outputMemMap.raw_output_data, MAX_READ_ADDR);
          HAL_I2C_Slave_Seq_Transmit_IT(hi2c, output_data_raw + currentRegister, 1, I2C_NEXT_FRAME);
      }
    }
  }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // One byte of data has been received from master
    if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
    {

        switch(commState)
        {
            case MASTER_REG:
                // Just received register address, stored in currentRegister
                commState = MASTER_WRITE; // Next reception will be data for the register or repeated start for read
                break;
            case MASTER_WRITE:
                // Master write data received
                ++rxcnt; // Increment received byte count
                break;
        }
        // Continue receiving data if needed
        if (commState == MASTER_WRITE && currentRegister + (rxcnt -1) <= MAX_WRITE_ADDR + 1) // Check if next register address is valid (+1 for CRC Byte)
        {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, &input_data_raw[currentRegister + rxcnt], 1, I2C_NEXT_FRAME);
        }
        else
        {
            // Invalid Master Write, just receive into dummy variable until STOP condition
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, &dummy, 1, I2C_NEXT_FRAME);
        }
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Handle completion of data transmission to master
    if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
    {
        // One Byte of data has been transmitted to master from output_data_raw[currentRegister]
        // If more data needs to be sent for this register, continue transmitting
        if (commState == MASTER_READ && currentRegister + txcnt < MAX_READ_ADDR) // Check if next register address is valid
        {
            ++txcnt; // Increment transmitted byte count
            HAL_I2C_Slave_Seq_Transmit_IT(hi2c, output_data_raw + currentRegister + txcnt, 1, I2C_NEXT_FRAME);
        }else
        {
            // Invalid Master Read, just send dummy data until STOP condition
            dummy = 0;
            HAL_I2C_Slave_Seq_Transmit_IT(hi2c, &dummy, 1, I2C_NEXT_FRAME);
             
        }
    }
}
uint32_t xfercnt = 0;

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Handle completion of listen mode (if used)
    if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
    {
        // Restart listening for new I2C communication
        
        if(commState == MASTER_WRITE) // Just finished receiving data from master
        {
            // Validate receive Data & copy to register if valid

            
            
            uint32_t crc = HAL_CRC_Calculate(&hcrc, &input_data_raw[currentRegister], rxcnt - 1); // Example CRC calculation, adjust as needed
            if((crc & 0x000000FF) == input_data_raw[currentRegister + rxcnt - 1])
            {
                for(int index = currentRegister; index < currentRegister + rxcnt - 1;)
                {
                    PC_InputRegister_t reg = (PC_InputRegister_t) index;
                    float input_value_f32;
                    uint32_t input_value_u32;
                    switch (reg)
                    {
                    case PC_TRACKING_CURRENT:
                        input_value_f32 = *((float*)(input_data_raw + index));
                        if(input_value_f32 > PC_TRACKING_CURRENT_MIN_VAL && input_value_f32 < PC_TRACKING_CURRENT_MAX_VAL)
                        {
                            memcpy((void*)(commHandler.inputMemMap.raw_input_data + index), &input_value_f32, sizeof(input_value_f32));
                            commHandler.outputMemMap.output_registers.pc_setpoint = commHandler.inputMemMap.input_registers.tracking_current;
                        }
                        index += sizeof(input_value_f32);
                        break;

                    case PC_WD_TIMEOUT:
                        input_value_u32 = *((uint32_t*)(input_data_raw + index));
                        if(input_value_u32 > PC_WD_TIMEOUT_MIN_VAL && input_value_u32 < PC_WD_TIMEOUT_MAX_VAL)
                        {
                            memcpy((void*)(commHandler.inputMemMap.raw_input_data + index), &input_value_u32, sizeof(input_value_u32));
                        }
                        index += sizeof(input_value_u32);
                        break;
                    
                    case PC_CLR_FLT:
                        if(HAL_GPIO_ReadPin(nEN_GPIO_Port, nEN_Pin) == GPIO_PIN_RESET)
                        {
                            index++;
                            break;
                        }
                        if (input_data_raw[index] == PC_CLR_FLT_VAL)
                        {

                            comm_reset_faults();
                        }
                        index++;
                        break;

                    case PC_OC_THRESHOLD_PC:
                        input_value_f32 = *((float*)(input_data_raw + index));
                        if(input_value_f32 > PC_OC_THRESHOLD_PC_MIN_VAL && input_value_f32 < PC_OC_THRESHOLD_PC_MAX_VAL)
                        {
                            memcpy((void*)(commHandler.inputMemMap.raw_input_data + index), &input_value_f32, sizeof(input_value_f32));
                        }
                        index += sizeof(input_value_f32);
                        break;

                    case PC_OC_THRESHOLD_BP:
                        input_value_f32 = *((float*)(input_data_raw + index));
                        if(input_value_f32 > PC_OC_THRESHOLD_BP_MIN_VAL && input_value_f32 < PC_OC_THRESHOLD_BP_MAX_VAL)
                        {
                            memcpy((void*)(commHandler.inputMemMap.raw_input_data + index), &input_value_f32, sizeof(input_value_f32));
                        }
                        index += sizeof(input_value_f32);
                        break;
                    
                    default:
                        index += rxcnt; //TODO: figure out a better way to do this
                        break;
                    }
                }
            }
        }
        else if(commState == MASTER_READ) // Just finished transmitting data to master
        {


        }
        commState = MASTER_REG; // Reset to expect register address again
        rxcnt = 0; // Reset received byte count for next communication
        txcnt = 0; // Reset transmitted byte count for next communication
        HAL_GPIO_WritePin(INT_GPIO_Port, INT_Pin, GPIO_PIN_RESET); // Master has read 

        // Rearm Listen mode to be ready for next communication
        HAL_I2C_EnableListen_IT(hi2c);
    }
}