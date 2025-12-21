/**
 ****************************************************************************************
 *
 * \file SSD1322_OLED.c
 *
 * \brief
 *
 * Copyright (c) 2025 KN NaMi
 *
 ****************************************************************************************
 */

#include "../NaMiMenu_lib/NaMiMenu.h"

#include "../NaMiMenu_lib/SSD1322_OLED_lib/Fonts/FreeSansOblique9pt7b.h"
#include "../NaMiMenu_lib/SSD1322_OLED_lib/Fonts/FreeSans9pt7b.h"
#include "../NaMiMenu_lib/Bitmaps/NaMi_logo_4bpp_64x64.h"

uint8_t NaMiMenu_OLED_MAIN_BUFFER[OLED_WIDTH * OLED_HEIGHT / 2] = {};

uint8_t NaMiMenu_menuBrightness = NAMIMENU_BRIGHTNESS_PRIMARY;
uint8_t NaMiMenu_cursorPos = 0;
uint8_t NaMiMenu_cursorDir = 0; // 1 -> Up/Right 2 -> Down/Left
uint8_t NaMiMenu_menuOffset = 0;
uint8_t NaMiMenu_isCursorFrozen = 0;

GPIO_PinState NaMiMenu_rissingEdgeSigBState;
GPIO_PinState NaMiMenu_fallingEdgeSigBState;
uint8_t NaMiMenu_lastDir = 0; // 1 -> RIGHT/UP 0 -> LEFT/DOWN

State *NaMiMenu_currentMenuState;
State *NaMiMenu_menuStates;
uint8_t NaMiMenu_size;

SPI_HandleTypeDef* NaMiMenu_OLED_hspi;

NaMiMenu_GPIO NaMiMenu_OLED_CS;
NaMiMenu_GPIO NaMiMenu_OLED_DC;
NaMiMenu_GPIO NaMiMenu_OLED_RESET;
NaMiMenu_GPIO NaMiMenu_ENC_SIGA;
NaMiMenu_GPIO NaMiMenu_ENC_SIGB;
NaMiMenu_GPIO NaMiMenu_ENC_BTN;

// ====================== NaMi Abstraction Level ====================== //

// ====================== OLED setup ====================== //
/**
 * @brief Sets pins for 4-wire SPI communication
 *
 * @param[in] CS_Port
 *            chip select port of spi protocol
 * @param[in] CS_Pin
 *            chip select pin of spi protocol
 * @param[in] DC_Port
 *            command port of spi protocol
 * @param[in] DC_Pin
 *            command pin of spi protocol
 * @param[in] RESET_Port
 *            reset port of spi protocol
 * @param[in] RESET_Pin
 *            reset pin of spi protocol
 * @param[in] hspi
 */
void NAL_NaMiMenu_OLED_HW_setup(GPIO_TypeDef *CS_Port, uint16_t CS_Pin, GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *RESET_Port, uint16_t RESET_Pin, SPI_HandleTypeDef *hspi) {
	NaMiMenu_OLED_CS.port = CS_Port;
	NaMiMenu_OLED_CS.pin = CS_Pin;
	NaMiMenu_OLED_DC.port = DC_Port;
	NaMiMenu_OLED_DC.pin = DC_Pin;
	NaMiMenu_OLED_RESET.port = RESET_Port;
	NaMiMenu_OLED_RESET.pin = RESET_Pin;
	NaMiMenu_OLED_hspi = hspi;
}

// ====================== encoder setup ====================== //
/**
 * @brief Sets pins for encoder communication
 *
 * @param[in] BTN_Port
 *            encoders button port
 * @param[in] BTN_Pin
 *            encoders button pin
 * @param[in] SIGA_Port
 *            encoders signal A port
 * @param[in] SIGA_Pin
 *            encoders signal A pin
 * @param[in] SIGB_Port
 *            encoders signal B port
 * @param[in] SIGB_Pin
 *            encoders signal B pin
 */
