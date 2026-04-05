/**
 ****************************************************************************************
 *
 * \file SSD1322_HW_Driver.c
 *
 * \brief Hardware dependent functions for SSD1322 OLED display.
 *
 * This file contains functions that rely on hardware of used MCU. In this example functions
 * are filled with STM32F411RE hardware implementation. To use this library on any other MCU
 * you just have to provide its hardware implementations of functions from this file and higher
 * level functions should work without modification.
 *
 * Copyright (c) 2025 KN NaMi
 *
 * Copyright (C) 2020 Wojciech Klimek
 * MIT license:
 * https://github.com/wjklimek1/SSD1322_OLED_library
 *
 ****************************************************************************************
 */

//=================== Include hardware HAL libraries =====================//
#include "stm32g4xx_hal.h"
#include "main.h"
//#include "spi.h"

#include "../SSD1322_OLED_lib/SSD1322_HW_Driver.h"
#include "../../NaMiMenu_lib/NaMiMenu.h"

//====================== CS pin low ========================//
/**
 *  @brief Drives CS (Chip Select) pin of SPI interface low.
 *
 *  CS pin may be also signed as "SS" or "NSS"
 */
void SSD1322_HW_drive_CS_low()
{
	HAL_GPIO_WritePin(NaMiMenu_OLED_CS.port, NaMiMenu_OLED_CS.pin, 0);
}

//====================== CS pin high ========================//
/**
 *  @brief Drives CS (Chip Select) pin of SPI interface high.
 *
 *  CS pin may be also signed as "SS" or "NSS"
 */
void SSD1322_HW_drive_CS_high()
{
	HAL_GPIO_WritePin(NaMiMenu_OLED_CS.port, NaMiMenu_OLED_CS.pin, 1);
}

//====================== DC pin low ========================//
/**
 *  @brief Drives DC (Data/Command) pin of OLED driver low.
 *
 *  High state is for data and low state is for command.
 */
void SSD1322_HW_drive_DC_low()
{
	HAL_GPIO_WritePin(NaMiMenu_OLED_DC.port, NaMiMenu_OLED_DC.pin, 0);
}

//====================== DC pin high ========================//
/**
 *  @brief Drives DC (Data/Command) pin of of OLED driver high.
 *
 *  High state is for data and low state is for command.
 */
void SSD1322_HW_drive_DC_high()
{
	HAL_GPIO_WritePin(NaMiMenu_OLED_DC.port, NaMiMenu_OLED_DC.pin, 1);
}

//====================== RESET pin low ========================//
/**
 *  @brief Drives RESET pin of of OLED driver low.
 *
 *  Logic low on RESET resets OLED driver.
 */
void SSD1322_HW_drive_RESET_low()
{
	HAL_GPIO_WritePin(NaMiMenu_OLED_RESET.port, NaMiMenu_OLED_RESET.pin, 0);
}

//====================== RESET pin high ========================//
/**
 *  @brief Drives RESET pin of of OLED driver high.
 *
 *  Logic low on RESET resets OLED driver.
 */
void SSD1322_HW_drive_RESET_high()
{
	HAL_GPIO_WritePin(NaMiMenu_OLED_RESET.port, NaMiMenu_OLED_RESET.pin, 1);
}

//====================== Send single SPI byte ========================//
/**
 *  @brief Transmits single byte through SPI interface.
 *
 *  @param[in] byte_to_transmit byte that will be transmitted through SPI interface
 */
void SSD1322_HW_SPI_send_byte(uint8_t byte_to_transmit)
{
	HAL_SPI_Transmit(NaMiMenu_OLED_hspi, &byte_to_transmit, 1, 10);
}

//====================== Send array of SPI bytes ========================//
/**
 *  @brief Transmits array of bytes through SPI interface.
 *
 *  @param[in] array_to_transmit array of bytes that will be transmitted through SPI interface
 *  @param[in] array_size amount of bytes to transmit
 */
void SSD1322_HW_SPI_send_array(uint8_t *array_to_transmit, uint32_t array_size)
{
	HAL_SPI_Transmit(NaMiMenu_OLED_hspi, array_to_transmit, array_size, 100);
}

//====================== Milliseconds delay ========================//
/**
 *  @brief Wait for x milliseconds.
 *
 *  NOTE: This function is only used in initialization sequence to pull reset down and high
 *  and later to wait for Vdd stabilization. It takes time ONLY on startup (around 200ms)
 *  and is NOT USED during normal operation.
 *
 *  @param[in] milliseconds time to wait
 */
void SSD1322_HW_msDelay(uint32_t milliseconds)
{
	HAL_Delay(milliseconds);
}
