#ifndef BCKUPC10_I2C_H
  #define BCKUPC10_I2C_H

typedef  enum   {I2C_7BIT_ADDR, I2C_10BIT_ADDR} T_I2C_addr_mode;

extern uint32_t    DCDC_DAC_comm_errors;


void     BCKUPC10_I2C1_Init(void);
void     BCKUPC10_I2C1_DeInit(void);
void     BCKUPC10_I2C0_Init(void);

uint32_t I2C1_WriteRead(T_I2C_addr_mode addr_mode, uint16_t addr, uint32_t wrlen, uint8_t *outbuf, uint32_t rdlen, uint8_t *inbuf, uint32_t timeout);


#endif // BCKUPC10_I2C_H