void NAL_NaMiMenu_Encoder_setup(GPIO_TypeDef *BTN_Port, uint16_t BTN_Pin, GPIO_TypeDef *SIGA_Port, uint16_t SIGA_Pin, GPIO_TypeDef *SIGB_Port, uint16_t SIGB_Pin) {
	NaMiMenu_ENC_BTN.port = BTN_Port;
	NaMiMenu_ENC_BTN.pin = BTN_Pin;
	NaMiMenu_ENC_SIGA.port = SIGA_Port;
	NaMiMenu_ENC_SIGA.pin = SIGA_Pin;
	NaMiMenu_ENC_SIGB.port = SIGB_Port;
	NaMiMenu_ENC_SIGB.pin = SIGB_Pin;
}

void NAL_NaMiMenu_OLED_Update() {
	NAL_NaMiMenu_clear_buffer();

	if(NaMiMenu_currentMenuState->callback)
		NaMiMenu_currentMenuState->callback();

	if(NaMiMenu_currentMenuState->objects) {

		if(NaMiMenu_cursorPos > (3 + NaMiMenu_menuOffset)) {
			NaMiMenu_menuOffset++;
		}

		if(NaMiMenu_cursorPos < (NaMiMenu_menuOffset)) {
			NaMiMenu_menuOffset--;
		}

		for(int i = 0; i < NaMiMenu_currentMenuState->object_cnt; i++) {
			NAL_draw_text_in_grid(NaMiMenu_currentMenuState->objects[i].text, 0, i - NaMiMenu_menuOffset, NaMiMenu_menuBrightness);
		}

		NAL_NaMiMenu_draw_right_arrow(6, (11 + 4) * (NaMiMenu_cursorPos - NaMiMenu_menuOffset + 1) - 6, 3);

		if(NaMiMenu_menuOffset > 0) {
			NAL_NaMiMenu_draw_up_arrow(3, 0, 3);
		}

		if(NaMiMenu_currentMenuState->object_cnt - NaMiMenu_menuOffset > 4) {
			NAL_NaMiMenu_draw_down_arrow(3, OLED_HEIGHT - 1, 3);
		}

	}

	NAL_NaMiMenu_OLEDUpdate_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);

	NAL_NaMiMenu_send_buffer_to_OLED();
}


void NAL_NaMiMenu_Encoder_Update(uint16_t GPIO_Pin) {

	if(GPIO_Pin == NaMiMenu_ENC_BTN.pin) {
		NAL_NaMiMenu_Encoder_Pressed();
	}

	if(GPIO_Pin == NaMiMenu_ENC_SIGA.pin) {

		// Rissing edge
		if(HAL_GPIO_ReadPin(NaMiMenu_ENC_SIGA.port, NaMiMenu_ENC_SIGA.pin) == GPIO_PIN_SET) {
			NaMiMenu_rissingEdgeSigBState = HAL_GPIO_ReadPin(NaMiMenu_ENC_SIGB.port, NaMiMenu_ENC_SIGB.pin);

			if(NaMiMenu_rissingEdgeSigBState == GPIO_PIN_RESET && NaMiMenu_fallingEdgeSigBState == GPIO_PIN_SET) {
				if(NaMiMenu_lastDir == 1)
					NAL_NaMiMenu_Cursor_down();
				NaMiMenu_lastDir = 1;
			}
		}

		// Falling edge
		if(HAL_GPIO_ReadPin(NaMiMenu_ENC_SIGA.port, NaMiMenu_ENC_SIGA.pin) == GPIO_PIN_RESET) {
			NaMiMenu_fallingEdgeSigBState = HAL_GPIO_ReadPin(NaMiMenu_ENC_SIGB.port, NaMiMenu_ENC_SIGB.pin);

			if(NaMiMenu_rissingEdgeSigBState == GPIO_PIN_SET && NaMiMenu_fallingEdgeSigBState == GPIO_PIN_RESET) {
				if(NaMiMenu_lastDir == 0)
					NAL_NaMiMenu_Cursor_up();
				NaMiMenu_lastDir = 0;
			}
		}
	}
}

