/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>

#include "NaMiMenu_lib/NaMiMenu.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// OLED VARIABLES

// TIP ETCHER VARIBLES

uint16_t var_etchTime = 60;   // s
uint16_t var_depth    = 5000; // um
uint16_t var_step     = 20;   // um
uint16_t var_polishTime = 0; // ms

uint16_t etchTime_cnt = 0;
uint16_t stepTime_cnt = 0;

uint16_t polishTime_cnt = 0;
int etchStartPosition = 0;

//

uint16_t var_current = 100;
uint8_t progress_bar = 0;

double test_chart_data[100] = {
	//0, 1, 2, 4, 8, 16, 20, 28, 24, -10, -5, -4, -3, -2, -1, -14, -15, -20, -20, -20, 15, 16, 17
};

uint8_t dataCounter = 0;
uint8_t iterator = 0;

Chart test_chart = {
		test_chart_data,
		sizeof(test_chart_data) / sizeof(double),
		50,
		0
};

ProgressBar bar1 = {255 / 2, 0};

typedef struct Motor {
	int pos;
	int setPos;
    GPIO_TypeDef* dir_port;
    uint16_t dir_pin;
    GPIO_TypeDef* step_port;
    uint16_t step_pin;
} Motor;

// 23.2u / krok
Motor Motor1 = {0, 0, DIR_1_GPIO_Port, DIR_1_Pin, STEP_1_GPIO_Port, STEP_1_Pin};
Motor Motor2 = {0, 0, DIR_2_GPIO_Port, DIR_2_Pin, STEP_2_GPIO_Port, STEP_2_Pin};

//uint8_t current_value = 0;

void StateTransision(uint8_t nextState);

void voltage_select(State *state, Clickable *clickable);
void etching_enable(State *state, Clickable *clickable);

void led_toggle(State *state, Clickable *clickable);
void etching_start_stop(State *state, Clickable *clickable);
void settings_toggle_cursor(State *state, Clickable *clickable);
void settings_voltage_select(State *state, Clickable *clickable);
void Update_Menu();
void Update_Settings();
void Update_Manual();
void Motor2_Home();

void Update_Debug();


void INA219_Init();

void INA219_ReadCurrent();

// =======================================================

void Tip_Enable() {
	HAL_GPIO_WritePin(TIP_EN_GPIO_Port, TIP_EN_Pin, 1);
}
void Tip_Disable() {
	HAL_GPIO_WritePin(TIP_EN_GPIO_Port, TIP_EN_Pin, 0);
}
void Tip_5V() {
	HAL_GPIO_WritePin(V_SELECT_GPIO_Port, V_SELECT_Pin, 0);
}
void Tip_24V() {
	HAL_GPIO_WritePin(V_SELECT_GPIO_Port, V_SELECT_Pin, 1);
}

// ETCHING ===============================================

uint8_t etching = 0;
uint16_t etchingTime = 10;
uint16_t currentValue = 0;


double currentValue2 = 0;
double currentMiliAmp = 0;
double minCurrentMiliAmp = 0.4;

double Tip_Current() {
	return currentMiliAmp;
}

uint8_t frame[2] = {0, 0};

enum Etching_States {
	None,
	IDLE,
	HEAD_DOWN,
	TIP_DOWN,
	TIP_EXTEND,
	HOMING,
	APPROACH,
	DIVE,
	ETCH,
	POLISH
};

enum Etching_States currentEtchingState = None;
enum Etching_States nextEtchingState    = None;

void StateTransision(uint8_t nextState) {
	currentEtchingState = nextState;
}

int SelectState(enum Etching_States state) {
	if(nextEtchingState == state && nextEtchingState != currentEtchingState) {
		currentEtchingState = nextEtchingState;
		NAL_NaMiMenu_OLED_Update();
		return 1;
	} return 0;
}

// MENU ===============================================

enum Menu_States {
	NONE,
	Menu,
	Settings,
	Manual,
	Debug
};

Clickable menu_clickable[] = {
	{NONE, NULL, ""},
	{NONE, &etching_start_stop, "Start"},
	{Settings, NULL, "Settings"},
	{Manual, NULL, "Manual"},
	{Debug, NULL, "Debug"},
	{NONE, &INA219_ReadCurrent, "R Current"}
};

