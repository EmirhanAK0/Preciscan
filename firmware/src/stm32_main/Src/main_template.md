/* 
 * PRECISCAN STM32 MIGRATION - MAIN.C INTEGRATION GUIDE
 * 
 * Bu dosya, CubeMX tarafindan uretilen main.c dosyaniza eklemeniz gereken
 * kod parcalarini icerir. CubeMX ile projenizi olusturduktan sonra, asagidaki
 * 'USER CODE BEGIN' bloklarini kendi main.c dosyaniza kopyalayabilirsiniz.
 */

/* USER CODE BEGIN Includes */
#include "app_controller.h"
#include "hal_time.h"
#include "uart_comm.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
AppController app;
/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */
  
  // DWT Time Sayacini Baslat
  hal_time_init();

  // UART buffer'larini ilklendir
  uart_comm_init();

  // UART Receive Interrupt'ini baslat (1 byte olarak)
  // NOT: huart2 kendi projenize gore degisebilir
  uint8_t rx_byte;
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

  // Uygulama denetleyicisini baslat
  app_init(&app);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /* USER CODE END WHILE */

      /* USER CODE BEGIN 3 */
      // Ana Dongu - Herhangi bir Delay() OLMAMALIDIR!
      app_update(&app, hal_millis(), hal_micros());
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

// UART Rx Complete Callback fonksiyonu
// Her karakter geldiginde cagrilir.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) // Kendi UART donaniminiza gore degistirin
    {
        extern uint8_t rx_byte; // Yukarida tanimladigimiz degisken
        
        // Karakteri buffer'a ekle
        uart_comm_rx_callback(rx_byte);
        
        // Bir sonraki karakteri beklemek icin Interrupt'i tekrar kur
        HAL_UART_Receive_IT(huart, &rx_byte, 1);
    }
}

/* USER CODE END 4 */