void NAL_NaMiMenu_Cursor_up() {
	NaMiMenu_cursorDir = 1;

	if(NaMiMenu_cursorPos > 0 && !NaMiMenu_isCursorFrozen)
		NaMiMenu_cursorPos--;

	if(NaMiMenu_isCursorFrozen) {
		NAL_FrozenCursorMove_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);
	} else {
		NAL_CursorMove_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);
	}

	NAL_NaMiMenu_OLED_Update();
}

void NAL_NaMiMenu_Cursor_down() {

	NaMiMenu_cursorDir = 0;

	uint8_t lenght = NaMiMenu_currentMenuState->object_cnt;

	if((lenght - 1) > NaMiMenu_cursorPos && !NaMiMenu_isCursorFrozen)
		NaMiMenu_cursorPos++;

	if(NaMiMenu_isCursorFrozen) {
		NAL_FrozenCursorMove_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);
	} else {
		NAL_CursorMove_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);
	}

	NAL_NaMiMenu_OLED_Update();
}

void NAL_NaMiMenu_SetCursorPos(uint8_t pos) {

	NaMiMenu_cursorPos = pos;

	if(NaMiMenu_isCursorFrozen) {
			NAL_FrozenCursorMove_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);
		} else {
			NAL_CursorMove_Callback(NaMiMenu_currentMenuState, &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos]);
		}

		NAL_NaMiMenu_OLED_Update();
}

void NAL_NaMiMenu_Encoder_Pressed() {

	if(NaMiMenu_currentMenuState->objects) {

		Clickable *temp = &NaMiMenu_currentMenuState->objects[NaMiMenu_cursorPos];

		if(temp->onClick) {
			temp->onClick(NaMiMenu_currentMenuState, temp);
			NAL_NaMiMenu_OLED_Update();
		}

		if(temp->nextState) {
			Transision(temp->nextState);
		}
	}
}

// ====================== init ====================== //
/**
 * @brief Initializes menu
 *
 * @param[in] CS_Port
 *            chip select port of spi protocol
 * @param[in] CS_Pin
 *            chip select pin of spi protocol
 * @param[in] DC_Port
 *            command port of spi protocol
 * @param[in] DC_Pin
 *            command pin of spi protocol
 * @param[in] RESET_Port
 *            reset port of spi protocol
 * @param[in] RESET_Pin
 *            reset pin of spi protocol
 * @param[in] hspi
 *
 * @paramp[in] mainMenuStates
 *
 */
void NAL_NaMiMenu_init(GPIO_TypeDef *CS_Port, uint16_t CS_Pin, GPIO_TypeDef *DC_Port, uint16_t DC_Pin, GPIO_TypeDef *RESET_Port, uint16_t RESET_Pin, SPI_HandleTypeDef *hspi) {
	NAL_NaMiMenu_OLED_HW_setup(CS_Port, CS_Pin, DC_Port, DC_Pin, RESET_Port, RESET_Pin, hspi);

	SSD1322_API_init();
	set_buffer_size(OLED_WIDTH, OLED_HEIGHT);

	select_font(&FreeSans9pt7b);

	NAL_NaMiMenu_clear_buffer();
	NAL_NaMiMenu_send_buffer_to_OLED();
}

void NAL_NaMiMenu_setMenuStates(State *menuStates, uint8_t size) {
	NaMiMenu_menuStates = menuStates;
	NaMiMenu_size = size;
}

// ====================== splash screen ====================== //
/**
 * @brief Renders to OLED basic splash screen for projects
 *
 * @param[in] name
 *            name of a project
 * @param[in] CS_Pin
 *            version of a project
 */
void NAL_NaMiMenu_splash_screen(const char* name, const char* version) {

	select_font(&FreeSans9pt7b);
	draw_text(NaMiMenu_OLED_MAIN_BUFFER, name, 78, 32 - 6, 5);
	draw_text(NaMiMenu_OLED_MAIN_BUFFER, version, 78, 32 + 16, 5);

	NAL_NaMiMenu_draw_bitmap_4bpp(NaMi_logo_4bpp_64x64, 3, 1, 64, 64);

	NAL_NaMiMenu_draw_rect(0, 0, OLED_WIDTH - 1, OLED_HEIGHT - 1, 10);

	NAL_NaMiMenu_send_buffer_to_OLED();

	HAL_Delay(2000);
}

