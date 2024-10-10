/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.h
  * @brief   This file contains all the function prototypes for
  *          the fdcan.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
extern uint8_t can_id;
extern uint8_t can_change_flag;
extern uint8_t flash_data_write_back_flag;
extern uint8_t change_baudrate_flag;
extern uint32_t change_baudrate_delay;
/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

/* USER CODE BEGIN Private defines */
enum can_func_t {
  FUNC_SAVE_FLASH = 0x7002,
  FUNC_REMOVE_PROTECTION = 0x7003,
  FUNC_ON_OFF = 0x7004,
  FUNC_RUN_MODE = 0x7005,
  FUNC_CURRENT_SETTING = 0x7006,
  FUNC_SPEED_SETTING = 0x700A,
  FUNC_POSITION_SETTING = 0x7016,
  FUNC_POSITION_MAX_CURRENT = 0x7017,
  FUNC_SPEED_MAX_CURRENT = 0x7018,
  FUNC_SPEED_KP = 0x7020,
  FUNC_SPEED_KI = 0x7021,
  FUNC_SPEED_KD = 0x7022,
  FUNC_POSITION_KP = 0x7023,
  FUNC_POSITION_KI = 0x7024,
  FUNC_POSITION_KD = 0x7025,
  FUNC_READBACK_SPEED = 0x7030,
  FUNC_READBACK_POSITION = 0x7031,
  FUNC_READBACK_CURRENT = 0x7032,
  FUNC_DIAL_COUNTER = 0x7033,
  FUNC_VIN = 0x7034,
  FUNC_TEMP = 0x7035,
  FUNC_OVERVOLTAGE_PROTECTION_RELEASE_MODE = 0x7040,
  FUNC_MAX_STALL_PROTECTION_ATTEMPTS = 0x7041,
  FUNC_SPEED_MODE_STALL_THRESHOLD = 0x7042,
  FUNC_SPEED_MODE_STALL_TIMEOUT = 0x7043,
  FUNC_POSITION_MODE_STALL_THRESHOLD = 0x7044,
  FUNC_POSITION_MODE_STALL_TIMEOUT = 0x7045,
  FUNC_RGB_MODE = 0x7050,
  FUNC_RGB_COLOR = 0x7051,
  FUNC_RGB_BRIGHTNESS = 0x7052
};
/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);

/* USER CODE BEGIN Prototypes */
void user_fdcan_init(void);
uint8_t FDCAN1_Send_Msg(uint8_t cmd_id, uint16_t option, uint8_t can_id, uint8_t* msg);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */

