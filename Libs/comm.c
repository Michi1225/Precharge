#include "comm.h"  
#include "crc.h"
#include "stm32g4xx_hal_i2c.h"


enum {
    MASTER_REG,
    MASTER_WRITE,
    MASTER_READ
} commState = MASTER_REG;

static Output_MemMap_t outputMemMap; // This will hold the output registers in a structured format, adjust as needed
static Input_MemMap_t inputMemMap; // This will hold the input registers in a structured format, adjust as needed

uint8_t currentRegister = 0xFF; //Invalid register address, next write must be register address
uint8_t input_data_raw[256]; // Buffer for received data, adjust size as needed
uint8_t output_data_raw[256]; // Buffer for data to send, adjust size as needed

uint8_t rxcnt = 0; // Counter for received bytes, adjust as needed


Communication_Handler_t commHandler = {};

HAL_StatusTypeDef comm_init()
{
    return HAL_I2C_EnableListen_IT(&hi2c2);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection,
                          uint16_t AddrMatchCode) {
  // Handle I2C address match event
  if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
  {
    if (TransferDirection == I2C_DIRECTION_TRANSMIT) // Master is writing to slave
    {
      // Prepare to receive data from master
      // You can set up a buffer and call HAL_I2C_Slave_Receive_DMA here

      // Master writes register address.
      if (commState == MASTER_REG) // Expecting register address
      {
        // Set up to receive the register address
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, &currentRegister, 1, I2C_NEXT_FRAME);
      } else {
        // Set up to receive data for the specified register
        if (currentRegister + rxcnt < MAX_WRITE_ADDR) // Check if register address is valid
        {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, &input_data_raw[currentRegister + rxcnt], 1, I2C_NEXT_FRAME);
        }
        // Process received data and update the corresponding register value
      }
    } else // Master is reading from slave
    {
      // Prepare data to send to master
      // You can set up a buffer with the data you want to send and call
      // HAL_I2C_Slave_Transmit_DMA here
      HAL_I2C_Slave_Seq_Transmit_IT(hi2c, output_data_raw + currentRegister, 1, I2C_NEXT_FRAME);
    }
  }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Handle completion of data reception from master
    if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
    {
        if(commState == MASTER_REG) // Just received register address
        {
            commState = MASTER_WRITE; // Next reception will be data for the register
        }
        else
        {
            // Master write data received
            ++rxcnt; // Increment received byte count, adjust as needed
        }
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, &input_data_raw[currentRegister + rxcnt], 1, I2C_NEXT_FRAME); // Continue receiving data if needed
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Handle completion of data transmission to master
    if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
    {
        // Transmission complete, you can perform any necessary cleanup or prepare for the next transmission
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, output_data_raw + currentRegister, 1, I2C_NEXT_FRAME); // Continue transmitting data if needed
    }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Handle completion of listen mode (if used)
    if (hi2c == &I2C_HANDLER) // Check if it's the correct I2C instance
    {
        // Restart listening for new I2C communication
        commState = MASTER_REG; // Reset to expect register address again

        // Validate receive Data & copy to register if valid
        uint32_t crc = HAL_CRC_Calculate(&hcrc, &input_data_raw[currentRegister], rxcnt - 1); // Example CRC calculation, adjust as needed
        if((crc & 0x000000FF) == input_data_raw[currentRegister + rxcnt - 1]) // Dummy validation
        {
            for(int index = currentRegister; index < currentRegister + rxcnt; ++index)
            {
                // Update the corresponding register value based on received data
                // This is where you would parse input_data_raw and update your inputMemMap accordingly
                commHandler.inputMemMap.raw_input_data[index] = input_data_raw[index];
            }
        }
        rxcnt = 0; // Reset received byte count for next communication

        HAL_I2C_EnableListen_IT(hi2c);
    }
}