void Transision(uint8_t nextState) {
	for(int i = 0; i < NaMiMenu_size; i++) {
		if(NaMiMenu_menuStates[i].state == nextState) {
			NaMiMenu_cursorPos = 0;
			NaMiMenu_menuOffset = 0;
			NaMiMenu_currentMenuState = &NaMiMenu_menuStates[i];
			NAL_NaMiMenu_OLED_Update();
			break;
		}
	}
}

uint8_t NAL_NaMiMenu_IsCursorFrozen() {
	return NaMiMenu_isCursorFrozen;
}

uint8_t NAL_NaMiMenu_CursorDir() {
	return NaMiMenu_cursorDir;
}

uint8_t NAL_NaMiMenu_CursorPos() {
	return NaMiMenu_cursorPos;
}

void NAL_NaMiMenu_Cursor_freeze() {
	NaMiMenu_isCursorFrozen = 1;
	NaMiMenu_menuBrightness = 3;
}

void NAL_NaMiMenu_Cursor_unfreeze() {
	NaMiMenu_isCursorFrozen = 0;
	NaMiMenu_menuBrightness = 10;
}

void NAL_NaMiMenu_Cursor_toggle_freeze() {
	if(NaMiMenu_isCursorFrozen)
		NAL_NaMiMenu_Cursor_unfreeze();
	else
		NAL_NaMiMenu_Cursor_freeze();
}


State* NAL_NaMiMenu_CurrentState() {
	return NaMiMenu_currentMenuState;
}

// ====================== fill buffer ====================== //
/**
 * @brief Fill buffer with specified brightness
 *
 * Layered implementation of SSD1322_OLED_lib function fill_buffer.
 * Uses main static buffer.
 *
 * @param[in] brightness
 */
void NAL_NaMiMenu_fill_buffer(uint8_t brightness) {
	fill_buffer(NaMiMenu_OLED_MAIN_BUFFER, brightness);
}

// ====================== clear buffer ====================== //
/**
 * @brief Clear buffer / Fill buffer with brightness of 0
 *
 * Uses main static buffer.
 *
 * Layered implementation of SSD1322_OLED_lib function fill_buffer.
 * Uses main static buffer.
 */
void NAL_NaMiMenu_clear_buffer() {
	fill_buffer(NaMiMenu_OLED_MAIN_BUFFER, 0);
}

//====================== send frame buffer to OLED ========================//
/**
 *  @brief Sends frame buffer to OLED display.
 *
 *  Uses main static buffer.
 *
 *  Layered implementation of SSD1322_OLED_lib function fill_buffer.
 *  Uses main static buffer.
 */
void NAL_NaMiMenu_send_buffer_to_OLED() {
	send_buffer_to_OLED(NaMiMenu_OLED_MAIN_BUFFER, 0, 0);
}

//====================== draw pixel ========================//
/**
 *  @brief Draws one pixel on frame buffer
 *
 *  Layered implementation of SSD1322_OLED_lib function draw_pixel.
 *  Uses main static buffer.
 *
 *  @param[in] x
 *             horizontal coordinate of pixel
 *  @param[in] y
 *             vertical coordinate of pixel
 *  @param[in] brightness
 *             brightness value of pixel (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_draw_pixel(uint16_t x, uint16_t y, uint8_t brightness) {
	draw_pixel(NaMiMenu_OLED_MAIN_BUFFER, x, y, brightness);
}

//====================== draw sloping line ========================//
/**
 *  @brief Draws sloping line
 *
 *  Layered implementation of SSD1322_OLED_lib function draw_line.
 *  Uses main static buffer.
 *
 *  @param[in] x0
 *             x position of line beginning
 *  @param[in] y0
 *             y position of line beginning
 *  @param[in] x1
 *             x position of line ending
 *  @param[in] y1
 *             y position of line ending
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
*/
void NAL_NaMiMenu_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t brightness) {
	draw_line(NaMiMenu_OLED_MAIN_BUFFER, x0, y0, x1, y1, brightness);
}

