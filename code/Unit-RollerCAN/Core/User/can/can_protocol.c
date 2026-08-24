/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "can_protocol.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "fdcan.h"
#include "i2c.h"
#include "main.h"
#include "motordriver.h"
#include "myadc.h"
#include "mysys.h"
#include "rgb.h"
#include "smart_knob.h"
#include "u8g2_disp_fun.h"

#define FIRMWARE_VERSION                 (3)
#define ROLLER_DEVICE_ID                 (0x02)
#define EXTENDED_POSITION_CMD            (0x17)
#define EXTENDED_POSITION_READ_CMD       (0x18)
#define EXTENDED_POSITION_MAX_ANGLE_CDEG (35999U)
#define EXTENDED_POSITION_MAX_CURRENT_MA (1200U)

FDCAN_TxHeaderTypeDef fdcan1_TxHeader;

static uint8_t is_valid_i2c_7bit_address(uint16_t address)
{
    return address >= 0x08 && address <= 0x77;
}

static uint8_t is_valid_can_request(const FDCAN_RxHeaderTypeDef *header)
{
    return header->IdType == FDCAN_EXTENDED_ID && header->RxFrameType == FDCAN_DATA_FRAME &&
           header->DataLength == FDCAN_DLC_BYTES_8 && header->FDFormat == FDCAN_CLASSIC_CAN &&
           header->BitRateSwitch == FDCAN_BRS_OFF;
}

static int16_t saturate_float_to_i16(float value)
{
    if (!isfinite(value)) {
        return 0;
    }
    if (value > (float)INT16_MAX) {
        return INT16_MAX;
    }
    if (value < (float)INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)value;
}

static int32_t saturate_float_to_i32(float value)
{
    if (!isfinite(value)) {
        return 0;
    }
    if (value >= (float)INT32_MAX) {
        return INT32_MAX;
    }
    if (value <= (float)INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t)value;
}

static int16_t get_legacy_position_deg_i16(void)
{
    int32_t position_deg = get_legacy_position_cdeg() / 100;

    if (position_deg > INT16_MAX) {
        return INT16_MAX;
    }
    if (position_deg < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)position_deg;
}

static uint8_t get_request_host_id(uint16_t option)
{
    return (uint8_t)option;
}

static uint16_t make_command_response_option(uint8_t success, uint8_t host_id)
{
    return (uint16_t)(((uint16_t)success << 8) | host_id);
}

static void get_uid_words(uint32_t uid[3])
{
    uid[0] = LL_GetUID_Word0();
    uid[1] = LL_GetUID_Word1();
    uid[2] = LL_GetUID_Word2();
}

static uint8_t get_device_i2c_address(void)
{
    return i2c_address[0];
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
    } else if (motor_mode == MODE_POS_ERR_PROTECT) {
        motor_mode_temp = MODE_POS;
    }
    tx_status |= (motor_mode_temp << 3);
    tx_status |= (sys_status << 6);
    tx_option |= (tx_status << 8);

    int16_t rpm_temp = saturate_float_to_i16(motor_rpm);
    memcpy(&data[0], &rpm_temp, 2);
    int16_t position_temp = get_legacy_position_deg_i16();
    memcpy(&data[2], &position_temp, 2);
    int16_t current_temp = saturate_float_to_i16(ph_crrent_lpf);
    memcpy(&data[4], &current_temp, 2);
    int16_t vin_temp = saturate_float_to_i16(vol_lpf / 100.0f);
    memcpy(&data[6], &vin_temp, 2);

    return tx_option;
}

void feedback_function_read(uint8_t *data, uint16_t index, int32_t tx_data)
{
    memcpy(&data[0], &index, 2);
    memcpy(&data[4], &tx_data, 4);
}

