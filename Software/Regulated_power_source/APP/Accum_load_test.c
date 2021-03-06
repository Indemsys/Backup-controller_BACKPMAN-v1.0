// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-08-27
// 15:18:50 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Accumulator_load_test_stop(void)
{
  sys.charge_discharge_test_state = CH_DSCH_OFF;
  sys.disable_charging = 1;
  Set_INVSW_ON(0);
  Set_EN_DCDC(0);
  Set_AIDS_SHDN(0);
}
/*-----------------------------------------------------------------------------------------------------
  Процедура периодеческого разряда и заряда аккумулятора

  \param void
-----------------------------------------------------------------------------------------------------*/
void Accumulator_load_test_step(void)
{
  static  TIME_STRUCT   test_state_ref_time;
  float accum_v  = Get_float_val(&sys.accum_v);

  switch (sys.charge_discharge_test_state)
  {
  case CH_DSCH_OFF       :
    // Включаем разряд аккумулятора на внешний резистор через ключ инвертера
    sys.disable_charging = 1;
    Set_EN_DCDC(0);
    Set_AIDS_SHDN(0);
    Set_INVSW_ON(1);

    Set_output_blink(OUTP_LED_GREEN);
    Set_output_off(OUTP_LED_RED);

    Get_time(&test_state_ref_time);
    sys.charge_discharge_test_state = CH_DSCH_DISCHARGE;
    break;
  case CH_DSCH_DISCHARGE :
    // Ждем пока напряжение на аккумуляторе не опустится ниже 16 В
    if (accum_v < 17.5f)
    {
      Set_INVSW_ON(0);

      Set_output_on(OUTP_LED_GREEN);
      Set_output_off(OUTP_LED_RED);

      Get_time(&test_state_ref_time);
      sys.charge_discharge_test_state = CH_DSCH_IDLE1;
    }
    break;
  case CH_DSCH_IDLE1     :
    if (Time_elapsed_sec(&test_state_ref_time) > (60*5))
    {
      sys.disable_charging = 0;
      wvar.en_charger      = 1;

      Set_output_blink(OUTP_LED_GREEN);
      Set_output_blink(OUTP_LED_RED);

      Get_time(&test_state_ref_time);
      sys.charge_discharge_test_state = CH_DSCH_CHARGE_START;
    }
    break;
  case CH_DSCH_CHARGE_START:
    if (Time_elapsed_sec(&test_state_ref_time) > (60*1))
    {
      if (sys.accum_status == ACCUM_CHARGING)
      {
        Set_output_off(OUTP_LED_GREEN);
        Set_output_blink(OUTP_LED_RED);

        Get_time(&test_state_ref_time);
        sys.charge_discharge_test_state = CH_DSCH_CHARGE;
      }
      else
      {
        sys.disable_charging = 1;

        Set_output_on(OUTP_LED_GREEN);
        Set_output_off(OUTP_LED_RED);

        Get_time(&test_state_ref_time);
        sys.charge_discharge_test_state = CH_DSCH_IDLE1;
      }
    }
    break;
  case CH_DSCH_CHARGE    :
    if (sys.accum_status != ACCUM_CHARGING)
    {
      sys.disable_charging = 1;

      Set_output_on(OUTP_LED_GREEN);
      Set_output_on(OUTP_LED_RED);

      Get_time(&test_state_ref_time);
      sys.charge_discharge_test_state = CH_DSCH_IDLE2;
    }
    break;
  case CH_DSCH_IDLE2     :
    if (Time_elapsed_sec(&test_state_ref_time) > (60*5))
    {
      Get_time(&test_state_ref_time);
      sys.charge_discharge_test_state = CH_DSCH_OFF;
    }
    break;
  }
}