//====================== draw empty rectangle ========================//
/**
 *  @brief Draws empty rectangle on frame buffer
 *
 *  Layered implementation of SSD1322_OLED_lib function draw_rect.
 *  Uses main static buffer.
 *
 *  @param[in] x0
 *             x position of first corner
 *  @param[in] y0
 *             y position of first corner
 *  @param[in] x1
 *             x position of second corner
 *  @param[in] y1
 *             y position of second corner
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t x2, uint8_t brightness) {
	draw_rect(NaMiMenu_OLED_MAIN_BUFFER, x0, y0, x1, x2, brightness);
}


void NAL_NaMiMenu_draw_rect_filled(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t x2, uint8_t brightness) {
	draw_rect_filled(NaMiMenu_OLED_MAIN_BUFFER, x0, y0, x1, x2, brightness);
}
//====================== draw empty circle ========================//
/**
 *  @brief Draws empty circle on frame buffer
 *
 *  Layered implementation of SSD1322_OLED_lib function draw_circle.
 *  Uses main static buffer.
 *
 *  @param[in] x0
 *             x position of circle's center
 *  @param[in] y0
 *             y position of circle's center
 *  @param[in] r
 *             radius of the circle (pixels)
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t brightness) {
	draw_circle(NaMiMenu_OLED_MAIN_BUFFER, x0, y0, r, brightness);
}

//====================== draw filled circle ========================//
/**
 *  @brief Draws filled circle on frame buffer
 *
 *  Uses main static buffer.
 *
 *  @param[in] x0
 *             x position of circle's center
 *  @param[in] y0
 *             y position of circle's center
 *  @param[in] r
 *             radius of the circle (pixels)
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_circle_filled(uint16_t x0, uint16_t y0, uint16_t r, uint8_t brightness) {
	for(int x = -r; x < r; x++)
		for(int y = -r; y < r; y++) {
			if((x*x + y*y) < r*r)
				draw_pixel(NaMiMenu_OLED_MAIN_BUFFER, x0 + x, y0 + y, brightness);
		}
}

//====================== draw string ========================//
/**
 *  @brief Draws string in a buffer using selected font
 *
 *	To draw string font has to be selected.
 *
 *	WARNING: This works only for NULL-terminated strings!
 *
 *  Layered implementation of SSD1322_OLED_lib function draw_rect.
 *  Uses main static buffer.
 *
 *  @param[in] text
 *             string with ASCII values
 *  @param[in] x
 *             x position of bottom left corner of first character
 *  @param[in] y
 *             y position of bottom left corner of first character
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_text(const char* text, uint16_t x, uint16_t y, uint8_t brightness) {
	draw_text(NaMiMenu_OLED_MAIN_BUFFER, text, x, y, brightness);
}

//====================== draw string in a grid ========================//
/**
 *  @brief Draws string in a buffer using preselected font in a grid
 *
 *  WARNING: This function changes selected font
 *  WARNING: Preselected font should not be changed in library files
 *
 *  Uses main static buffer.
 *
 *  @param[in] text
 *             string with ASCII values
 *  @param[in] x
 *             x position of bottom left corner of first character
 *  @param[in] y
 *             y position of bottom left corner of first character
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_draw_text_in_grid(const char* text, uint16_t x, uint16_t y, uint8_t brightness) {
	select_font(&FreeSans9pt7b);
	NAL_NaMiMenu_draw_text(text, (11 + 4) * x + 10, (11 + 4) * (y + 1) - 1, brightness);
}

//====================== draw up pointing arrow ========================//
/**
 *  @brief Draws up arrow
 *
 *  x and y coordinates are a tip position of an arrowhead.
 *  Uses main static buffer.
 *
 *  @param[in] x
 *             x position of the tip
 *  @param[in] y0
 *             y position of the tip
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_up_arrow(uint16_t x, uint16_t y, uint8_t brightness) {
	NAL_NaMiMenu_draw_line(x - 2, y + 5, x + 2, y + 5, brightness);
	NAL_NaMiMenu_draw_line(x + 2, y + 5, x, y, brightness);
	NAL_NaMiMenu_draw_line(x - 2, y + 5, x, y, brightness);
}

//====================== draw down pointing arrow ========================//
/**
 *  @brief Draws down arrow
 *
 *  x and y coordinates are a tip position of an arrowhead.
 *  Uses main static buffer.
 *
 *  @param[in] x
 *             x position of the tip
 *  @param[in] y0
 *             y position of the tip
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_down_arrow(uint16_t x, uint16_t y, uint8_t brightness) {
	NAL_NaMiMenu_draw_line(x - 2, y - 5, x + 2, y - 5, brightness);
	NAL_NaMiMenu_draw_line(x + 2, y - 5, x, y, brightness);
	NAL_NaMiMenu_draw_line(x - 2, y - 5, x, y, brightness);
}

//====================== draw left pointing arrow ========================//
/**
 *  @brief Draws left arrow
 *
 *  x and y coordinates are a tip position of an arrowhead.
 *  Uses main static buffer.
 *
 *  @param[in] x
 *             x position of the tip
 *  @param[in] y0
 *             y position of the tip
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_left_arrow(uint16_t x, uint16_t y, uint8_t brightness) {
	NAL_NaMiMenu_draw_line(x + 5, y + 2, x + 5, y - 2, brightness);
	NAL_NaMiMenu_draw_line(x + 5, y + 2, x, y, brightness);
	NAL_NaMiMenu_draw_line(x + 5, y - 2, x, y, brightness);
}

//====================== draw right pointing arrow ========================//
/**
 *  @brief Draws right arrow
 *
 *  x and y coordinates are a tip position of an arrowhead.
 *  Uses main static buffer.
 *
 *  @param[in] x
 *             x position of the tip
 *  @param[in] y0
 *             y position of the tip
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_NaMiMenu_draw_right_arrow(uint16_t x, uint16_t y, uint8_t brightness) {
	NAL_NaMiMenu_draw_line(x - 5, y + 2, x - 5, y - 2, brightness);
	NAL_NaMiMenu_draw_line(x - 5, y + 2, x, y, brightness);
	NAL_NaMiMenu_draw_line(x - 5, y - 2, x, y, brightness);
}


void NAL_NaMiMenu_draw_bitmap_4bpp(const uint8_t *bitmap, uint16_t x0, uint16_t y0, uint16_t x_size, uint16_t y_size) {
	draw_bitmap_4bpp(NaMiMenu_OLED_MAIN_BUFFER, bitmap, x0, y0, x_size, y_size);
}

//====================== draw indicator ========================//
/**
 *  @brief Draws an indicator
 *
 *	x and y coordinates reffer to a grid square
 *  Uses main static buffer.
 *
 *  @param[in] status
 *  		   binary status
 *  @param[in] x0
 *             x position
 *  @param[in] y0
 *             y position
 * 	@param[in] brightness
 *             brightness value of pixels (range 0-15 dec or 0x00-0x0F hex)
 */
