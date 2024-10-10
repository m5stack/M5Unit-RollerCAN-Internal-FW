/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// #include "usart.h"
#include "u8g2_disp_fun.h"
#include "motordriver.h"
#include "mysys.h"
#include "myadc.h"
#include "smart_knob.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// void Usart_Receive_Data(USART_TypeDef *USARTx)
// {
//   if (LL_USART_IsActiveFlag_IDLE(USARTx) && LL_USART_IsEnabledIT_IDLE(USARTx))
//   {
//     LL_USART_ClearFlag_IDLE(USARTx);          
//     LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
//     dat_Uart1.LEN = UART1_RX_BUF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_2);

//     dat_Uart1.FLG = 1;
//     dat_Uart1.pRxBuf[dat_Uart1.LEN] = 0;

//     LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, UART1_RX_BUF_SIZE);
//     LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
//     if (dat_Uart1.LEN) {
//       comm_flash_count = 4;
//       if (dat_Uart1.LEN == 15) {
//         if (crc8_MAXIM(dat_Uart1.pRxBuf, 14) == dat_Uart1.pRxBuf[14] && dat_Uart1.pRxBuf[1] == motor_id) {
//           switch (dat_Uart1.pRxBuf[0])
//           {
//           case 1:
//             if (dat_Uart1.pRxBuf[2] >= 1 && dat_Uart1.pRxBuf[2] <= 4 && !err_stalled_flag) {
//               motor_mode = dat_Uart1.pRxBuf[2];
//               if (last_motor_mode != motor_mode) {
//                 MotorDriverSetCurrentReal(0);
//                 pid_ctrl_speed_t.setpoint = 0;
//                 pid_ctrl_speed_t.iTerm = 0;
//                 pid_ctrl_pos_t.iTerm = 0;
//                 pid_ctrl_pos_t.setpoint = 0;
//                 last_motor_mode = motor_mode;
//               }      

//               HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//               usart_tx_delay = HAL_GetTick();
//               usart_tx_flag = 1;
//               for (int i = 0; i < 17; i++) {
//                 dat_Uart1.pTxBuf[i] = 0;
//               }
//               dat_Uart1.pTxBuf[0] = 0xAA;
//               dat_Uart1.pTxBuf[1] = 0x55;
//               dat_Uart1.pTxBuf[2] = dat_Uart1.pRxBuf[0] + 0x10;
//               dat_Uart1.pTxBuf[3] = motor_id;
//               dat_Uart1.pTxBuf[4] = motor_mode;
//               dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//               UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);
//             }
//             break;

//           case 0:
//             motor_output = dat_Uart1.pRxBuf[2];
//             if (motor_output) {
//               if (!over_vol_flag && !err_stalled_flag)
//                 MotorDriverSetMode(MDRV_MODE_RUN);
//             }
//             else {
//               MotorDriverSetMode(MDRV_MODE_OFF);
//             } 
//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[3] = motor_id;
//             dat_Uart1.pTxBuf[4] = motor_output;
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 2:
//             if (dat_Uart1.pRxBuf[2])
//               over_vol_protect_mode = 1;
//             else
//               over_vol_protect_mode = 0;   

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[3] = motor_id;
//             dat_Uart1.pTxBuf[4] = over_vol_protect_mode;
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 3:
//             err_recover_try_max = dat_Uart1.pRxBuf[2];

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[3] = motor_id;
//             dat_Uart1.pTxBuf[4] = err_recover_try_max;
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 4:
//             memcpy((uint8_t *)&speed_err_value, &dat_Uart1.pRxBuf[2], 2);
//             speed_err_timeout = dat_Uart1.pRxBuf[6];

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &speed_err_value, 2);
//             dat_Uart1.pTxBuf[6+2] = speed_err_timeout;
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 5:
//             memcpy((uint8_t *)&pos_err_value, &dat_Uart1.pRxBuf[2], 2);
//             pos_err_timeout = dat_Uart1.pRxBuf[6];

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &pos_err_value, 2);
//             dat_Uart1.pTxBuf[6+2] = pos_err_timeout;
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 6:
//             if (dat_Uart1.pRxBuf[2]) {
//               if (over_vol_flag)
//                 sys_status = SYS_STANDBY;
//               over_vol_flag = 0;
//               error_code &= ~ERR_OVER_VOLTAGE;
//             }
//             if (dat_Uart1.pRxBuf[6]) {
//               speed_err_recover_try_counter = 0;
//               pos_err_recover_try_counter = 0;
//               if (motor_mode == MODE_SPEED_ERR_PROTECT) {
//                 sys_status = SYS_STANDBY;
//                 motor_mode = MODE_SPEED;
//               }
//               else if (motor_mode == MODE_POS_ERR_PROTECT) {
//                 sys_status = SYS_STANDBY;
//                 motor_mode = MODE_POS;
//               }
//               error_code &= ~ERR_STALLED;
//               speed_err_count_flag = 0;
//               speed_err_auto_flag = 0;
//               pos_err_count_flag = 0;
//               pos_err_auto_flag = 0;
//               err_stalled_flag = 0;
//             }

