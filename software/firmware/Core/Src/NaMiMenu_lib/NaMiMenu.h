/**
 ****************************************************************************************
 *
 * \file SSD1322_OLED.h
 *
 * \brief
 *
 * Copyright (c) 2025 KN NaMi
 *
 ****************************************************************************************
 */

#ifndef NaMiMenu_H
#define NaMiMenu_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_API.h"
#include "../NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_GFX.h"
#include "../NaMiMenu_lib/SSD1322_OLED_lib/SSD1322_HW_Driver.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define NAMIMENU_BRIGHTNESS_PRIMARY 10
#define NAMIMENU_BRIGHTNESS_SECONDARY 7
#define NAMIMENU_BRIGHTNESS_TETRIARY 4

#define NAMIMENU_CURSOR_UP 1
#define NAMIMENU_CURSOR_DOWN 0

typedef struct State State;
typedef struct Clickable Clickable;
typedef struct Chart Chart;
typedef struct ProgressBar ProgressBar;

typedef struct State {
	uint8_t state;
	void (*callback)();
	Clickable *objects;
	uint8_t object_cnt;
} State;

typedef struct Clickable {
	uint8_t nextState;
	void (*onClick)(State *state, Clickable *clickable);
	const char *text;
} Clickable;

typedef struct Chart {
	double *data;
	uint16_t dataSize;
	double maxValue;
	double minValue;
} Chart;

typedef struct ProgressBar {
	uint8_t value;
	uint8_t sections;
} ProgressBar;

/* ================= SPI & Encoder data ================= */

extern SPI_HandleTypeDef* NaMiMenu_OLED_hspi;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} NaMiMenu_GPIO;

extern NaMiMenu_GPIO NaMiMenu_OLED_CS;
extern NaMiMenu_GPIO NaMiMenu_OLED_DC;
extern NaMiMenu_GPIO NaMiMenu_OLED_RESET;
extern NaMiMenu_GPIO NaMiMenu_ENC_SIGA;
extern NaMiMenu_GPIO NaMiMenu_ENC_SIGB;
extern NaMiMenu_GPIO NaMiMenu_ENC_BTN;

// ====================== NaMi Abstraction Level ====================== //

void NAL_NaMiMenu_OLED_HW_setup(GPIO_TypeDef *CS_Port, uint16_t CS_Pin, GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *RESET_Port, uint16_t RESET_Pin, SPI_HandleTypeDef* hspi);
void NAL_NaMiMenu_Encoder_setup(GPIO_TypeDef *BTN_Port, uint16_t BTN_Pin, GPIO_TypeDef *SIGA_Port, uint16_t SIGA_Pin, GPIO_TypeDef *SIGB_Port, uint16_t SIGB_Pin);
void NAL_NaMiMenu_OLED_Update();
void NAL_NaMiMenu_Encoder_Update(uint16_t GPIO_Pin);
void NAL_NaMiMenu_Cursor_up();
void NAL_NaMiMenu_Cursor_down();
void NAL_NaMiMenu_SetCursorPos(uint8_t pos);
void NAL_NaMiMenu_Encoder_Pressed();
void NAL_NaMiMenu_init(GPIO_TypeDef *CS_Port, uint16_t CS_Pin, GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *RESET_Port, uint16_t RESET_Pin, SPI_HandleTypeDef *hspi);
void NAL_NaMiMenu_setMenuStates(State *menuStates, uint8_t size);
//
void NAL_NaMiMenu_splash_screen(const char* name, const char* version);
void Transision(uint8_t nextState);
uint8_t NAL_NaMiMenu_IsCursorFrozen();
uint8_t NAL_NaMiMenu_CursorDir();
uint8_t NAL_NaMiMenu_CursorPos();
void NAL_NaMiMenu_Cursor_freeze();
void NAL_NaMiMenu_Cursor_unfreeze();
void NAL_NaMiMenu_Cursor_toggle_freeze();
State* NAL_NaMiMenu_CurrentState();
//
void NAL_NaMiMenu_fill_buffer(uint8_t brightness);
void NAL_NaMiMenu_clear_buffer();
void NAL_NaMiMenu_send_buffer_to_OLED();
//
void NAL_draw_pixel(uint16_t x, uint16_t y, uint8_t brightness);
void NAL_NaMiMenu_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t brightness);
void NAL_NaMiMenu_draw_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t x2, uint8_t brightness);
void NAL_NaMiMenu_draw_rect_filled(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t x2, uint8_t brightness);
void NAL_NaMiMenu_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t brightness);
void NAL_NaMiMenu_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t r, uint8_t brightness);
void NAL_NaMiMenu_draw_text(const char* text, uint16_t x, uint16_t y, uint8_t brightness);
void NAL_draw_text_in_grid(const char* text, uint16_t x, uint16_t y, uint8_t brightness);
//
void NAL_NaMiMenu_draw_up_arrow(uint16_t x, uint16_t y, uint8_t brightness);
void NAL_NaMiMenu_draw_down_arrow(uint16_t x, uint16_t y, uint8_t brightness);
void NAL_NaMiMenu_draw_left_arrow(uint16_t x, uint16_t y, uint8_t brightness);
void NAL_NaMiMenu_draw_right_arrow(uint16_t x, uint16_t y, uint8_t brightness);
//
void NAL_NaMiMenu_draw_bitmap_4bpp(const uint8_t *bitmap, uint16_t x0, uint16_t y0, uint16_t x_size, uint16_t y_size);
//
void NAL_draw_indicator_in_grid(uint8_t status, uint16_t x, uint16_t y, uint8_t brightness);
void NAL_draw_progress_bar(ProgressBar *progressBar);
void NAL_draw_chart_in_grid(Chart *chart, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t brightness);
//
void NAL_NaMiMenu_OLEDUpdate_Callback(State *state, Clickable *clickable);
void NAL_CursorMove_Callback(State *state, Clickable *clickable);
void NAL_FrozenCursorMove_Callback(State *state, Clickable *clickable);

#ifdef __cplusplus
}
#endif

#endif
