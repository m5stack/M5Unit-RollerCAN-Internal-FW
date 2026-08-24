/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __CAN_PROTOCOL_H__
#define __CAN_PROTOCOL_H__

#include <stdint.h>

enum can_func_t {
    FUNC_SAVE_FLASH                = 0x7002,
    FUNC_REMOVE_PROTECTION         = 0x7003,
    FUNC_ON_OFF                    = 0x7004,
    FUNC_RUN_MODE                  = 0x7005,
    FUNC_CURRENT_SETTING           = 0x7006,
    FUNC_SPEED_SETTING             = 0x700A,
    FUNC_POSITION_SETTING          = 0x7016,
    FUNC_POSITION_MAX_CURRENT      = 0x7017,
    FUNC_SPEED_MAX_CURRENT         = 0x7018,
    FUNC_SPEED_KP                  = 0x7020,
    FUNC_SPEED_KI                  = 0x7021,
    FUNC_SPEED_KD                  = 0x7022,
    FUNC_POSITION_KP               = 0x7023,
    FUNC_POSITION_KI               = 0x7024,
    FUNC_POSITION_KD               = 0x7025,
    FUNC_READBACK_SPEED            = 0x7030,
    FUNC_READBACK_POSITION         = 0x7031,
    FUNC_READBACK_CURRENT          = 0x7032,
    FUNC_DIAL_COUNTER              = 0x7033,
    FUNC_VIN                       = 0x7034,
    FUNC_TEMP                      = 0x7035,
    FUNC_RGB_MODE                  = 0x7050,
    FUNC_RGB_COLOR                 = 0x7051,
    FUNC_RGB_BRIGHTNESS            = 0x7052,
    FUNC_FIRMWARE_VERSION          = 0x7060,
    FUNC_PRODUCT_DEVICE_ID         = 0x7061,
    FUNC_UID_WORD_0                = 0x7062,
    FUNC_UID_WORD_1                = 0x7063,
    FUNC_UID_WORD_2                = 0x7064,
    FUNC_CAN_ID                    = 0x7065,
    FUNC_CAN_BAUD_INDEX            = 0x7066,
    FUNC_COMMUNICATION_MODE        = 0x7067,
    FUNC_I2C_ADDRESS               = 0x7068,
    FUNC_SYSTEM_STATUS             = 0x7069,
    FUNC_ERROR_CODE                = 0x706A,
    FUNC_STALL_PROTECTION          = 0x706B,
    FUNC_POSITION_RANGE_PROTECTION = 0x706C,
    FUNC_BUTTON_MODE_SWITCH        = 0x706D,
    FUNC_START_ENCODER_CALIBRATION = 0x7070,
    FUNC_SAVE_ENCODER_CALIBRATION  = 0x7071,
    FUNC_ENCODER_CALIBRATION_BUSY  = 0x7072
};

uint8_t FDCAN1_Send_Msg(uint8_t cmd_id, uint16_t option, uint8_t can_id, uint8_t* msg);

#endif
