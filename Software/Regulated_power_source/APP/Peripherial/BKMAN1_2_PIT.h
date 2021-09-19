#ifndef BCKUPC01_PIT_H
  #define BCKUPC01_PIT_H

  #define PIT_CHANNELS_NUM  4


typedef void (*T_PIT_callback)(void);

void     PIT_init_module(void);

void     PIT_init_channel(uint8_t chnl, uint32_t period, uint8_t prio, uint8_t en_int,  T_PIT_callback callback_func);
void     PIT_disable(uint8_t chnl);

uint32_t PIT_get_curr_val(uint8_t chnl);
uint32_t PIT_get_load_val(uint8_t chnl);

#endif