Clickable debug_clickable[] = {
	{None, NULL, "Stop"},
	{NONE, NULL, "LowerHead"},
	{NONE, NULL, "LowerTip"},
	{NONE, NULL, "LiftHead"},
	{NONE, NULL, "LowerHeadAndEtch"},
	{Menu, NULL, "Back"}
};

Clickable manual_clickable[] = {
	{NONE, NULL, ""},
	{NONE, &voltage_select, "Relay"},
	{NONE, &NAL_NaMiMenu_Cursor_toggle_freeze, "Motor1"},
	{NONE, &NAL_NaMiMenu_Cursor_toggle_freeze, "Motor2"},
	{NONE, &etching_enable, "Etch. En."},
	{NONE, &Motor2_Home, "Home Motor2"},
	{Menu, NULL, "Back"}
};

Clickable settings_clickable[] = {
	{NONE, &settings_toggle_cursor, "Etching time"},
	{NONE, &settings_toggle_cursor, "Depth"},
	{NONE, &settings_toggle_cursor, "Step"},
	{NONE, &settings_voltage_select, "voltage"},
	{NONE, &settings_toggle_cursor, "Polish time"},
	{NONE, &settings_toggle_cursor, "Max current"},
	{Menu, NULL, "Back"}
};

State states[] = {
	{Menu, &Update_Menu, menu_clickable, ARRAY_SIZE(menu_clickable)},
	{Settings, &Update_Settings, settings_clickable, ARRAY_SIZE(settings_clickable)},
	{Manual, &Update_Manual, manual_clickable, ARRAY_SIZE(manual_clickable)},
	{Debug, &Update_Debug, debug_clickable, ARRAY_SIZE(debug_clickable)}
};

int voltageSelection = 0;

void settings_voltage_select(State *state, Clickable *clickable) {
	voltageSelection = !voltageSelection;
}

void Update_Debug() { }

void Update_Menu() {

	//NAL_draw_indicator_in_grid(HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin), 13, 0, 5);
	//NAL_draw_text_in_grid("5V", 14, 0, 5);
	//NAL_draw_indicator_in_grid(!HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin), 13, 1, 5);
	//NAL_draw_text_in_grid("24V", 14, 1, 5);

	char text1[24];
	//sprintf(text1, "%d", currentValue);

	int miliAmp = currentValue2 / 1000.0;
	int nanoAmp = (int)((currentValue2 - miliAmp * 1000.0));

	char nanoAmpText[16];

	if(miliAmp < 0)
		miliAmp = 0;

	if(nanoAmp < 0)
		nanoAmp = 0;

	if(nanoAmp >= 100)
		sprintf(nanoAmpText, "%d", nanoAmp);
	if(nanoAmp < 100)
		sprintf(nanoAmpText, "0%d", nanoAmp);
	else if(nanoAmp < 10)
		sprintf(nanoAmpText, "00%d", nanoAmp);
	else if(nanoAmp < 1)
		sprintf(nanoAmpText, "000");

	sprintf(text1, "%d,%s mA", miliAmp, nanoAmpText);
	//sprintf(text1, "%d | %d", frame[0], frame[1]);
	NAL_draw_text_in_grid(text1, 8, 0, 5);
	test_chart_data[dataCounter++ % 100] = miliAmp;

	if(!etching)
		NAL_draw_text_in_grid("Idle", 5, 0, 5);
	else {
		sprintf(text1, "T: %d", var_etchTime - etchTime_cnt * 2 / 1000);
		NAL_draw_text_in_grid(text1, 5, 0, 5);
	}

	NAL_draw_chart_in_grid(&test_chart, 5, 1, 8, 3, 5);

	//NAL_draw_progress_bar(&bar1);
}