void NAL_draw_indicator_in_grid(uint8_t status, uint16_t x, uint16_t y, uint8_t brightness) {
	uint8_t x_pos = (11 + 4) * x + 10;
	uint8_t y_pos = (11 + 4) * (y + 1) - 10 - 1;
	NAL_NaMiMenu_draw_rect(x_pos, y_pos, x_pos + 10, y_pos + 10, brightness);

	if(status)
		NAL_NaMiMenu_draw_circle_filled(x_pos + 5, y_pos + 5, 4, brightness);
	else
		NAL_NaMiMenu_draw_circle(x_pos + 5, y_pos + 5, 4, brightness);
}

//====================== draw progress bar ========================//
/**
 *  @brief Draws a progress bar on the right side of the screen
 *
 *  Uses main static buffer.
 *
 *  @param[in] progressBar
 *             struct object with progress bar information
 */
void NAL_draw_progress_bar(ProgressBar *progressBar) {
	NAL_NaMiMenu_draw_line(247, 4, 247, 59, 5);
	NAL_NaMiMenu_draw_line(254, 4, 254, 59, 5);

	NAL_draw_pixel(248, 3, 5);
	NAL_draw_pixel(249, 2, 5);
	NAL_draw_pixel(250, 2, 5);
	NAL_draw_pixel(251, 2, 5);
	NAL_draw_pixel(252, 2, 5);
	NAL_draw_pixel(253, 3, 5);

	NAL_draw_pixel(248, 60, 5);
	NAL_draw_pixel(249, 61, 5);
	NAL_draw_pixel(250, 61, 5);
	NAL_draw_pixel(251, 61, 5);
	NAL_draw_pixel(252, 61, 5);
	NAL_draw_pixel(253, 60, 5);

	double progress = ((double)progressBar->value) / 255;

	if(progress) {
		NAL_NaMiMenu_draw_rect_filled(249, 59 - progress * 55, 252, 59, 1);
		NAL_draw_pixel(249, 59, 0);
		NAL_draw_pixel(252, 59, 0);
		NAL_draw_pixel(249, 4, 0);
		NAL_draw_pixel(252, 4, 0);
	}

	if(progressBar->sections) {
		uint8_t section_offset = 55 / progressBar->sections;

		for(int i = 0; i < (progressBar->sections - 1); i++) {
			uint8_t section_y = 59 - (i + 1) * section_offset;
			NAL_NaMiMenu_draw_line(249, section_y, 252, section_y, 5);
		}
	}

}


