#ifndef __I2C_BUS_H
#define __I2C_BUS_H

#include "stm32f4xx_hal.h"

typedef struct
{
    GPIO_TypeDef * I2C_SDA_PORT;
    GPIO_TypeDef * I2C_SCL_PORT;
    uint16_t I2C_SDA_PIN;
    uint16_t I2C_SCL_PIN;

}i2c_bus_t;

void I2CStart                (const i2c_bus_t *bus);
void I2CStop                 (const i2c_bus_t *bus);
unsigned char I2CWaitAck     (const i2c_bus_t *bus);
void I2CSendAck              (const i2c_bus_t *bus);
void I2CSendNotAck           (const i2c_bus_t *bus);
void I2CSendByte             (const i2c_bus_t *bus, unsigned char cSendByte);
unsigned char I2CReceiveByte (const i2c_bus_t *bus);
void I2CInit                 (const i2c_bus_t *bus);

uint8_t I2C_Write_One_Byte(i2c_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t data);
uint8_t I2C_Write_Multi_Byte(i2c_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[]);
unsigned char I2C_Read_One_Byte(i2c_bus_t *bus, uint8_t daddr,uint8_t reg);
uint8_t I2C_Read_Multi_Byte(i2c_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[]);

#endif