void Update_Settings() {
	char text1[16];

	// Etching time
	sprintf(text1, "%d s", var_etchTime);
	NAL_draw_text_in_grid(text1, 8, 0, 5);
	// Depth
	sprintf(text1, "%d um", var_depth);
	NAL_draw_text_in_grid(text1, 8, 1, 5);
	// Step
	sprintf(text1, "%d um", var_step);
	NAL_draw_text_in_grid(text1, 8, 2, 5);
	// Voltage Selection
	if(voltageSelection) {
		NAL_draw_text_in_grid("5 V", 8, 3, 5);
	} else {
		NAL_draw_text_in_grid("24 V", 8, 3, 5);
	}
	// Max current
	//sprintf(text1, "%d mA", var_current);
	//NAL_draw_text_in_grid(text1, 8, 3, 5);
	// Polish time
	sprintf(text1, "%d ms", var_polishTime);
	NAL_draw_text_in_grid(text1, 12, 0, 5);
}

void Update_Manual() {
	NAL_draw_indicator_in_grid(!HAL_GPIO_ReadPin(V_SELECT_GPIO_Port, V_SELECT_Pin), 8, 0, 5);
	NAL_draw_text_in_grid("5V", 9, 0, 5);
	NAL_draw_indicator_in_grid(HAL_GPIO_ReadPin(V_SELECT_GPIO_Port, V_SELECT_Pin), 11, 0, 5);
	NAL_draw_text_in_grid("24V", 12, 0, 5);
	//
	NAL_draw_indicator_in_grid(!HAL_GPIO_ReadPin(LIMIT_SWITCH_1_GPIO_Port, LIMIT_SWITCH_1_Pin), 7, 2, 5);
	char text1[16];
	sprintf(text1, "%d", Motor1.setPos);
	NAL_draw_text_in_grid(text1, 8, 1, 5);
//	sprintf(text1, "%d", Motor1.pos);
//	NAL_draw_text_in_grid(text1, 12, 1, 5);
	//
	sprintf(text1, "%d", Motor2.setPos);
	NAL_draw_text_in_grid(text1, 8, 2, 5);
//	sprintf(text1, "%d", Motor2.pos);
//	NAL_draw_text_in_grid(text1, 12, 2, 5);
	//
	NAL_draw_indicator_in_grid(HAL_GPIO_ReadPin(TIP_EN_GPIO_Port, TIP_EN_Pin), 8, 3, 5);
	NAL_draw_text_in_grid("Electrode", 9, 3, 5);
}

void Motor2_Home() {
	Motor2.setPos = 10000;
}

void voltage_select(State *state, Clickable *clickable) {
	HAL_GPIO_TogglePin(V_SELECT_GPIO_Port, V_SELECT_Pin);
}

void etching_enable(State *state, Clickable *clickable) {
	HAL_GPIO_TogglePin(TIP_EN_GPIO_Port, TIP_EN_Pin);
}