//           case 7:
//             if (dat_Uart1.pRxBuf[2]) {
//               flash_data_write_back();
//             }

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             dat_Uart1.pTxBuf[2+2] = dat_Uart1.pRxBuf[2];
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;
//           case 8:
//             memcpy((uint8_t *)&current_position, &dat_Uart1.pRxBuf[2], 4);

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &current_position, 4);
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;
//           case 9:
//             if (dat_Uart1.pRxBuf[2]) {
//               mode_switch_flag = 1;
//             }
//             else {
//               mode_switch_flag = 0;
//             }

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = dat_Uart1.pRxBuf[0] + 0x10;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             dat_Uart1.pTxBuf[2+2] = dat_Uart1.pRxBuf[2];
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 0x20:
//             memcpy((uint8_t *)&speed_point, &dat_Uart1.pRxBuf[2], 4);
//             memcpy((uint8_t *)&max_speed_current, &dat_Uart1.pRxBuf[6], 4);
//             pid_ctrl_speed_t.iTerm = 0;
//             pid_ctrl_speed_t.setpoint = (float)speed_point / 100.0f;
//             if (max_speed_current < 0)
//               max_speed_current = -max_speed_current;
//             if (max_speed_current > 120000)
//               max_speed_current = 120000;            
//             pid_ctrl_speed_t.outMin = -((float)max_speed_current / 100);
//             pid_ctrl_speed_t.outMax = (float)max_speed_current / 100;            

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x30;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &speed_point, 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[6+2], &max_speed_current, 4);
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 0x21:
//             memcpy((uint8_t *)&speed_pid_int[0], &dat_Uart1.pRxBuf[2], 4);
//             memcpy((uint8_t *)&speed_pid_int[1], &dat_Uart1.pRxBuf[6], 4);
//             memcpy((uint8_t *)&speed_pid_int[2], &dat_Uart1.pRxBuf[0x0A], 4);
//             for (int i = 0; i < 3; i++) {
//               speed_pid_float[i] = (float)speed_pid_int[i] / 10000000;
//             }
//             PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1], speed_pid_float[2]);
//             pid_ctrl_speed_t.iTerm = 0;          

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x31;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &speed_pid_int[0], 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[6+2], &speed_pid_int[1], 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[0x0A+2], &speed_pid_int[2], 4);
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 0x22:
//             memcpy((uint8_t *)&pos_point, &dat_Uart1.pRxBuf[2], 4);
//             memcpy((uint8_t *)&max_pos_current, &dat_Uart1.pRxBuf[6], 4);
//             pid_ctrl_pos_t.setpoint = (float)pos_point / 100.0f;
//             if (max_pos_current < 0)
//               max_pos_current = -max_pos_current;
//             if (max_pos_current > 120000)
//               max_pos_current = 120000;

//             pid_ctrl_pos_t.outMin = -((float)max_pos_current / 100);
//             pid_ctrl_pos_t.outMax = (float)max_pos_current / 100;           

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x32;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &pos_point, 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[6+2], &max_pos_current, 4);
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;

//           case 0x23:
//             memcpy((uint8_t *)&pos_pid_int[0], &dat_Uart1.pRxBuf[2], 4);
//             memcpy((uint8_t *)&pos_pid_int[1], &dat_Uart1.pRxBuf[6], 4);
//             memcpy((uint8_t *)&pos_pid_int[2], &dat_Uart1.pRxBuf[0x0A], 4);
//             for (int i = 0; i < 3; i++) {
//               pos_pid_float[i] = (float)pos_pid_int[i] / 10000000;
//             }
//             PIDTuningsSet(&pid_ctrl_pos_t, pos_pid_float[0], pos_pid_float[1], pos_pid_float[2]);
//             pid_ctrl_pos_t.iTerm = 0;           

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x33;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &pos_pid_int[0], 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[6+2], &pos_pid_int[1], 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[0x0A+2], &pos_pid_int[2], 4);
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;  

