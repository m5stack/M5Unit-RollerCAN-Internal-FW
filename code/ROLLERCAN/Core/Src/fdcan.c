/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "fdcan.h"

/* USER CODE BEGIN 0 */
#include "mysys.h"
#include "motordriver.h"
#include "smart_knob.h"
#include "u8g2_disp_fun.h"
#include "myadc.h"
#include "i2c.h"
#include "ws2812.h"

FDCAN_TxHeaderTypeDef fdcan1_TxHeader;
uint8_t can_id = 168;
uint8_t can_change_flag = 0;
uint8_t flash_data_write_back_flag = 0;
uint8_t change_baudrate_flag = 0;
uint32_t change_baudrate_delay = 0;
/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV4;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 32;
  hfdcan1.Init.NominalTimeSeg2 = 9;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 32;
  hfdcan1.Init.DataTimeSeg2 = 9;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  /* 配置过滤器以接收经典 CAN 帧 */
  FDCAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // 或者 FDCAN_FILTER_TO_RXBUFFER
  sFilterConfig.FilterID1 = (uint32_t)can_id;  // 示例的标准 ID，过滤器会将该 ID 的帧放入接收 FIFO 或者接收缓冲区
  sFilterConfig.FilterID2 = 0x000000FF;  // 如果不需要双标识符过滤器，可以不用配置
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, ENABLE, ENABLE);

  /* 注册 FDCAN 接收中断回调函数 */
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    Error_Handler();
  }    

  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void user_fdcan_init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */
  if (bps_index > 2)
    bps_index = 2;
  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV4;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = bps_list[bps_index];
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 32;
  hfdcan1.Init.NominalTimeSeg2 = 9;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 32;
  hfdcan1.Init.DataTimeSeg2 = 9;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  /* 配置过滤器以接收经典 CAN 帧 */
  FDCAN_FilterTypeDef sFilterConfig;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // 或者 FDCAN_FILTER_TO_RXBUFFER
  sFilterConfig.FilterID1 = (uint32_t)can_id;  // 示例的标准 ID，过滤器会将该 ID 的帧放入接收 FIFO 或者接收缓冲区
  sFilterConfig.FilterID2 = 0x000000FF;  // 如果不需要双标识符过滤器，可以不用配置
  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, ENABLE, ENABLE);

  /* 注册 FDCAN 接收中断回调函数 */
  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
  {
    Error_Handler();
  }    

  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END FDCAN1_Init 2 */

}

uint8_t identifier_to_cmd(uint32_t id)
{
  return (id >> 24) & 0x1F;
}

uint8_t identifier_to_cmd_para(uint32_t id)
{
  return (id >> 16) & 0xFF;
}

uint16_t identifier_to_option(uint32_t id)
{
  return (id >> 8) & 0xFFFF;
}

uint16_t feedback_option_data(uint8_t my_can_id, uint8_t *data)
{
  uint16_t tx_option;
  uint8_t tx_status;

  tx_option = my_can_id;

  tx_status = (error_code & 0x07);

  uint8_t motor_mode_temp = motor_mode;
  if (motor_mode == MODE_SPEED_ERR_PROTECT) {
    motor_mode_temp = MODE_SPEED;
  }
  else if (motor_mode == MODE_POS_ERR_PROTECT) {
    motor_mode_temp = MODE_POS;
  }        
  tx_status |= (motor_mode_temp << 3);
  tx_status |= (sys_status << 6);
  tx_option |= (tx_status << 8);  

  int16_t rpm_temp = (int16_t)motor_rpm;
  memcpy(&data[0], &rpm_temp, 2);
  int16_t position_temp = (int16_t)mechanical_angle;
  memcpy(&data[2], &position_temp, 2);
  int16_t current_temp = (int16_t)ph_crrent_lpf;
  memcpy(&data[4], &current_temp, 2);
  int16_t vin_temp = (int16_t)(vol_lpf/100.0f);
  memcpy(&data[6], &vin_temp, 2);  

  return tx_option;
}

void feedback_function_read(uint8_t *data, uint16_t index, int32_t tx_data)
{
  memcpy(&data[0], &index, 2);
  memcpy(&data[4], &tx_data, 4);
}