void led_toggle(State *state, Clickable *clickable) {
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

uint8_t motor1Pos;

void etching_start_stop(State *state, Clickable *clickable) {
	etching = !etching;

	if(etching) {
		//clickable->text = "Stop";
		//NAL_NaMiMenu_Cursor_freeze();
		//currentEtchingState = HEAD_DOWN;
		motor1Pos = Motor1.setPos;
	}
	else {
		//clickable->text = "Start";
		//NAL_NaMiMenu_Cursor_unfreeze();
	}
}

void settings_toggle_cursor(State *state, Clickable *clickable) {
	NAL_NaMiMenu_Cursor_toggle_freeze();
}

void NAL_FrozenCursorMove_Callback(State *state, Clickable *clickable) {

	if(state->state == Settings) {

		switch(NAL_NaMiMenu_CursorPos()) {
		case 0: // Etching time
			var_etchTime = NAL_NaMiMenu_CursorDir() ? var_etchTime + 1 : var_etchTime - 1;
			break;
		case 1: // Depth
			var_depth = NAL_NaMiMenu_CursorDir() ? var_depth + 100 : var_depth - 100;
			break;
		case 2: // Step
			var_step = NAL_NaMiMenu_CursorDir() ? var_step + 5 : var_step - 5;
			break;
		case 5: // Max current
			//if(NAL_NaMiMenu_CursorDir()) { var_current++; } else { var_current--; }
			var_current = NAL_NaMiMenu_CursorDir() ? var_current + 1 : var_current - 1;
			break;
		case 4: // Polish time
			var_polishTime = NAL_NaMiMenu_CursorDir() ? var_polishTime + 10 : var_polishTime - 10;
			break;
		}
	}

	if(state->state == Manual) {
		if(NAL_NaMiMenu_CursorPos() == 2) {
			if(NAL_NaMiMenu_CursorDir()) {
				Motor1.setPos += 100;
			} else {
				Motor1.setPos -= 100;
			}
		}

		if(NAL_NaMiMenu_CursorPos() == 3) {
			if(NAL_NaMiMenu_CursorDir()) {
				Motor2.setPos += 10;
			} else {
				Motor2.setPos -= 10;
			}
		}
	}

	if(state->state == Menu && NAL_NaMiMenu_CursorPos() == 1) {
		if(NAL_NaMiMenu_CursorDir()) {
			if(bar1.value < 255)
				bar1.value += 15;
		} else {
			if(bar1.value > 0)
				bar1.value -= 15;
		}
	}
}

// Callback do wywołania przed wysłaniem buffera do OLED'a

void MoveMotor(Motor *motor) {

	if(motor->pos != motor->setPos) {

		if(motor->pos < motor->setPos) {
			HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, 0);
			motor->pos++;

			if(!HAL_GPIO_ReadPin(LIMIT_SWITCH_1_GPIO_Port, LIMIT_SWITCH_1_Pin) && motor == &Motor2) {
				Motor2.pos = 0;
				Motor2.setPos = 0;
			}
		} else {
			if(motor == &Motor2 && motor->pos <= -2400) {
				return;
			}
			HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, 1);
			motor->pos--;
		}

		HAL_GPIO_WritePin(motor->step_port, motor->step_pin, 1);
		HAL_GPIO_WritePin(motor->step_port, motor->step_pin, 0);
	}

}

uint16_t tim_cnt = 0;
uint16_t debounce = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim == &htim2) { // Updates every 2 ms

		debounce++;

		if(currentEtchingState == ETCH)
			etchingTime++;

		tim_cnt++;
		if((tim_cnt >= 250) && HAL_GPIO_ReadPin(TIP_EN_GPIO_Port, TIP_EN_Pin)) {
			//NAL_NaMiMenu_OLED_Update();
			if(currentEtchingState == ETCH || currentEtchingState == IDLE) {
				INA219_ReadCurrent();
				NAL_NaMiMenu_OLED_Update();
			}
			tim_cnt = 0;
		}

		if(etching && currentEtchingState == ETCH) {
			etchTime_cnt++;
			stepTime_cnt++;
		}

		if(currentEtchingState == POLISH && Motor2.pos == etchStartPosition) {
			polishTime_cnt++;
		}

		MoveMotor(&Motor1);

		MoveMotor(&Motor2);
	}
}

int Motor2_MaxPos() {
	return Motor2.pos <= -2400;
}

int M1startingPos = 0;
int M1maxPos = 12; // mm

int Motor1_MaxPos() {
	return Motor1.pos >= M1startingPos + ((double)(M1maxPos) / 0.0232);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == NaMiMenu_ENC_BTN.pin && debounce > 10) {
		NAL_NaMiMenu_Encoder_Update(GPIO_Pin);
		debounce = 0;
	} else if(GPIO_Pin != NaMiMenu_ENC_BTN.pin)
		NAL_NaMiMenu_Encoder_Update(GPIO_Pin);
}

void INA219_Init() {
    uint8_t calibration[2];
    uint16_t cal;

    double R_SHUNT = 0.16; // 0.33;  // Ω
    double maxAmp = 0.3;    //

    double current_lsb = maxAmp / 32767.0;
    cal = (uint16_t)(0.04096 / (current_lsb * R_SHUNT));

    calibration[0] = (uint8_t)(cal >> 8);
    calibration[1] = (uint8_t) cal;

    // Zapis kalibracji
    HAL_I2C_Mem_Write(&hi2c1, (0x40 << 1), 0x05, I2C_MEMADD_SIZE_8BIT, calibration, 2, HAL_MAX_DELAY);

    // Ustawienie trybu: 32V / 320mV / 12bit / ciągły
    uint8_t config[2] = {0x01, 0x9F};
    HAL_I2C_Mem_Write(&hi2c1, (0x40 << 1), 0x00, I2C_MEMADD_SIZE_8BIT, config, 2, HAL_MAX_DELAY);
}