//           case 0x24:
//             memcpy((uint8_t *)&current_point, &dat_Uart1.pRxBuf[2], 4);
//             if (current_point > 120000)
//               current_point = 120000;
//             else if (current_point < -120000)
//               current_point = -120000;

//             float current_set = (float)current_point / 100.0f;

//             MotorDriverSetCurrentReal(current_set);          

//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 17; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x34;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &current_point, 4);
//             dat_Uart1.pTxBuf[14+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 14);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 17);            
//             break;                                  
          
//           default:
//             break;
//           }
//         }
//       }
//       else if (dat_Uart1.LEN == 4) {
//         if (crc8_MAXIM(dat_Uart1.pRxBuf, 3) == dat_Uart1.pRxBuf[3] && dat_Uart1.pRxBuf[1] == motor_id &&
//             dat_Uart1.pRxBuf[0] == 0x30) {
//           if (!dat_Uart1.pRxBuf[2]) {
//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 20; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x40;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             int32_t motor_rpm_int = motor_rpm * 100;       
//             int32_t mechanical_angle_int = mechanical_angle * 100;   
//             int32_t ph_current_int = ph_crrent_lpf * 100;  
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &motor_rpm_int, 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[6+2], &mechanical_angle_int, 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[10+2], &ph_current_int, 4);
//             dat_Uart1.pTxBuf[14+2] = motor_mode;
//             dat_Uart1.pTxBuf[15+2] = sys_status;
//             dat_Uart1.pTxBuf[16+2] = error_code;
//             dat_Uart1.pTxBuf[17+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 17);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 20);             
//           }
//         }
//         else if (crc8_MAXIM(dat_Uart1.pRxBuf, 3) == dat_Uart1.pRxBuf[3] && dat_Uart1.pRxBuf[1] == motor_id &&
//             dat_Uart1.pRxBuf[0] == 0x31) {
//           if (!dat_Uart1.pRxBuf[2]) {
//             HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_SET); 
//             usart_tx_delay = HAL_GetTick();
//             usart_tx_flag = 1;  
//             for (int i = 0; i < 20; i++) {
//               dat_Uart1.pTxBuf[i] = 0;
//             }
//             dat_Uart1.pTxBuf[0] = 0xAA;
//             dat_Uart1.pTxBuf[1] = 0x55;
//             dat_Uart1.pTxBuf[0+2] = 0x41;
//             dat_Uart1.pTxBuf[1+2] = motor_id;
//             int32_t vol_int32 = (int32_t)vol_lpf; 
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[2+2], &vol_int32, 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[6+2], &internal_temp, 4);
//             memcpy((uint8_t *)&dat_Uart1.pTxBuf[10+2], &current_position, 4);
//             dat_Uart1.pTxBuf[17+2] = crc8_MAXIM((uint8_t *)&dat_Uart1.pTxBuf[0+2], 17);
//             UART_DMA_Send(USART3, dat_Uart1.pTxBuf, 20);             
//           }
//         }
//       }
//     }    
//   }
//   if (LL_USART_IsActiveFlag_TC(USARTx))
//   {
//     LL_USART_DisableIT_TC(USARTx);          
//     LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
//     HAL_GPIO_WritePin(GPIOB, RS485_DIR_Pin, GPIO_PIN_RESET);
//     usart_tx_flag = 0;
//   }
// }
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern FDCAN_HandleTypeDef hfdcan1;
extern DMA_HandleTypeDef hdma_tim3_ch2;
extern TIM_HandleTypeDef htim1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel3 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */

  /* USER CODE END DMA1_Channel3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_tim3_ch2);
  /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

  /* USER CODE END DMA1_Channel3_IRQn 1 */
}

/**
  * @brief This function handles FDCAN1 interrupt 0.
  */
void FDCAN1_IT0_IRQHandler(void)
{
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 0 */

  /* USER CODE END FDCAN1_IT0_IRQn 0 */
  HAL_FDCAN_IRQHandler(&hfdcan1);
  /* USER CODE BEGIN FDCAN1_IT0_IRQn 1 */

  /* USER CODE END FDCAN1_IT0_IRQn 1 */
}

/**
  * @brief This function handles I2C1 event interrupt / I2C1 wake-up interrupt through EXTI line 23.
  */
__weak void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */

  /* USER CODE END I2C1_EV_IRQn 0 */

  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C1 error interrupt.
  */
__weak void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */

  /* USER CODE END I2C1_ER_IRQn 0 */

  /* USER CODE BEGIN I2C1_ER_IRQn 1 */

  /* USER CODE END I2C1_ER_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