uint8_t FDCAN1_Send_Msg(uint8_t cmd_id, uint16_t option, uint8_t can_id, uint8_t* msg)
{	
    fdcan1_TxHeader.Identifier = ((cmd_id << 24) | (option << 8) | can_id);
    fdcan1_TxHeader.IdType=FDCAN_EXTENDED_ID;                  //标准ID
    fdcan1_TxHeader.TxFrameType=FDCAN_DATA_FRAME;              //数据
    fdcan1_TxHeader.DataLength=FDCAN_DLC_BYTES_8;                            //数据长度
    fdcan1_TxHeader.ErrorStateIndicator=FDCAN_ESI_PASSIVE;            
    fdcan1_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;               //关闭速率切换
    fdcan1_TxHeader.FDFormat=FDCAN_CLASSIC_CAN;                //传统的CAN模式
    fdcan1_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;     //无发送事
    fdcan1_TxHeader.MessageMarker=0;       

    if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1,&fdcan1_TxHeader,msg)!=HAL_OK) return 1;

    return 0;                    	
}

/* FDCAN 接收中断回调函数 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];    
		uint16_t my_tx_option;
    uint8_t motor_mode_temp;
    int32_t int_value_temp;
    uint8_t i2c_address, i2c_len, i2c_success, is_stop_bit;

    comm_flash_count = 4;

    /* 从 FIFO0 读取消息 */
    if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
    {
      uint8_t cmd_id = identifier_to_cmd(rx_header.Identifier);
      uint8_t cmd_para = identifier_to_cmd_para(rx_header.Identifier);
      uint8_t option = identifier_to_option(rx_header.Identifier);
			uint8_t data[8] = {0};
			uint16_t func_index = 0;
			uint16_t i2c_address = 0;
      uint32_t i2c_data = 0;
			int32_t func_data = 0;

      switch (cmd_id)
      {
      case 0:
        FDCAN1_Send_Msg(cmd_id, can_id, 0xFE, data);
        break;
      case 3:
        motor_output = 1;
        if (motor_output) {
          if (!over_vol_flag && !err_stalled_flag && motor_mode < MODE_MAX) {
            if (motor_mode == MODE_DIAL && motor_disable_flag) {
              init_smart_knob();
            }             
            motor_disable_flag = 0;
            MotorDriverSetMode(MDRV_MODE_RUN);
          }
        }
        else {
          motor_disable_flag = 1;
          MotorDriverSetMode(MDRV_MODE_OFF);
        }      
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 4:
        motor_output = 0;
        if (motor_output) {
          if (!over_vol_flag && !err_stalled_flag) {
            motor_disable_flag = 0;
            MotorDriverSetMode(MDRV_MODE_RUN);
          }
        }
        else {
          motor_disable_flag = 1;
          MotorDriverSetMode(MDRV_MODE_OFF);
        }      
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 7:
        can_id = cmd_para;
        can_change_flag = 1;

        FDCAN1_Send_Msg(0, can_id, 0xFE, data);
        break;
      // case 8:
      //   if (over_vol_flag)
      //     sys_status = SYS_STANDBY;
      //   over_vol_flag = 0;
      //   error_code &= ~ERR_OVER_VOLTAGE;  
      //   my_tx_option = feedback_option_data(can_id, data);

      //   FDCAN1_Send_Msg(2, my_tx_option, option, data);
      //   break;
      case 9:
        speed_err_recover_try_counter = 0;
        pos_err_recover_try_counter = 0;
        if (motor_mode == MODE_SPEED_ERR_PROTECT) {
          sys_status = SYS_STANDBY;
          motor_mode = MODE_SPEED;
        }
        else if (motor_mode == MODE_POS_ERR_PROTECT) {
          sys_status = SYS_STANDBY;
          motor_mode = MODE_POS;
        }
        error_code &= ~ERR_STALLED;
        speed_err_count_flag = 0;
        speed_err_auto_flag = 0;
        pos_err_count_flag = 0;
        pos_err_auto_flag = 0;
        err_stalled_flag = 0;
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 10:
        flash_data_write_back_flag = 1;
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 11:
        if (cmd_para <= 2) {
          change_baudrate_flag = 1;
          change_baudrate_delay = HAL_GetTick();
          bps_index = cmd_para;
        }         
        my_tx_option = (option | (bps_index << 8));
        FDCAN1_Send_Msg(cmd_id, my_tx_option, can_id, data);
        break;
      case 12:
        motor_stall_protection_flag = 1;
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 13:
        motor_stall_protection_flag = 0;
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 14:
        motor_overvalue_protection_flag = 1;
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 15:
        motor_overvalue_protection_flag = 0;
        my_tx_option = feedback_option_data(can_id, data);

        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 17:
        memcpy(&func_index, &rx_data[0], 2);
        switch (func_index)
        {
        case FUNC_ON_OFF:
            motor_output ? (motor_output = 1) : (motor_output = 0);
            feedback_function_read(data, func_index, motor_output);
          break;
        case FUNC_RUN_MODE:
            motor_mode_temp = motor_mode;
            if (motor_mode == MODE_SPEED_ERR_PROTECT) {
              motor_mode_temp = MODE_SPEED;
            }
            else if (motor_mode == MODE_POS_ERR_PROTECT) {
              motor_mode_temp = MODE_POS;
            } 
            feedback_function_read(data, func_index, motor_mode_temp);
          break;
        case FUNC_CURRENT_SETTING:
            feedback_function_read(data, func_index, current_point);
          break;
        case FUNC_SPEED_SETTING:
            feedback_function_read(data, func_index, speed_point);
          break;
        case FUNC_POSITION_SETTING:
            feedback_function_read(data, func_index, pos_point);
          break;
        case FUNC_POSITION_MAX_CURRENT:
            feedback_function_read(data, func_index, max_pos_current);
          break;
        case FUNC_SPEED_MAX_CURRENT:
            feedback_function_read(data, func_index, max_speed_current);
          break;
        case FUNC_SPEED_KP:
            switch (speed_pid_index)
            {
            case 0:
              feedback_function_read(data, func_index, speed_pid_int[0]);
              break;
            case 1:
              feedback_function_read(data, func_index, speed_pid_low_int[0]);
              break;
            case 2:
              feedback_function_read(data, func_index, speed_pid_mid_int[0]);
              break;
            case 3:
              feedback_function_read(data, func_index, speed_pid_high_int[0]);
              break;
            
            default:
              break;
            }
          break;
        case FUNC_SPEED_KI:
            switch (speed_pid_index)
            {
            case 0:
              feedback_function_read(data, func_index, speed_pid_int[1]);
              break;
            case 1:
              feedback_function_read(data, func_index, speed_pid_low_int[1]);
              break;
            case 2:
              feedback_function_read(data, func_index, speed_pid_mid_int[1]);
              break;
            case 3:
              feedback_function_read(data, func_index, speed_pid_high_int[1]);
              break;
            
            default:
              break;
            }        
          break;          
        case FUNC_SPEED_KD:
            switch (speed_pid_index)
            {
            case 0:
              feedback_function_read(data, func_index, speed_pid_int[2]);
              break;
            case 1:
              feedback_function_read(data, func_index, speed_pid_low_int[2]);
              break;
            case 2:
              feedback_function_read(data, func_index, speed_pid_mid_int[2]);
              break;
            case 3:
              feedback_function_read(data, func_index, speed_pid_high_int[2]);
              break;
            
            default:
              break;
            }            
          break;          
        case FUNC_POSITION_KP:
            switch (pos_pid_index)
            {
            case 0:
              feedback_function_read(data, func_index, pos_pid_int[0]);
              break;
            case 1:
              feedback_function_read(data, func_index, pos_pid_low_int[0]);
              break;
            case 2:
              feedback_function_read(data, func_index, pos_pid_mid_int[0]);
              break;
            case 3:
              feedback_function_read(data, func_index, pos_pid_high_int[0]);
              break;
            
            default:
              break;
            }         
          break;
        case FUNC_POSITION_KI:
            switch (pos_pid_index)
            {
            case 0:
              feedback_function_read(data, func_index, pos_pid_int[1]);
              break;
            case 1:
              feedback_function_read(data, func_index, pos_pid_low_int[1]);
              break;
            case 2:
              feedback_function_read(data, func_index, pos_pid_mid_int[1]);
              break;
            case 3:
              feedback_function_read(data, func_index, pos_pid_high_int[1]);
              break;
            
            default:
              break;
            }                 
          break;        
        case FUNC_POSITION_KD:
            switch (pos_pid_index)
            {
            case 0:
              feedback_function_read(data, func_index, pos_pid_int[2]);
              break;
            case 1:
              feedback_function_read(data, func_index, pos_pid_low_int[2]);
              break;
            case 2:
              feedback_function_read(data, func_index, pos_pid_mid_int[2]);
              break;
            case 3:
              feedback_function_read(data, func_index, pos_pid_high_int[2]);
              break;
            
            default:
              break;
            }          
          break;            
        case FUNC_READBACK_SPEED:
            int_value_temp = motor_rpm * 100;        
            feedback_function_read(data, func_index, int_value_temp);
          break;            
        case FUNC_READBACK_POSITION:
            int_value_temp = mechanical_angle * 100;      
            feedback_function_read(data, func_index, int_value_temp);
          break;            
        case FUNC_READBACK_CURRENT:
            int_value_temp = ph_crrent_lpf * 100;      
            feedback_function_read(data, func_index, int_value_temp);
          break;            
        case FUNC_DIAL_COUNTER:
            feedback_function_read(data, func_index, current_position);
          break;            
        case FUNC_VIN:
            int_value_temp = (int32_t)vol_lpf;
            feedback_function_read(data, func_index, int_value_temp);
          break;            
        case FUNC_TEMP:
            feedback_function_read(data, func_index, internal_temp);
          break;            
        case FUNC_RGB_MODE:
            feedback_function_read(data, func_index, rgb_show_mode);
          break;                   
        case FUNC_RGB_COLOR:
            feedback_function_read(data, func_index, lastest_rgb_color);
          break;                   
        case FUNC_RGB_BRIGHTNESS:
            feedback_function_read(data, func_index, brightness_index);
          break;                   
        
        default:
          break;
        }
        FDCAN1_Send_Msg(cmd_id, option, can_id, data);
        break;
      case 18:
        memcpy(&func_index, &rx_data[0], 2);
        memcpy(&func_data, &rx_data[4], 4);
        switch (func_index)
        {
        case FUNC_SAVE_FLASH:
          if (abs(func_data)) {
            flash_data_write_back_flag = 1;
          }

          break;
        case FUNC_REMOVE_PROTECTION:
          if (abs(func_data)) {
            speed_err_recover_try_counter = 0;
            pos_err_recover_try_counter = 0;
            if (motor_mode == MODE_SPEED_ERR_PROTECT) {
              sys_status = SYS_STANDBY;
              motor_mode = MODE_SPEED;
            }
            else if (motor_mode == MODE_POS_ERR_PROTECT) {
              sys_status = SYS_STANDBY;
              motor_mode = MODE_POS;
            }
            error_code &= ~ERR_STALLED;
            speed_err_count_flag = 0;
            speed_err_auto_flag = 0;
            pos_err_count_flag = 0;
            pos_err_auto_flag = 0;
            err_stalled_flag = 0;
          }

          break;
        case FUNC_ON_OFF:
          motor_output = abs(func_data);
          if (motor_output) {
            if (!over_vol_flag && !err_stalled_flag && motor_mode < MODE_MAX) {
              if (motor_mode == MODE_DIAL && motor_disable_flag) {
                init_smart_knob();
              }            
              motor_disable_flag = 0;
              MotorDriverSetMode(MDRV_MODE_RUN);
            }
          }
          else {
            motor_disable_flag = 1;
            MotorDriverSetMode(MDRV_MODE_OFF);
          }

          break;
        case FUNC_RUN_MODE:
          if (func_data && func_data < MODE_MAX && !err_stalled_flag) {
            motor_mode = func_data;
            if (last_motor_mode != motor_mode) {
              if (motor_mode < MODE_DIAL) {
                MotorDriverSetCurrentReal(0);
                pid_ctrl_speed_t.iTerm = 0;
                pid_ctrl_pos_t.iTerm = 0;
              }
              else {
                init_smart_knob();
              }
              last_motor_mode = motor_mode;
            }
          } 

          break;
        case FUNC_CURRENT_SETTING:
          current_point = func_data;
          if (current_point > 120000)
            current_point = 120000;
          else if (current_point < -120000)
            current_point = -120000;

          float current_set = (float)current_point / 100.0f;
          MotorDriverSetCurrentReal(current_set);
          break;
        case FUNC_SPEED_SETTING:
            speed_point = func_data;
            // pid_ctrl_speed_t.iTerm = 0;
            if (speed_point > MY_INT32_MAX)
              speed_point = MY_INT32_MAX;
            else if (speed_point < MY_INT32_MIN)
              speed_point = MY_INT32_MIN;              
            pid_ctrl_speed_t.setpoint = (float)speed_point / 100.0f;
          break;
        case FUNC_POSITION_SETTING:
            pos_point = func_data;
            if (pos_point > MY_INT32_MAX)
              pos_point = MY_INT32_MAX;
            else if (pos_point < MY_INT32_MIN)
              pos_point = MY_INT32_MIN;              
            pid_ctrl_pos_t.setpoint = (float)pos_point / 100.0f;
          break;
        case FUNC_POSITION_MAX_CURRENT:
          max_pos_current = func_data;
          if (max_pos_current < 0)
            max_pos_current = -max_pos_current;
          if (max_pos_current > 120000)
            max_pos_current = 120000;

          pid_ctrl_pos_t.outMin = -((float)max_pos_current / 100);
          pid_ctrl_pos_t.outMax = (float)max_pos_current / 100;
          break;
        case FUNC_SPEED_MAX_CURRENT:
          max_speed_current = func_data;
          if (max_speed_current < 0)
            max_speed_current = -max_speed_current;
          if (max_speed_current > 120000)
            max_speed_current = 120000;

          pid_ctrl_speed_t.outMin = -((float)max_speed_current / 100);
          pid_ctrl_speed_t.outMax = (float)max_speed_current / 100;
          break;
        case FUNC_SPEED_KP:
          speed_pid_int[0] = func_data;
          speed_pid_float[0] = (float)speed_pid_int[0] / 100000;
          if (speed_pid_index == 0) {
            PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1], speed_pid_float[2]);
            pid_ctrl_speed_t.iTerm = 0;   
          }
          break;
        case FUNC_SPEED_KI:
          speed_pid_int[1] = func_data;
          speed_pid_float[1] = (float)speed_pid_int[1] / 10000000;
          if (speed_pid_index == 0) {
            PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1], speed_pid_float[2]);
            pid_ctrl_speed_t.iTerm = 0; 
          }
          break;          
        case FUNC_SPEED_KD:
          speed_pid_int[2] = func_data;
          speed_pid_float[2] = (float)speed_pid_int[2] / 100000;
          if (speed_pid_index == 0) {
            PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1], speed_pid_float[2]);
            pid_ctrl_speed_t.iTerm = 0; 
          }
          break;          
        case FUNC_POSITION_KP:
          pos_pid_int[0] = func_data;
          pos_pid_float[0] = (float)pos_pid_int[0] / 100000;
          if (pos_pid_index == 0) {
            PIDTuningsSet(&pid_ctrl_pos_t, pos_pid_float[0], pos_pid_float[1], pos_pid_float[2]);
            pid_ctrl_pos_t.iTerm = 0;  
          }
          break;
        case FUNC_POSITION_KI:
          pos_pid_int[1] = func_data;
          pos_pid_float[1] = (float)pos_pid_int[1] / 10000000;
          if (pos_pid_index == 0) {
            PIDTuningsSet(&pid_ctrl_pos_t, pos_pid_float[0], pos_pid_float[1], pos_pid_float[2]);
            pid_ctrl_pos_t.iTerm = 0;   
          }
          break;        
        case FUNC_POSITION_KD:
          pos_pid_int[2] = func_data;
          pos_pid_float[2] = (float)pos_pid_int[2] / 100000;
          if (pos_pid_index == 0) {
            PIDTuningsSet(&pid_ctrl_pos_t, pos_pid_float[0], pos_pid_float[1], pos_pid_float[2]);
            pid_ctrl_pos_t.iTerm = 0;  
          }
          break;        
        case FUNC_DIAL_COUNTER:
          current_position = func_data;
          break;        
        case FUNC_RGB_MODE:
          memcpy(&rgb_show_mode, &func_data, 1);
          if (rgb_show_mode > 1)
            rgb_show_mode = 1;
          break;      
        case FUNC_RGB_COLOR:
          memcpy(&rgb_color_buffer[rgb_color_buffer_index], &func_data, 3);
          if (rgb_color_buffer[rgb_color_buffer_index]) {
            lastest_rgb_color = rgb_color_buffer[rgb_color_buffer_index];
            if (rgb_color_buffer_index < (RGB_BUFFER_SIZE-1))   
              ++rgb_color_buffer_index;  
          }          
          break;        
        case FUNC_RGB_BRIGHTNESS:
          if (func_data <= 100 && func_data >= 0) {
            memcpy(&brightness_index, &func_data, 1);        
            ws2812_show();
          }
          break;        
        
        default:
          break;
        }
        my_tx_option = feedback_option_data(can_id, data);
        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 19:
        LL_I2C_Disable(I2C1);
        I2C1_Start();
        memcpy(&func_index, &rx_data[0], 2);
        memcpy(&i2c_address, &rx_data[2], 2);
        i2c_address = ((i2c_address << 1) | 1);
        switch (func_index)
        {
        case FUNC_ON_OFF:
            I2C_Read_Bytes(i2c_address, 0, (uint8_t *)&i2c_data, 1, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_RUN_MODE:
            I2C_Read_Bytes(i2c_address, 1, (uint8_t *)&i2c_data, 1, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_CURRENT_SETTING:
            I2C_Read_Bytes(i2c_address, 0xB0, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);
      
            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_SPEED_SETTING:
            I2C_Read_Bytes(i2c_address, 0x40, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_POSITION_SETTING:
            I2C_Read_Bytes(i2c_address, 0x80, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_POSITION_MAX_CURRENT:
            I2C_Read_Bytes(i2c_address, 0x20, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_SPEED_MAX_CURRENT:
            I2C_Read_Bytes(i2c_address, 0x50, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_SPEED_KP:
            I2C_Read_Bytes(i2c_address, 0x70, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);
          break;
        case FUNC_SPEED_KI:
            I2C_Read_Bytes(i2c_address, 0x74, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);      
          break;          
        case FUNC_SPEED_KD:
            I2C_Read_Bytes(i2c_address, 0x78, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);            
          break;          
        case FUNC_POSITION_KP:
            I2C_Read_Bytes(i2c_address, 0xA0, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);          
          break;
        case FUNC_POSITION_KI:
            I2C_Read_Bytes(i2c_address, 0xA4, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data);   
          break;        
        case FUNC_POSITION_KD:
            I2C_Read_Bytes(i2c_address, 0xA8, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_READBACK_SPEED:
            I2C_Read_Bytes(i2c_address, 0x60, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_READBACK_POSITION:
            I2C_Read_Bytes(i2c_address, 0x90, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_READBACK_CURRENT:
            I2C_Read_Bytes(i2c_address, 0xC0, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_DIAL_COUNTER:
            I2C_Read_Bytes(i2c_address, 0x3C, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_VIN:
            I2C_Read_Bytes(i2c_address, 0x34, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_TEMP:
            I2C_Read_Bytes(i2c_address, 0x38, (uint8_t *)&i2c_data, 4, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_RGB_MODE:
            I2C_Read_Bytes(i2c_address, 0x33, (uint8_t *)&i2c_data, 1, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;
        case FUNC_RGB_COLOR:
            I2C_Read_Bytes(i2c_address, 0x30, (uint8_t *)&i2c_data, 3, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        case FUNC_RGB_BRIGHTNESS:
            I2C_Read_Bytes(i2c_address, 0x12, (uint8_t *)&i2c_data, 1, I2C_TIMOUT_MS);

            feedback_function_read(data, func_index, i2c_data); 
          break;            
        
        default:
          break;
        }
        FDCAN1_Send_Msg(cmd_id, option, can_id, data);
        break;        
      case 20:
        LL_I2C_Disable(I2C1);
        I2C1_Start();
        memcpy(&func_index, &rx_data[0], 2);
        memcpy(&func_data, &rx_data[4], 4);
        memcpy(&i2c_address, &rx_data[2], 2);
        i2c_address = (i2c_address << 1);

        switch (func_index)
        {
        case FUNC_ON_OFF:
          I2C_Write_Bytes(i2c_address, 0x00, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS);

          break;
        case FUNC_REMOVE_PROTECTION:
          I2C_Write_Bytes(i2c_address, 0x03, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS);
          I2C_Write_Bytes(i2c_address, 0x0B, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS);

          break;
        case FUNC_SAVE_FLASH:
          I2C_Write_Bytes(i2c_address, 0xF0, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS);

          break;
        case FUNC_RUN_MODE:
          I2C_Write_Bytes(i2c_address, 0x01, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS); 

          break;
        case FUNC_CURRENT_SETTING:
          I2C_Write_Bytes(i2c_address, 0xB0, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS); 
          break;
        case FUNC_SPEED_SETTING:
            I2C_Write_Bytes(i2c_address, 0x40, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS); 
          break;
        case FUNC_POSITION_SETTING:
            I2C_Write_Bytes(i2c_address, 0x80, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);
          break;
        case FUNC_POSITION_MAX_CURRENT:
          I2C_Write_Bytes(i2c_address, 0x20, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);
          break;
        case FUNC_SPEED_MAX_CURRENT:
          I2C_Write_Bytes(i2c_address, 0x50, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);
          break;
        case FUNC_SPEED_KP:
          I2C_Write_Bytes(i2c_address, 0x70, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);       
          break;
        case FUNC_SPEED_KI:
          I2C_Write_Bytes(i2c_address, 0x74, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);          
          break;          
        case FUNC_SPEED_KD:
          I2C_Write_Bytes(i2c_address, 0x78, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);         
          break;          
        case FUNC_POSITION_KP:
          I2C_Write_Bytes(i2c_address, 0xA0, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);         
          break;
        case FUNC_POSITION_KI:
          I2C_Write_Bytes(i2c_address, 0xA4, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);         
          break;        
        case FUNC_POSITION_KD:
          I2C_Write_Bytes(i2c_address, 0xA8, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);            
          break;        
        case FUNC_DIAL_COUNTER:
          I2C_Write_Bytes(i2c_address, 0x3C, (uint8_t *)&func_data, 4, I2C_TIMOUT_MS);
          break;        
        case FUNC_RGB_MODE:
          I2C_Write_Bytes(i2c_address, 0x33, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS);
          break;        
        case FUNC_RGB_COLOR:
          I2C_Write_Bytes(i2c_address, 0x30, (uint8_t *)&func_data, 3, I2C_TIMOUT_MS);
          break;        
        case FUNC_RGB_BRIGHTNESS:
          I2C_Write_Bytes(i2c_address, 0x12, (uint8_t *)&func_data, 1, I2C_TIMOUT_MS);
          break;        
        
        default:
          break;
        }
        my_tx_option = feedback_option_data(can_id, data);
        FDCAN1_Send_Msg(2, my_tx_option, option, data);
        break;
      case 21:
        LL_I2C_Disable(I2C1);
        I2C1_Start();
        i2c_address = rx_data[0];
        i2c_address = ((i2c_address << 1) | 1);
        i2c_len = rx_data[1];
        if (i2c_len > 8)
          i2c_len = 8;
        i2c_success = I2C1_ReceiveData(i2c_address, data, i2c_len, 10);
        i2c_success = !i2c_success;
        my_tx_option = (option | (i2c_success << 8));
        FDCAN1_Send_Msg(cmd_id, my_tx_option, can_id, data);
        break;
      case 22:
        LL_I2C_Disable(I2C1);
        I2C1_Start();
        i2c_address = cmd_para;
        i2c_address = (i2c_address << 1);
        i2c_len = rx_data[0];
        if (i2c_len > 7)
          i2c_len = 7;
        is_stop_bit = !!(cmd_para & 0x80);
        if (is_stop_bit)
          i2c_success = I2C1_TransmitData(i2c_address, &rx_data[1], i2c_len, 10);
        else
          i2c_success = I2C1_TransmitData_RepeatedStart(i2c_address, &rx_data[1], i2c_len, 10);
        i2c_success = !i2c_success;
        my_tx_option = (option | (i2c_success << 8));
        FDCAN1_Send_Msg(cmd_id, my_tx_option, can_id, data);
        break;
      
      default:
        break;
      }
    }

    /* 在这里处理接收到的消息，可以根据 rx_header 中的信息来进行区分和处理 */
  }
}
/* USER CODE END 1 */
