#ifndef __MYADC_H
#define __MYADC_H
#include "stm32g4xx.h"
#include "arm_const_structs.h"

extern int32_t internal_temp;
extern int32_t internal_temp_raw;

void MyADCInit(void);
void MyAdcProcess(void);
void MyADCZeroCal(void);

uint16_t    MyAdcGetVal(uint8_t adc_hw,uint8_t channel);
int32_t     MyAdcGetCurrent(uint8_t channel);

#endif
