//
// Created by redmiX on 2026/4/6.
//

//******************************** Includes *********************************//
#include "st7789_system_adaption.h"
#include "DWT_delay.h"
#include "main.h"
#include "spi.h"
#include "st7789_driver.h"
#include "stm32f4xx_hal.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
static inline uint8_t spi_transmit_data(const uint8_t *pData, uint32_t dataLength);
static inline uint8_t spi_transmit_data_dma(const uint8_t *pData, uint32_t dataLength);
static inline uint8_t gpio_write_reset_pin(uint8_t pinState);
static inline uint8_t gpio_write_cs_pin(uint8_t pinState);
static inline uint8_t gpio_write_dc_pin(uint8_t pinState);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
static basic_oper_driver_interface_t st7789_oper_instance = {
    .pf_spi_transmit = spi_transmit_data,
    .pf_spi_transmit_dma = spi_transmit_data_dma,
    .pf_write_cs_pin = gpio_write_cs_pin,
    .pf_write_dc_pin = gpio_write_dc_pin,
    .pf_write_reset_pin = gpio_write_reset_pin
};

static st7789_timebase_interface_t st7789_time_instance = {
    .pf_delay_no_os = DWT_Delay_ms
};

bsp_st7789_driver_t st7789_driver_instance;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
static inline uint8_t spi_transmit_data(const uint8_t *pData, uint32_t dataLength)
{
    HAL_SPI_Transmit(&hspi1, pData, dataLength, 0xFF);
    return 0;
}

static inline uint8_t spi_transmit_data_dma(const uint8_t *pData, uint32_t dataLength)
{
    HAL_SPI_Transmit_DMA(&hspi1, pData, dataLength);
    while (hspi1.hdmatx->State != HAL_DMA_STATE_READY);

    return 0;
}

static inline uint8_t gpio_write_reset_pin(uint8_t pinState)
{
    if(0 == pinState)
    {
        HAL_GPIO_WritePin(SPI_RESET_GPIO_Port, SPI_RESET_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(SPI_RESET_GPIO_Port, SPI_RESET_Pin, GPIO_PIN_SET);
    }
    return pinState;
}

static inline uint8_t gpio_write_cs_pin(uint8_t pinState)
{
    if(0 == pinState)
    {
        HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
    }
    return pinState;
}

static inline uint8_t gpio_write_dc_pin(uint8_t pinState)
{
    if(0 == pinState)
    {
        HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(SPI_DC_GPIO_Port, SPI_DC_Pin, GPIO_PIN_SET);
    }
    return pinState;
}

void st7789_system_adaption(void)
{
    st7789_driver_inst(&st7789_driver_instance, &st7789_time_instance, &st7789_oper_instance);
}