void INA219_ReadCurrent() {
    uint8_t data[2];

    HAL_I2C_Mem_Read(&hi2c1, (0x40 << 1), 0x04, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);

    int16_t rawCurrent = (int16_t)((data[0] << 8) | data[1]);

    frame[0] = data[0];
    frame[1] = data[1];

    //double R_SHUNT = 100.0;
    double maxAmp = 0.3;
    double current_lsb = maxAmp / 32767.0;

    double current_A = rawCurrent * current_lsb;

    currentValue2 = current_A * 1000000.0;
    currentMiliAmp = currentValue2 / 1000.0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);

  HAL_TIM_Base_Start_IT(&htim2);

  NAL_NaMiMenu_init(OLED_CS_GPIO_Port, OLED_CS_Pin, OLED_DC_GPIO_Port, OLED_DC_Pin, OLED_RESET_GPIO_Port, OLED_RESET_Pin, &hspi2);
  NAL_NaMiMenu_setMenuStates(states, sizeof(states) / sizeof(State));
  NAL_NaMiMenu_Encoder_setup(ENCODER_BTN_GPIO_Port, ENCODER_BTN_Pin, ENCODER_SIGA_GPIO_Port, ENCODER_SIGA_Pin, ENCODER_SIGB_GPIO_Port, ENCODER_SIGB_Pin);

  NAL_NaMiMenu_splash_screen("Tip Etcher", "v2.2 | in dev");

  // Device initialization

  INA219_Init();

  //

  Transision(Menu);

  HAL_GPIO_WritePin(M_ENABLE_GPIO_Port, M_ENABLE_Pin, 0);


  NAL_NaMiMenu_SetCursorPos(1);

  //if(!HAL_GPIO_ReadPin(LIMIT_SWITCH_1_GPIO_Port, LIMIT_SWITCH_1_Pin))
	  //;//NAL_NaMiMenu_Cursor_freeze();

  //Motor2_Home();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(etching)
		  INA219_ReadCurrent();

	  switch(nextEtchingState) {

	  case IDLE:
		  if(SelectState(IDLE)) {
			  Tip_Disable();
			  Tip_5V();
			  Motor2_Home();
			  etchTime_cnt = 0;
			  etching = 0;
			  Motor1.setPos = M1startingPos;
		  }
		  // Klinkięto "Start"
		  if(etching) { nextEtchingState = HEAD_DOWN; M1startingPos = Motor1.pos; }
		  break;

	  case HEAD_DOWN:
		  if(SelectState(HEAD_DOWN)) {
			  Tip_5V();
			  Tip_Enable();
		  }
		  // Wykryto taflę cieczy (przepływa prąd).
		  if(Tip_Current() > minCurrentMiliAmp) { nextEtchingState = TIP_EXTEND; }
		  // Osiągnięto pozycję maksymalną M2.
		  if(Motor2_MaxPos()) { nextEtchingState = TIP_DOWN; }

		  if(Motor2.pos == Motor2.setPos) {
			  Motor2.setPos -= 1;
		  }
		  break;

	  case TIP_DOWN:
		  if(SelectState(TIP_DOWN)) {
			  Tip_5V();
			  Tip_Enable();
		  }
		  if(Motor1.pos == Motor1.setPos) {
			  Motor1.setPos += 1;
		  }
		  // Wykryto taflę cieczy (przepływa prąd).
		  if(Tip_Current() > minCurrentMiliAmp) { nextEtchingState = TIP_EXTEND; }
		  // Osiągnięto pozycję maksymalną M1.
		  if(Motor1_MaxPos()) { nextEtchingState = IDLE; }
		  break;

	  case TIP_EXTEND:
		  if(SelectState(TIP_EXTEND)) {
			  Tip_5V();
			  Tip_Disable();
			  Motor1.setPos += ((double)(var_depth + 500) / 23.2);
		  }
		  // Osiągnięto pozycję maksymalną M1.
		  if(Motor1_MaxPos()) { nextEtchingState = IDLE; }
		  if(Motor1.pos >= M1startingPos + ((double)(var_depth) / 23.2)) { nextEtchingState = HOMING; }
		  break;

	  case HOMING:
		  if(SelectState(HOMING)) {
			  Tip_Disable();
			  Motor2_Home();
		  }
		  if(Motor2.pos == 0) { nextEtchingState = APPROACH; }
		  break;

	  case APPROACH:
		  if(SelectState(APPROACH)) {
			  Tip_5V();
			  Tip_Enable();
		  }
		  // Wykryto taflę cieczy (przepływa prąd).
		  if(Tip_Current() > minCurrentMiliAmp) { nextEtchingState = DIVE; }

		  if(Motor2.pos == Motor2.setPos && !Motor2_MaxPos()) {
			  Motor2.setPos -= 1;
		  }
		  break;

	  case DIVE:
		  if(SelectState(DIVE)) {
			  Tip_5V();
			  Tip_Disable();
			  Motor2.setPos = Motor2.pos - (double)(var_depth) / 5.0;
		  }
		  if(Motor2.pos == Motor2.setPos) { nextEtchingState = ETCH; }
		  break;

	  case ETCH:
		  if(SelectState(ETCH)) {
			  if(voltageSelection)
				  Tip_5V();
			  else
				  Tip_24V();
			  Tip_Enable();
			  etchStartPosition = Motor2.pos;
		  }
		  if(stepTime_cnt * 2 >= ((double)(var_step) / (double)(var_depth * 1.0)) * (var_etchTime * 1000.0)) {
			  stepTime_cnt = 0;
			  Motor2.setPos += (double)(var_step) / 5;
		  }

		  if(etchTime_cnt * 2 >= var_etchTime * 1000.0) {
			  if(var_polishTime > 0)
				  nextEtchingState = POLISH;
			  else
				  nextEtchingState = IDLE;
		  }
		  break;
	  case POLISH:
		  if(SelectState(POLISH)) {
			  Motor2.setPos = etchStartPosition;
			  Tip_Disable();
		  }

		  if(Motor2.pos == etchStartPosition) {
			  if(polishTime_cnt * 2 >= var_polishTime) {
				  nextEtchingState = IDLE;
			  } else {
				  Tip_24V();
				  Tip_Enable();
			  }
		  }

		  break;
	  default:
	  case None:
		  NAL_NaMiMenu_OLED_Update();
		  nextEtchingState = IDLE;
		  break;
	  }

	  if(!etching) {
		  nextEtchingState = IDLE;
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00503D58;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 31999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OLED_RESET_Pin|OLED_DC_Pin|OLED_CS_Pin|M_ENABLE_Pin
                          |TIP_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Pin|V_SELECT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIR_1_Pin|STEP_1_Pin|DIR_2_Pin|STEP_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BOARD_BTN_Pin */
  GPIO_InitStruct.Pin = BOARD_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOARD_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OLED_RESET_Pin OLED_DC_Pin OLED_CS_Pin M_ENABLE_Pin
                           TIP_EN_Pin */
  GPIO_InitStruct.Pin = OLED_RESET_Pin|OLED_DC_Pin|OLED_CS_Pin|M_ENABLE_Pin
                          |TIP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ENCODER_BTN_Pin */
  GPIO_InitStruct.Pin = ENCODER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENCODER_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LIMIT_SWITCH_1_Pin */
  GPIO_InitStruct.Pin = LIMIT_SWITCH_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LIMIT_SWITCH_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Pin V_SELECT_Pin */
  GPIO_InitStruct.Pin = LED_Pin|V_SELECT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ENCODER_SIGA_Pin */
  GPIO_InitStruct.Pin = ENCODER_SIGA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENCODER_SIGA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENCODER_SIGB_Pin */
  GPIO_InitStruct.Pin = ENCODER_SIGB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENCODER_SIGB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIR_1_Pin STEP_1_Pin DIR_2_Pin STEP_2_Pin */
  GPIO_InitStruct.Pin = DIR_1_Pin|STEP_1_Pin|DIR_2_Pin|STEP_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
