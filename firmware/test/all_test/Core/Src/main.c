/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <string.h>
#include <stdio.h>
#include <stdint.h>
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
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

	#define AS5600_ADDR        (0x36 << 1)
	#define AS5600_RAW_ANGLE_H 0x0C

	#define STEP_PULSE_US      5

	/*
	   LIMIT SWITCH CONNECTION:
	   Switch type: NO + C
	   CubeMX input: GPIO Input + Pull-Up

	   Normal state: HIGH
	   Triggered:    LOW

	   So active state is GPIO_PIN_RESET.
	*/
	#define LIMIT_ACTIVE_STATE GPIO_PIN_RESET
	#define USE_LIMITS 1

	/*
	   All drivers are configured as 1/64 microstepping.
	*/
	#define FULL_STEPS_PER_REV 200
	#define X_MICROSTEPS 64
	#define Z_MICROSTEPS 64
	#define C_MICROSTEPS 64

	#define X_STEPS_PER_REV (FULL_STEPS_PER_REV * X_MICROSTEPS)
	#define Z_STEPS_PER_REV (FULL_STEPS_PER_REV * Z_MICROSTEPS)
	#define C_STEPS_PER_REV (FULL_STEPS_PER_REV * C_MICROSTEPS)

	/*
	   C axis has no limit switch.
	   It will rotate 1 revolution forward, then 1 revolution backward.
	*/
	#define C_SOFT_LIMIT_STEPS C_STEPS_PER_REV

	typedef struct
	{
		const char *name;

		GPIO_TypeDef *stepPort;
		uint16_t stepPin;

		GPIO_TypeDef *dirPort;
		uint16_t dirPin;

		GPIO_TypeDef *enPort;
		uint16_t enPin;

		I2C_HandleTypeDef *hi2c;

		int32_t stepsPerRev;
		uint32_t stepIntervalUs;

		int8_t dir;
		int8_t dirInvert;
		int8_t encSign;

		int32_t cmdSteps;

		uint16_t lastRaw;
		int32_t encCounts;
		uint8_t encOk;

		uint32_t lastStepUs;

	} Axis_t;

	static uint32_t cyclesPerUs = 84;
	static uint32_t lastPrintMs = 0;

	/*
	   AXIS ORDER:
	   axis[0] = X
	   axis[1] = Z
	   axis[2] = C
	*/
	static Axis_t axis[3] =
	{
		{
			"X",
			GPIOA, GPIO_PIN_10,   // X_STEP PA10
			GPIOB, GPIO_PIN_3,    // X_DIR  PB3
			GPIOA, GPIO_PIN_0,    // X_EN   PA0
			&hi2c1,               // AS5600 X
			X_STEPS_PER_REV,
			2500,                 // step interval us; 1/64 olduğu için güvenli yavaş hız
			1,
			0,
			1,
			0,
			0,
			0,
			0,
			0
		},
		{
			"Z",
			GPIOB, GPIO_PIN_5,    // Z_STEP PB5
			GPIOB, GPIO_PIN_4,    // Z_DIR  PB4
			GPIOA, GPIO_PIN_1,    // Z_EN   PA1
			&hi2c2,               // AS5600 Z
			Z_STEPS_PER_REV,
			2500,
			1,
			0,
			1,
			0,
			0,
			0,
			0,
			0
		},
		{
			"C",
			GPIOA, GPIO_PIN_9,    // C_STEP PA9
			GPIOC, GPIO_PIN_7,    // C_DIR  PC7
			GPIOA, GPIO_PIN_4,    // C_EN   PA4
			&hi2c3,               // AS5600 C
			C_STEPS_PER_REV,
			2000,
			1,
			0,
			1,
			0,
			0,
			0,
			0,
			0
		}
	};

	static void DWT_Init_Custom(void)
	{
		cyclesPerUs = HAL_RCC_GetHCLKFreq() / 1000000UL;

		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CYCCNT = 0;
		DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	}

	static uint32_t micros_custom(void)
	{
		return DWT->CYCCNT / cyclesPerUs;
	}

	static void delay_us_custom(uint32_t us)
	{
		uint32_t start = DWT->CYCCNT;
		uint32_t cycles = us * cyclesPerUs;

		while ((uint32_t)(DWT->CYCCNT - start) < cycles)
		{
			// wait
		}
	}

	static void UART_Print(const char *s)
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 100);
	}

	static uint8_t AS5600_ReadRaw(I2C_HandleTypeDef *hi2c, uint16_t *raw)
	{
		uint8_t data[2];

		HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
			hi2c,
			AS5600_ADDR,
			AS5600_RAW_ANGLE_H,
			I2C_MEMADD_SIZE_8BIT,
			data,
			2,
			20
		);

		if (status != HAL_OK)
		{
			return 0;
		}

		*raw = ((uint16_t)data[0] << 8) | data[1];
		*raw &= 0x0FFF;

		return 1;
	}

	static int32_t abs_i32(int32_t x)
	{
		return (x < 0) ? -x : x;
	}

	static void format_x100(char *buf, uint32_t len, int32_t value)
	{
		char sign = '+';

		if (value < 0)
		{
			sign = '-';
			value = -value;
		}

		snprintf(buf, len, "%c%ld.%02ld",
				 sign,
				 (long)(value / 100),
				 (long)(value % 100));
	}

	static void Axis_SetDir(Axis_t *a, int8_t dir)
	{
		if (dir >= 0)
		{
			a->dir = 1;
		}
		else
		{
			a->dir = -1;
		}

		GPIO_PinState state;

		if (a->dir > 0)
		{
			state = GPIO_PIN_SET;
		}
		else
		{
			state = GPIO_PIN_RESET;
		}

		if (a->dirInvert)
		{
			state = (state == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;
		}

		HAL_GPIO_WritePin(a->dirPort, a->dirPin, state);
	}

	static void Axis_Enable(Axis_t *a)
	{
		// TMC EN active LOW
		HAL_GPIO_WritePin(a->enPort, a->enPin, GPIO_PIN_RESET);
	}

	static void Axis_Disable(Axis_t *a)
	{
		HAL_GPIO_WritePin(a->enPort, a->enPin, GPIO_PIN_SET);
	}

	static void Axis_Init(Axis_t *a)
	{
		HAL_GPIO_WritePin(a->stepPort, a->stepPin, GPIO_PIN_RESET);
		Axis_SetDir(a, a->dir);
		Axis_Enable(a);

		uint16_t raw;

		if (AS5600_ReadRaw(a->hi2c, &raw))
		{
			a->lastRaw = raw;
			a->encCounts = 0;
			a->encOk = 1;
		}
		else
		{
			a->lastRaw = 0;
			a->encCounts = 0;
			a->encOk = 0;
		}

		a->cmdSteps = 0;
		a->lastStepUs = micros_custom();
	}

	static void Axis_UpdateEncoder(Axis_t *a)
	{
		uint16_t raw;

		if (!AS5600_ReadRaw(a->hi2c, &raw))
		{
			a->encOk = 0;
			return;
		}

		int16_t diff = (int16_t)raw - (int16_t)a->lastRaw;

		if (diff > 2048)
		{
			diff -= 4096;
		}
		else if (diff < -2048)
		{
			diff += 4096;
		}

		a->encCounts += diff;
		a->lastRaw = raw;
		a->encOk = 1;
	}

	static void Axis_StepIfDue(Axis_t *a, uint32_t nowUs)
	{
		if ((uint32_t)(nowUs - a->lastStepUs) >= a->stepIntervalUs)
		{
			a->lastStepUs = nowUs;

			HAL_GPIO_WritePin(a->stepPort, a->stepPin, GPIO_PIN_SET);
			delay_us_custom(STEP_PULSE_US);
			HAL_GPIO_WritePin(a->stepPort, a->stepPin, GPIO_PIN_RESET);

			a->cmdSteps += a->dir;
		}
	}

	static uint8_t Limit_IsActive(GPIO_TypeDef *port, uint16_t pin)
	{
	#if USE_LIMITS
		return (HAL_GPIO_ReadPin(port, pin) == LIMIT_ACTIVE_STATE);
	#else
		return 0;
	#endif
	}

	static void Check_Limits_And_Reverse(void)
	{
		uint8_t xMin = Limit_IsActive(GPIOB, GPIO_PIN_0); // X_MIN PB0
		uint8_t xMax = Limit_IsActive(GPIOC, GPIO_PIN_1); // X_MAX PC1

		if (xMin && axis[0].dir < 0)
		{
			Axis_SetDir(&axis[0], +1);
		}

		if (xMax && axis[0].dir > 0)
		{
			Axis_SetDir(&axis[0], -1);
		}

		uint8_t zMin = Limit_IsActive(GPIOC, GPIO_PIN_0); // Z_MIN PC0
		uint8_t zMax = Limit_IsActive(GPIOA, GPIO_PIN_6); // Z_MAX PA6

		if (zMin && axis[1].dir < 0)
		{
			Axis_SetDir(&axis[1], +1);
		}

		if (zMax && axis[1].dir > 0)
		{
			Axis_SetDir(&axis[1], -1);
		}

		if (axis[2].cmdSteps >= C_SOFT_LIMIT_STEPS && axis[2].dir > 0)
		{
			Axis_SetDir(&axis[2], -1);
		}

		if (axis[2].cmdSteps <= -C_SOFT_LIMIT_STEPS && axis[2].dir < 0)
		{
			Axis_SetDir(&axis[2], +1);
		}
	}

	static void Print_Axis_Status(Axis_t *a)
	{
		char line[240];

		Axis_UpdateEncoder(a);

		int32_t expected_x100 =
			(int32_t)(((int64_t)a->cmdSteps * 36000LL) / a->stepsPerRev);

		int32_t measured_x100 =
			(int32_t)(((int64_t)a->encCounts * 36000LL) / 4096LL);

		measured_x100 *= a->encSign;

		int32_t error_x100 = measured_x100 - expected_x100;

		char expStr[20];
		char encStr[20];
		char errStr[20];

		format_x100(expStr, sizeof(expStr), expected_x100);
		format_x100(encStr, sizeof(encStr), measured_x100);
		format_x100(errStr, sizeof(errStr), error_x100);

		uint32_t absExpected = abs_i32(expected_x100);
		uint32_t absError = abs_i32(error_x100);

		if (a->encOk && absExpected > 100)
		{
			uint32_t errPct_x100 = (absError * 10000UL) / absExpected;

			snprintf(line, sizeof(line),
					 "%s | CMD:%s deg | ENC:%s deg | ERR:%s deg | ERR:%lu.%02lu%% | DIR:%+d\r\n",
					 a->name,
					 expStr,
					 encStr,
					 errStr,
					 (unsigned long)(errPct_x100 / 100),
					 (unsigned long)(errPct_x100 % 100),
					 a->dir);
		}
		else if (a->encOk)
		{
			snprintf(line, sizeof(line),
					 "%s | CMD:%s deg | ENC:%s deg | ERR:%s deg | ERR:-- | DIR:%+d\r\n",
					 a->name,
					 expStr,
					 encStr,
					 errStr,
					 a->dir);
		}
		else
		{
			snprintf(line, sizeof(line),
					 "%s | CMD:%s deg | ENC:ERR | DIR:%+d\r\n",
					 a->name,
					 expStr,
					 a->dir);
		}

		UART_Print(line);
	}

	static void Print_All_Status(void)
	{
		char header[180];

		uint8_t xMin = Limit_IsActive(GPIOB, GPIO_PIN_0);
		uint8_t xMax = Limit_IsActive(GPIOC, GPIO_PIN_1);
		uint8_t zMin = Limit_IsActive(GPIOC, GPIO_PIN_0);
		uint8_t zMax = Limit_IsActive(GPIOA, GPIO_PIN_6);

		snprintf(header, sizeof(header),
				 "\r\nLIMITS | X_MIN:%u X_MAX:%u Z_MIN:%u Z_MAX:%u\r\n",
				 xMin, xMax, zMin, zMax);

		UART_Print(header);

		Print_Axis_Status(&axis[0]);
		Print_Axis_Status(&axis[1]);
		Print_Axis_Status(&axis[2]);
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
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  DWT_Init_Custom();

  HAL_Delay(500);

  Axis_Init(&axis[0]);
  Axis_Init(&axis[1]);
  Axis_Init(&axis[2]);

  UART_Print("\r\nSystem motor + encoder comparison test started.\r\n");
  UART_Print("Switch type: NO + C, Pull-Up, active LOW.\r\n");
  UART_Print("All axes microstep: 1/64.\r\n");
  UART_Print("TMC EN active LOW. Motors enabled.\r\n");
  UART_Print("If encoder direction is opposite, change encSign from 1 to -1 for that axis.\r\n\r\n");

  lastPrintMs = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  uint32_t nowUs = micros_custom();

	  Check_Limits_And_Reverse();

	  Axis_StepIfDue(&axis[0], nowUs);
	  Axis_StepIfDue(&axis[1], nowUs);
	  Axis_StepIfDue(&axis[2], nowUs);

	  if ((HAL_GetTick() - lastPrintMs) >= 500)
	  {
	      lastPrintMs = HAL_GetTick();
	      Print_All_Status();
	  }
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, X_EN_Pin|Z_EN_Pin|C_EN_Pin|LD2_Pin
                          |C_STEP_Pin|X_STEP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(C_DIR_GPIO_Port, C_DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, X_DIR_Pin|Z_DIR_Pin|Z_STEP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Z_MIN_Pin X_MAX_Pin */
  GPIO_InitStruct.Pin = Z_MIN_Pin|X_MAX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : X_EN_Pin Z_EN_Pin C_EN_Pin LD2_Pin
                           C_STEP_Pin X_STEP_Pin */
  GPIO_InitStruct.Pin = X_EN_Pin|Z_EN_Pin|C_EN_Pin|LD2_Pin
                          |C_STEP_Pin|X_STEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Z_MAX_Pin */
  GPIO_InitStruct.Pin = Z_MAX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Z_MAX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : X_MIN_Pin */
  GPIO_InitStruct.Pin = X_MIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(X_MIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : C_DIR_Pin */
  GPIO_InitStruct.Pin = C_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(C_DIR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : X_DIR_Pin Z_DIR_Pin Z_STEP_Pin */
  GPIO_InitStruct.Pin = X_DIR_Pin|Z_DIR_Pin|Z_STEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