uint8_t FDCAN1_Send_Msg(uint8_t cmd_id, uint16_t option, uint8_t can_id, uint8_t *msg)
{
    fdcan1_TxHeader.Identifier          = ((cmd_id << 24) | (option << 8) | can_id);
    fdcan1_TxHeader.IdType              = FDCAN_EXTENDED_ID;  // 标准ID
    fdcan1_TxHeader.TxFrameType         = FDCAN_DATA_FRAME;   // 数据
    fdcan1_TxHeader.DataLength          = FDCAN_DLC_BYTES_8;  // 数据长度
    fdcan1_TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    fdcan1_TxHeader.BitRateSwitch       = FDCAN_BRS_OFF;       // 关闭速率切换
    fdcan1_TxHeader.FDFormat            = FDCAN_CLASSIC_CAN;   // 传统的CAN模式
    fdcan1_TxHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;  // 无发送事
    fdcan1_TxHeader.MessageMarker       = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &fdcan1_TxHeader, msg) != HAL_OK) {
        return 1;
    }

    return 0;
}

/* FDCAN 接收中断回调函数 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if (hfdcan != &hfdcan1) {
        return;
    }

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
        FDCAN_RxHeaderTypeDef rx_header;
        uint8_t rx_data[8] = {0};
        uint16_t my_tx_option;
        uint8_t motor_mode_temp;
        int32_t int_value_temp;
        uint8_t i2c_len     = 0;
        uint8_t i2c_success = 0;
        uint8_t is_stop_bit = 0;

        comm_flash_count = 4;

        /* 从 FIFO0 读取消息 */
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
            return;
        }

        if (!is_valid_can_request(&rx_header)) {
            return;
        }

        {
            uint8_t cmd_id       = identifier_to_cmd(rx_header.Identifier);
            uint8_t cmd_para     = identifier_to_cmd_para(rx_header.Identifier);
            uint8_t option       = identifier_to_option(rx_header.Identifier);
            uint8_t data[8]      = {0};
            uint16_t func_index  = 0;
            uint16_t i2c_address = 0;
            uint32_t i2c_data    = 0;
            int32_t func_data    = 0;

            switch (cmd_id) {
                case 0:
                    FDCAN1_Send_Msg(cmd_id, can_id, 0xFE, data);
                    break;
                case 2:
                    my_tx_option = feedback_option_data(can_id, data);
                    FDCAN1_Send_Msg(cmd_id, my_tx_option, get_request_host_id(option), data);
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
                    } else {
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
                    } else {
                        motor_disable_flag = 1;
                        MotorDriverSetMode(MDRV_MODE_OFF);
                    }
                    my_tx_option = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 7:
                    can_id          = cmd_para;
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
                    pos_err_recover_try_counter   = 0;
                    if (motor_mode == MODE_SPEED_ERR_PROTECT) {
                        sys_status = SYS_STANDBY;
                        motor_mode = MODE_SPEED;
                    } else if (motor_mode == MODE_POS_ERR_PROTECT) {
                        sys_status = SYS_STANDBY;
                        motor_mode = MODE_POS;
                    }
                    error_code &= ~ERR_STALLED;
                    speed_err_count_flag = 0;
                    speed_err_auto_flag  = 0;
                    pos_err_count_flag   = 0;
                    pos_err_auto_flag    = 0;
                    err_stalled_flag     = 0;
                    my_tx_option         = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 10:
                    flash_data_write_back_flag = 1;
                    my_tx_option               = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 11:
                    if (cmd_para <= 2) {
                        change_baudrate_flag  = 1;
                        change_baudrate_delay = HAL_GetTick();
                        bps_index             = cmd_para;
                    }
                    my_tx_option = (option | (bps_index << 8));
                    FDCAN1_Send_Msg(cmd_id, my_tx_option, can_id, data);
                    break;
                case 12:
                    motor_stall_protection_flag = 1;
                    my_tx_option                = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 13:
                    motor_stall_protection_flag = 0;
                    my_tx_option                = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 14:
                    motor_overvalue_protection_flag = 1;
                    my_tx_option                    = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 15:
                    motor_overvalue_protection_flag = 0;
                    my_tx_option                    = feedback_option_data(can_id, data);

                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 17:
                    memcpy(&func_index, &rx_data[0], 2);
                    switch (func_index) {
                        case FUNC_ON_OFF:
                            motor_output ? (motor_output = 1) : (motor_output = 0);
                            feedback_function_read(data, func_index, motor_output);
                            break;
                        case FUNC_RUN_MODE:
                            motor_mode_temp = motor_mode;
                            if (motor_mode == MODE_SPEED_ERR_PROTECT) {
                                motor_mode_temp = MODE_SPEED;
                            } else if (motor_mode == MODE_POS_ERR_PROTECT) {
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
                            switch (speed_pid_index) {
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
                            switch (speed_pid_index) {
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
                            switch (speed_pid_index) {
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
                            switch (pos_pid_index) {
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
                            switch (pos_pid_index) {
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
                            switch (pos_pid_index) {
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
                            int_value_temp = saturate_float_to_i32(motor_rpm * 100.0f);
                            feedback_function_read(data, func_index, int_value_temp);
                            break;
                        case FUNC_READBACK_POSITION:
                            int_value_temp = get_legacy_position_cdeg();
                            feedback_function_read(data, func_index, int_value_temp);
                            break;
                        case FUNC_READBACK_CURRENT:
                            int_value_temp = saturate_float_to_i32(ph_crrent_lpf * 100.0f);
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
                        case FUNC_FIRMWARE_VERSION:
                            feedback_function_read(data, func_index, FIRMWARE_VERSION);
                            break;
                        case FUNC_PRODUCT_DEVICE_ID:
                            feedback_function_read(data, func_index, ROLLER_DEVICE_ID);
                            break;
                        case FUNC_UID_WORD_0:
                        case FUNC_UID_WORD_1:
                        case FUNC_UID_WORD_2: {
                            uint32_t uid[3];
                            get_uid_words(uid);
                            feedback_function_read(data, func_index, (int32_t)uid[func_index - FUNC_UID_WORD_0]);
                            break;
                        }
                        case FUNC_CAN_ID:
                            feedback_function_read(data, func_index, can_id);
                            break;
                        case FUNC_CAN_BAUD_INDEX:
                            feedback_function_read(data, func_index, bps_index);
                            break;
                        case FUNC_COMMUNICATION_MODE:
                            feedback_function_read(data, func_index, comm_type);
                            break;
                        case FUNC_I2C_ADDRESS:
                            feedback_function_read(data, func_index, get_device_i2c_address());
                            break;
                        case FUNC_SYSTEM_STATUS:
                            feedback_function_read(data, func_index, sys_status);
                            break;
                        case FUNC_ERROR_CODE:
                            feedback_function_read(data, func_index, error_code);
                            break;
                        case FUNC_STALL_PROTECTION:
                            feedback_function_read(data, func_index, motor_stall_protection_flag);
                            break;
                        case FUNC_POSITION_RANGE_PROTECTION:
                            feedback_function_read(data, func_index, motor_overvalue_protection_flag);
                            break;
                        case FUNC_BUTTON_MODE_SWITCH:
                            feedback_function_read(data, func_index, mode_switch_flag);
                            break;
                        case FUNC_ENCODER_CALIBRATION_BUSY:
                            feedback_function_read(data, func_index, IsMotorDriverEncCalBusy());
                            break;

                        default:
                            break;
                    }
                    FDCAN1_Send_Msg(cmd_id, option, can_id, data);
                    break;
                case 18:
                    memcpy(&func_index, &rx_data[0], 2);
                    memcpy(&func_data, &rx_data[4], 4);
                    switch (func_index) {
                        case FUNC_SAVE_FLASH:
                            if (func_data != 0) {
                                flash_data_write_back_flag = 1;
                            }

                            break;
                        case FUNC_REMOVE_PROTECTION:
                            if (func_data != 0) {
                                speed_err_recover_try_counter = 0;
                                pos_err_recover_try_counter   = 0;
                                if (motor_mode == MODE_SPEED_ERR_PROTECT) {
                                    sys_status = SYS_STANDBY;
                                    motor_mode = MODE_SPEED;
                                } else if (motor_mode == MODE_POS_ERR_PROTECT) {
                                    sys_status = SYS_STANDBY;
                                    motor_mode = MODE_POS;
                                }
                                error_code &= ~ERR_STALLED;
                                speed_err_count_flag = 0;
                                speed_err_auto_flag  = 0;
                                pos_err_count_flag   = 0;
                                pos_err_auto_flag    = 0;
                                err_stalled_flag     = 0;
                            }

                            break;
                        case FUNC_ON_OFF:
                            motor_output = (func_data != 0);
                            if (motor_output) {
                                if (!over_vol_flag && !err_stalled_flag && motor_mode < MODE_MAX) {
                                    if (motor_mode == MODE_DIAL && motor_disable_flag) {
                                        init_smart_knob();
                                    }
                                    motor_disable_flag = 0;
                                    MotorDriverSetMode(MDRV_MODE_RUN);
                                }
                            } else {
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
                                        pid_ctrl_pos_t.iTerm   = 0;
                                    } else {
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
                            set_pos_point_legacy_cdeg(pos_point);
                            break;
                        case FUNC_POSITION_MAX_CURRENT:
                            max_pos_current = apply_pid_current_limit(&pid_ctrl_pos_t, func_data);
                            break;
                        case FUNC_SPEED_MAX_CURRENT:
                            max_speed_current = apply_pid_current_limit(&pid_ctrl_speed_t, func_data);
                            break;
                        case FUNC_SPEED_KP:
                            speed_pid_int[0]   = func_data;
                            speed_pid_float[0] = (float)speed_pid_int[0] / 100000;
                            if (speed_pid_index == 0) {
                                PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1],
                                              speed_pid_float[2]);
                                pid_ctrl_speed_t.iTerm = 0;
                            }
                            break;
                        case FUNC_SPEED_KI:
                            speed_pid_int[1]   = func_data;
                            speed_pid_float[1] = (float)speed_pid_int[1] / 10000000;
                            if (speed_pid_index == 0) {
                                PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1],
                                              speed_pid_float[2]);
                                pid_ctrl_speed_t.iTerm = 0;
                            }
                            break;
                        case FUNC_SPEED_KD:
                            speed_pid_int[2]   = func_data;
                            speed_pid_float[2] = (float)speed_pid_int[2] / 100000;
                            if (speed_pid_index == 0) {
                                PIDTuningsSet(&pid_ctrl_speed_t, speed_pid_float[0], speed_pid_float[1],
                                              speed_pid_float[2]);
                                pid_ctrl_speed_t.iTerm = 0;
                            }
                            break;
                        case FUNC_POSITION_KP:
                            pos_pid_int[0]   = func_data;
                            pos_pid_float[0] = (float)pos_pid_int[0] / 100000;
                            if (pos_pid_index == 0) {
                                PIDTuningsSet(&pid_ctrl_pos_t, pos_pid_float[0], pos_pid_float[1], pos_pid_float[2]);
                                pid_ctrl_pos_t.iTerm = 0;
                            }
                            break;
                        case FUNC_POSITION_KI:
                            pos_pid_int[1]   = func_data;
                            pos_pid_float[1] = (float)pos_pid_int[1] / 10000000;
                            if (pos_pid_index == 0) {
                                PIDTuningsSet(&pid_ctrl_pos_t, pos_pid_float[0], pos_pid_float[1], pos_pid_float[2]);
                                pid_ctrl_pos_t.iTerm = 0;
                            }
                            break;
                        case FUNC_POSITION_KD:
                            pos_pid_int[2]   = func_data;
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
                            if (func_data == 0 || func_data == 1) {
                                rgb_show_mode = (uint8_t)func_data;
                            }
                            break;
                        case FUNC_RGB_COLOR:
                            memcpy(&rgb_color_buffer[rgb_color_buffer_index], &func_data, 3);
                            if (rgb_color_buffer[rgb_color_buffer_index]) {
                                lastest_rgb_color = rgb_color_buffer[rgb_color_buffer_index];
                                if (rgb_color_buffer_index < (RGB_BUFFER_SIZE - 1)) ++rgb_color_buffer_index;
                            }
                            break;
                        case FUNC_RGB_BRIGHTNESS:
                            if (func_data <= 100 && func_data >= 0) {
                                memcpy(&brightness_index, &func_data, 1);
                                ws2812_show();
                            }
                            break;
                        case FUNC_STALL_PROTECTION:
                            if (func_data == 0 || func_data == 1) {
                                motor_stall_protection_flag = (uint8_t)func_data;
                            }
                            break;
                        case FUNC_POSITION_RANGE_PROTECTION:
                            if (func_data == 0 || func_data == 1) {
                                motor_overvalue_protection_flag = (uint8_t)func_data;
                            }
                            break;
                        case FUNC_BUTTON_MODE_SWITCH:
                            if (func_data == 0 || func_data == 1) {
                                mode_switch_flag = (uint8_t)func_data;
                            }
                            break;
                        case FUNC_START_ENCODER_CALIBRATION:
                            if (func_data == 1 && !over_vol_flag && !IsMotorDriverEncCalBusy()) {
                                motor_output       = 0;
                                motor_disable_flag = 1;
                                MotorDriverSetMode(MDRV_MODE_ENC_CAL);
                            }
                            break;
                        case FUNC_SAVE_ENCODER_CALIBRATION:
                            if (func_data == 1 && !IsMotorDriverEncCalBusy() && IsMotorDriverEncCalValid()) {
                                angle_cal_offset = GetMotorDriverEncCalOffset();
                                MotorDriverSetAngleOffset(angle_cal_offset);
                                flash_data_write_back_flag = 1;
                            }
                            break;

                        default:
                            break;
                    }
                    my_tx_option = feedback_option_data(can_id, data);
                    FDCAN1_Send_Msg(2, my_tx_option, option, data);
                    break;
                case 19:
                    memcpy(&func_index, &rx_data[0], 2);
                    memcpy(&i2c_address, &rx_data[2], 2);

                    if (!is_valid_i2c_7bit_address(i2c_address)) {
                        FDCAN1_Send_Msg(cmd_id, option, can_id, data);
                        break;
                    }

                    LL_I2C_Disable(I2C1);
                    I2C1_Start();
                    i2c_address = ((i2c_address << 1) | 1);
                    switch (func_index) {
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
                    memcpy(&func_index, &rx_data[0], 2);
                    memcpy(&func_data, &rx_data[4], 4);
                    memcpy(&i2c_address, &rx_data[2], 2);

                    if (!is_valid_i2c_7bit_address(i2c_address)) {
                        my_tx_option = feedback_option_data(can_id, data);
                        FDCAN1_Send_Msg(2, my_tx_option, option, data);
                        break;
                    }

                    LL_I2C_Disable(I2C1);
                    I2C1_Start();
                    i2c_address = (i2c_address << 1);

                    switch (func_index) {
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
                    i2c_address = rx_data[0];
                    i2c_len     = rx_data[1];

                    if (is_valid_i2c_7bit_address(i2c_address) && i2c_len >= 1 && i2c_len <= sizeof(data)) {
                        LL_I2C_Disable(I2C1);
                        I2C1_Start();
                        i2c_address = ((i2c_address << 1) | 1);
                        i2c_success = (I2C1_ReceiveData(i2c_address, data, i2c_len, 10) == 0);
                    }
                    my_tx_option = (option | (i2c_success << 8));
                    FDCAN1_Send_Msg(cmd_id, my_tx_option, can_id, data);
                    break;
                case 22:
                    i2c_address = cmd_para & 0x7F;
                    i2c_len     = rx_data[0];
                    is_stop_bit = !!(cmd_para & 0x80);

                    if (is_valid_i2c_7bit_address(i2c_address) && i2c_len >= 1 && i2c_len <= sizeof(rx_data) - 1) {
                        LL_I2C_Disable(I2C1);
                        I2C1_Start();
                        i2c_address <<= 1;
                        if (is_stop_bit) {
                            i2c_success = (I2C1_TransmitData(i2c_address, &rx_data[1], i2c_len, 10) == 0);
                        } else {
                            i2c_success = (I2C1_TransmitData_RepeatedStart(i2c_address, &rx_data[1], i2c_len, 10) == 0);
                        }
                    }
                    my_tx_option = (option | (i2c_success << 8));
                    FDCAN1_Send_Msg(cmd_id, my_tx_option, can_id, data);
                    break;
                case EXTENDED_POSITION_CMD: {
                    int32_t target_turns;
                    uint16_t target_angle_cdeg;
                    uint16_t max_current_ma;
                    uint8_t success = 0;

                    memcpy(&target_turns, &rx_data[0], sizeof(target_turns));
                    memcpy(&target_angle_cdeg, &rx_data[4], sizeof(target_angle_cdeg));
                    memcpy(&max_current_ma, &rx_data[6], sizeof(max_current_ma));

                    if (target_angle_cdeg <= EXTENDED_POSITION_MAX_ANGLE_CDEG &&
                        max_current_ma <= EXTENDED_POSITION_MAX_CURRENT_MA && !IsMotorDriverEncCalBusy()) {
                        uint32_t primask = __get_PRIMASK();
                        __disable_irq();
                        set_pos_point_turns_angle(target_turns, (float)target_angle_cdeg / 100.0f);
                        max_pos_current = apply_pid_current_limit(&pid_ctrl_pos_t, (int32_t)max_current_ma * 100);
                        if (!primask) {
                            __enable_irq();
                        }
                        memcpy(data, rx_data, sizeof(data));
                        success = 1;
                    }
                    FDCAN1_Send_Msg(cmd_id, make_command_response_option(success, get_request_host_id(option)), can_id,
                                    data);
                    break;
                }
                case EXTENDED_POSITION_READ_CMD: {
                    uint8_t success = 0;

                    if (cmd_para == 0) {
                        int32_t target_turns;
                        uint16_t target_angle_cdeg;
                        uint16_t max_current_ma;

                        get_target_position_snapshot(&target_turns, &target_angle_cdeg);
                        max_current_ma = (uint16_t)(normalize_current_limit(max_pos_current) / 100);
                        memcpy(&data[0], &target_turns, sizeof(target_turns));
                        memcpy(&data[4], &target_angle_cdeg, sizeof(target_angle_cdeg));
                        memcpy(&data[6], &max_current_ma, sizeof(max_current_ma));
                        success = 1;
                    } else if (cmd_para == 1) {
                        int32_t actual_turns;
                        uint16_t actual_angle_cdeg;
                        int16_t actual_current_ma = saturate_float_to_i16(ph_crrent_lpf);

                        get_mechanical_position_snapshot(&actual_turns, &actual_angle_cdeg);
                        memcpy(&data[0], &actual_turns, sizeof(actual_turns));
                        memcpy(&data[4], &actual_angle_cdeg, sizeof(actual_angle_cdeg));
                        memcpy(&data[6], &actual_current_ma, sizeof(actual_current_ma));
                        success = 1;
                    }
                    FDCAN1_Send_Msg(cmd_id, make_command_response_option(success, get_request_host_id(option)), can_id,
                                    data);
                    break;
                }

                default:
                    break;
            }
        }

        /* 在这里处理接收到的消息，可以根据 rx_header 中的信息来进行区分和处理 */
    }
}
