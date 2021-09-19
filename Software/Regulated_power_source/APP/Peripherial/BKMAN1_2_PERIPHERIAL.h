#ifndef BCKUPC10_PERIPH_H
  #define BCKUPC10_PERIPH_H

#include "BKMAN1_2_clock_config.h"
#include "BKMAN1_2_PINS.h"
#include "BKMAN1_2_DMA.h"
#include "BKMAN1_2_PIT.h"
#include "BKMAN1_2_UART.h"
#include "BKMAN1_2_ADC.h"
#include "BKMAN1_2_Flash.h"
#include "BKMAN1_2_I2C.h"
#include "BKMAN1_2_SPI.h"
#include "BKMAN1_2_CAN.h"

extern void Delay_m7(int cnt); // Задержка на (cnt+1)*7 тактов . Передача нуля недопускается

#define  DELAY_1us    Delay_m7(16)           // 0.992     мкс при частоте 120 МГц
#define  DELAY_4us    Delay_m7(68)           // 4.025     мкс при частоте 120 МГц
#define  DELAY_8us    Delay_m7(136)          // 7.992     мкс при частоте 120 МГц
#define  DELAY_32us   Delay_m7(548)          // 32.025    мкс при частоте 120 МГц
#define  DELAY_ms(x)  Delay_m7(17143*x-1)    // 999.95*N мкс при частоте 120 МГц



#endif
