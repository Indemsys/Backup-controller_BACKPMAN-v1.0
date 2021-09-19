#ifndef BCKUPC10_OUTPUTS_CONTROL_H
#define BCKUPC10_OUTPUTS_CONTROL_H

#define OUTP_LEDCAN0   0
#define OUTP_OUT1      1
#define OUTP_OUT2      2
#define OUTP_LED_GREEN 3
#define OUTP_LED_RED   4

#define OUTS_NUM       5

int32_t Outputs_init(void);

void    Set_output_blink(uint32_t out_num);
void    Set_output_on(uint32_t out_num);
void    Set_output_short_on(uint32_t out_num);
void    Set_output_off(uint32_t out_num);
void    Set_output_blink_undef(uint32_t out_num);
void    Set_output_blink_3(uint32_t out_num);
void    Set_output_can_active_blink(uint32_t out_num);

void    Outputs_state_automat(void);
void    Outputs_set_pattern(const int32_t *pttn, uint32_t n, uint32_t period);


#endif // OUTPUTS_CONTROL_H



