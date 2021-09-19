#ifndef BACKPMAN10_APP_TIME_UTILS_H
  #define BACKPMAN10_APP_TIME_UTILS_H

  #define  SEC_TO_TICKS(x)  (x * TX_TIMER_TICKS_PER_SECOND)
  #define  MS_TO_TICKS(x)  (((x * TX_TIMER_TICKS_PER_SECOND) / 1000U) + 1U)
  #define  TICKS_TO_SEC(x) ((float)x/(float)TX_TIMER_TICKS_PER_SECOND)

typedef struct
{
    uint32_t usec;
    uint32_t sec;

} TIME_STRUCT;

uint32_t time_delay(uint32_t ms);
uint32_t Wait_ms(uint32_t ms);
void     Get_time(TIME_STRUCT *p_time);
uint32_t Time_elapsed_sec(TIME_STRUCT *p_time);

#endif // APP_TIME_UTILS_H



