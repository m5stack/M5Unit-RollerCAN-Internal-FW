/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef __I2C_PROTOCOL_H__
#define __I2C_PROTOCOL_H__

#include <stdint.h>

void Slave_Complete_Callback(uint8_t *rx_data, uint16_t len);

#endif