void NAL_draw_chart_in_grid(Chart *chart, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t brightness) {
	uint8_t x_pos_start = (11 + 4) * x + 10;
	uint8_t y_pos_start = (11 + 4) * (y + 1) - 10 - 1;
	uint8_t x_pos_end = (11 + 4) * (width + x - 1) + 10 + 10;
	uint8_t y_pos_end = (11 + 4) * (height + y) - 1;

	uint8_t chart_height = y_pos_end - y_pos_start;
	uint8_t chart_width = x_pos_end - x_pos_start;



	if(chart->maxValue < chart->minValue) {
		NAL_NaMiMenu_draw_rect(x_pos_start, y_pos_start, x_pos_end, y_pos_end, brightness);
		NAL_NaMiMenu_draw_line(x_pos_start, y_pos_start, x_pos_end, y_pos_end, brightness);
		NAL_NaMiMenu_draw_line(x_pos_start, y_pos_end, x_pos_end, y_pos_start, brightness);
		return;
	}

	double temp_var = 0;
	if(chart->maxValue <= 0)
		 temp_var = abs((int)chart->minValue);
	else
		temp_var = abs((int)chart->maxValue) + abs((int)chart->minValue);

	uint8_t y_offset = 0;

	if(chart->minValue < 0)
		 y_offset = (chart_height / temp_var * abs((int)chart->minValue));

	NAL_NaMiMenu_draw_up_arrow(x_pos_start, y_pos_start - 2, brightness);
	NAL_NaMiMenu_draw_line(x_pos_start, y_pos_start + 3, x_pos_start, y_pos_end, brightness);

	NAL_NaMiMenu_draw_right_arrow(x_pos_end + 2, y_pos_end - y_offset, brightness);
	NAL_NaMiMenu_draw_line(x_pos_start, y_pos_end - y_offset, x_pos_end - 3, y_pos_end - y_offset, brightness);

	// Plot data

	for(int i = 0; (i < chart->dataSize - 1) && (i < chart_width); i++) {
		if((chart->data[i] > chart->maxValue) || (chart->data[i] < chart->minValue))
			continue;
		if((chart->data[i + 1] > chart->maxValue) || (chart->data[i + 1] < chart->minValue))
			continue;

		double value_y_pos1 = chart_height / temp_var * chart->data[i];
		double value_y_pos2 = chart_height / temp_var * chart->data[i + 1];
		NAL_NaMiMenu_draw_line(x_pos_start + i, y_pos_end - y_offset - value_y_pos1, x_pos_start + i + 1, y_pos_end - y_offset - value_y_pos2, 3);
	}

}

__weak void NAL_NaMiMenu_OLEDUpdate_Callback(State *state, Clickable *clickable) {

	// No implementation

}

__weak void NAL_CursorMove_Callback(State *state, Clickable *clickable) {

	// No implementation

}

__weak void NAL_FrozenCursorMove_Callback(State *state, Clickable *clickable) {

	// No implementation

}

