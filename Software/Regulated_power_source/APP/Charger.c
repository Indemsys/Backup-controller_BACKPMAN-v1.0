#include   "App.h"



uint16_t                       g_charger_dac_code;

static uint8_t                 full_charge_detector;
static uint8_t                 disconnect_detector;

static  TIME_STRUCT            state_ref_time;

static  TX_EVENT_FLAGS_GROUP   charger_flags;

#pragma data_alignment=8
uint8_t                        thread_charger_stack[THREAD_CHARGER_STACK_SIZE];
static  void                   Charger_task(ULONG initial_input);
TX_THREAD                      charger_task;

uint8_t                        g_block_check_reverse;

// Граничные значения в наблюдателе за аварийными состояниями
volatile int32_t min_pwr_src_voltage_code      ;
volatile int32_t max_pwr_src_voltage_code      ;
volatile int32_t min_accum_voltage_code        ;
volatile int32_t max_accum_voltage_code        ;
volatile int32_t max_pulse_charge_current_code ;
volatile int32_t max_chrg_reverse_current_code ;
volatile int32_t max_pwrsrc_charge_current_code;

volatile float   end_charge_current            ;  // Значение тока при уменьшении до которого происходит переход из режима зарядки в режим IDLE


static void Goto_discharging_state(void);
static void Goto_charging_state(void);
static void Goto_IDLE_state(void);
static void Goto_off_state(void);

typedef struct
{
    int32_t      int_v;
    int32_t      int_dif;
    int32_t      exflt_state;
    int32_t      prev_int_v;
    int32_t      cnt;
    TIME_STRUCT  ref_time;
} T_fc_detector;

T_fc_detector fcd;


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t  Create_charger_task(void)
{
  UINT res;
  res = tx_thread_create(&charger_task, "Charger", Charger_task,
       0,
       (void *)thread_charger_stack, // stack_start
       THREAD_CHARGER_STACK_SIZE,    // stack_size
       THREAD_CHARGER_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
       THREAD_CHARGER_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
       TX_NO_TIME_SLICE,
       TX_AUTO_START);
  if (res != TX_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Функция наблюдения за аварийными состояниями

  Вызывается из функции прерывания !!!
  Вызывается с периодом ADC_PERIOD
  Время выполнения критично.


  id_accum_current  - Знаковое целое. Ток аккумулятора через идеальный диод в систему (с плюсом) и из системы (с минусом)
  sys_accum_current - Знаковое целое. Ток от аккумулятора в систему (с плюсом) и из системы (зарядка, с минусом). Не считая тока в инвертер
  pwr_src_current   - Знаковое целое. Ток источника питания в систему (с плюсом) и из системы (с минусом)
  invert_current    - Знаковое целое. Ток в инвертер от аккумулятора (с плюсом) и обратно (с минусом)


  \param void
-----------------------------------------------------------------------------------------------------*/
void Emergency_observer(void)
{
  if (sys.charger_task_active != 0)
  {
    if (Get_EN_DCDC() != 0)
    {
      uint32_t  fl = 0;
      int32_t   v;

      if  (Get_PIDS_FAULT() == 0)                                fl |= E_POWER_SOURCE_SWITCH_FAULT;  // Перенапряжение на источнике питания
      if  (Get_AIDS_FAULT() == 0)                                fl |= E_ACCUMULATOR_SWITCH_FAULT;   // Перенапряжение на аккумуляторе
      if  (Get_INVSW_ON() != 0)                                  fl |= E_INVERTER_SWITCHED_ON;       // Открыт ключ подачи питания на инвертер

      v = s_ACCUM_I;
      if  (-v > max_pulse_charge_current_code)                   fl |= E_ACCUMULATOR_CHARGE_CURRENT_TOO_HIGH;
      if (g_block_check_reverse == 0)
      {
        if  (v  > max_chrg_reverse_current_code)                 fl |= E_ACCUMULATOR_DISCHARGE_CURRENT_TOO_HIGH;
      }

      v = s_ACCUM_V;
      if  (v <= min_accum_voltage_code)                          fl |= E_ACCUMULATOR_VOLTAGE_TOO_LOW;
      if  (v >= max_accum_voltage_code)                          fl |= E_ACCUMULATOR_VOLTAGE_TOO_HIGH;

      v = s_PSRC_V;
      if  (v <= min_pwr_src_voltage_code)                        fl |= E_POWER_SOURCE_VOLTAGE_TOO_LOW;
      if  (v >= max_pwr_src_voltage_code)                        fl |= E_POWER_SOURCE_VOLTAGE_TOO_HIGH;

      if (s_PSRC_I > max_pwrsrc_charge_current_code)             fl |= E_POWER_SOURCE_CURRENT_TOO_HIGH;
      if  (sys.disable_charging != 0)                            fl |= E_DISABLED_CHARGING;
      if  (sys.dcdc_dac_error_flag != 0)                         fl |= E_DCDC_DAC_ERROR;


      if (fl != 0)
      {
        sys.last_emergency_signals = fl;
        tx_event_flags_set(&charger_flags, DCDC_OFF_EVENT, TX_OR);
      }
    }
  }
}



/*-----------------------------------------------------------------------------------------------------
  Возвращает не 0 если зарадка не допустима и должна быть прекращена

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Is_charging_disallowed(void)
{
  uint32_t fl = 0;
  float accum_i    = Get_float_val(&sys.accum_i);
  float accum_v    = Get_float_val(&sys.accum_v);
  float psrc_v     = Get_float_val(&sys.psrc_v);
  float invert_i   = Get_float_val(&sys.invert_i);

  if (Get_PIDS_FAULT() == 0)                                           fl |= W_POWER_SOURCE_SWITCH_FAULT;              // Сработал сигнал PIDS_FAULT, ошибка ключа внешнего источника питания - перенапряжение
  if (Get_AIDS_FAULT() == 0)                                           fl |= W_ACCUMULATOR_SWITCH_FAULT;               // Сработал сигнал AIDS_FAULT, ошибка идеального диода от аккумулятора в систему - перенапряжение
  if (-accum_i > (wvar.max_charge_current+MAX_CHARGE_CURRENT_EXCESS))  fl |= W_ACCUMULATOR_CHARGE_CURRENT_TOO_HIGH;    // Слишком большой ток заряда аккумулятора
  if (accum_i > BEFORE_CHARGING_MAX_REVERSE_CURRENT)                   fl |= W_ACCUMULATOR_DISCHARGE_CURRENT_TOO_HIGH; // Слишком большой ток разряда аккумулятора
  if (accum_v <= MIN_ACCUM_VOLTAGE)                                    fl |= W_ACCUMULATOR_VOLTAGE_TOO_LOW;            // Напряжение аккумулятора слишком низкое
  if (accum_v >= (wvar.max_charge_voltage + MAX_ACCUM_VOLTAGE_EXCESS)) fl |= W_ACCUMULATOR_VOLTAGE_TOO_HIGH;           // Напряжение аккумулятора слишком высокое
  if (psrc_v  <= MIN_PWR_SRC_VOLTAGE)                                  fl |= W_PWR_SRC_VOLTAGE_TOO_LOW;                // Напряжение источника слишком низкое
  if (psrc_v  >= MAX_PWR_SRC_VOLTAGE)                                  fl |= W_PWR_SRC_VOLTAGE_TOO_HIGH;               // Напряжение источника слишком высокое
  if (Get_INVSW_ON() != 0)                                             fl |= W_INVERTER_SWITCHED_ON;                   // Открыт ключ подачи питания на инвертер
  if (fabs(invert_i) >= MAX_INVERTER_LEAKAGE_CURRENT)                  fl |= W_INVERTER_CURRENT_PRESENT;               // Обнаружен ток через ключ инвертера
  if (sys.disable_charging != 0)                                       fl |= W_DISABLED_CHARGING;
  if (wvar.en_charger == 0)                                            fl |= W_DISABLED_CHARGING;
  if (sys.dcdc_dac_error_flag != 0)                                    fl |= W_DCDC_DAC_ERROR;
  if (sys.cpu_overheat)                                                fl |= W_CPU_TEMP_TOO_HIGH;
  if (sys.pwr_src_condition == 0)                                      fl |= W_PWR_SRC_DONT_READY;

  sys.last_disabling_signals = fl;
  return fl;
}





/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
static void Log_Emergency_signals(uint32_t v)
{
  if (v & E_POWER_SOURCE_SWITCH_FAULT)              LOGs(0, 0, SEVERITY_DEFAULT,"E: POWER_SOURCE_SWITCH_FAULT");
  if (v & E_ACCUMULATOR_SWITCH_FAULT)               LOGs(0, 0, SEVERITY_DEFAULT,"E: ACCUMULATOR_SWITCH_FAULT              ");
  if (v & E_DCDC_MODULE_FAULT)                      LOGs(0, 0, SEVERITY_DEFAULT,"E: DCDC_MODULE_FAULT                     ");
  if (v & E_INVERTER_SWITCHED_ON)                   LOGs(0, 0, SEVERITY_DEFAULT,"E: INVERTER_SWITCHED_ON                  ");
  if (v & E_ACCUMULATOR_CHARGE_CURRENT_TOO_HIGH)    LOGs(0, 0, SEVERITY_DEFAULT,"E: ACCUMULATOR_CHARGE_CURRENT_TOO_HIGH   ");
  if (v & E_ACCUMULATOR_DISCHARGE_CURRENT_TOO_HIGH) LOGs(0, 0, SEVERITY_DEFAULT,"E: ACCUMULATOR_DISCHARGE_CURRENT_TOO_HIGH");
  if (v & E_ACCUMULATOR_VOLTAGE_TOO_LOW)            LOGs(0, 0, SEVERITY_DEFAULT,"E: ACCUMULATOR_VOLTAGE_TOO_LOW           ");
  if (v & E_ACCUMULATOR_VOLTAGE_TOO_HIGH)           LOGs(0, 0, SEVERITY_DEFAULT,"E: ACCUMULATOR_VOLTAGE_TOO_HIGH          ");
  if (v & E_POWER_SOURCE_VOLTAGE_TOO_LOW)           LOGs(0, 0, SEVERITY_DEFAULT,"E: POWER_SOURCE_VOLTAGE_TOO_LOW          ");
  if (v & E_POWER_SOURCE_VOLTAGE_TOO_HIGH)          LOGs(0, 0, SEVERITY_DEFAULT,"E: POWER_SOURCE_VOLTAGE_TOO_HIGH         ");
  if (v & E_DISABLED_CHARGING)                      LOGs(0, 0, SEVERITY_DEFAULT,"E: DISABLED_CHARGING                     ");
  if (v & E_DCDC_DAC_ERROR)                         LOGs(0, 0, SEVERITY_DEFAULT,"E: DCDC_DAC_ERROR                        ");
  if (v & E_POWER_SOURCE_CURRENT_TOO_HIGH )         LOGs(0, 0, SEVERITY_DEFAULT,"E: POWER_SOURCE_CURRENT_TOO_HIGH         ");
}

/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
static void Log_Disable_signals(uint32_t v)
{
  if (v & W_POWER_SOURCE_SWITCH_FAULT)               LOGs(0, 0, SEVERITY_DEFAULT,"W: POWER_SOURCE_SWITCH_FAULT             ");
  if (v & W_ACCUMULATOR_SWITCH_FAULT)                LOGs(0, 0, SEVERITY_DEFAULT,"W: ACCUMULATOR_SWITCH_FAULT              ");
  if (v & W_INVERTER_SWITCH_FAULT)                   LOGs(0, 0, SEVERITY_DEFAULT,"W: INVERTER_SWITCH_FAULT                 ");
  if (v & W_ACCUMULATOR_CHARGE_CURRENT_TOO_HIGH)     LOGs(0, 0, SEVERITY_DEFAULT,"W: ACCUMULATOR_CHARGE_CURRENT_TOO_HIGH   ");
  if (v & W_ACCUMULATOR_DISCHARGE_CURRENT_TOO_HIGH)  LOGs(0, 0, SEVERITY_DEFAULT,"W: ACCUMULATOR_DISCHARGE_CURRENT_TOO_HIGH");
  if (v & W_ACCUMULATOR_VOLTAGE_TOO_LOW)             LOGs(0, 0, SEVERITY_DEFAULT,"W: ACCUMULATOR_VOLTAGE_TOO_LOW           ");
  if (v & W_ACCUMULATOR_VOLTAGE_TOO_HIGH)            LOGs(0, 0, SEVERITY_DEFAULT,"W: ACCUMULATOR_VOLTAGE_TOO_HIGH          ");
  if (v & W_PWR_SRC_VOLTAGE_TOO_LOW)                 LOGs(0, 0, SEVERITY_DEFAULT,"W: PWR_SRC_VOLTAGE_TOO_LOW               ");
  if (v & W_PWR_SRC_VOLTAGE_TOO_HIGH)                LOGs(0, 0, SEVERITY_DEFAULT,"W: PWR_SRC_VOLTAGE_TOO_HIGH              ");
  if (v & W_INVERTER_SWITCHED_ON)                    LOGs(0, 0, SEVERITY_DEFAULT,"W: INVERTER_SWITCHED_ON                  ");
  if (v & W_INVERTER_CURRENT_PRESENT)                LOGs(0, 0, SEVERITY_DEFAULT,"W: INVERTER_CURRENT_PRESENT              ");
  if (v & W_DISABLED_CHARGING)                       LOGs(0, 0, SEVERITY_DEFAULT,"W: DISABLED_CHARGING                     ");
  if (v & W_DCDC_DAC_ERROR)                          LOGs(0, 0, SEVERITY_DEFAULT,"W: DCDC_DAC_ERROR                        ");
  if (v & W_CPU_TEMP_TOO_HIGH)                       LOGs(0, 0, SEVERITY_DEFAULT,"W: CPU_TEMP_TOO_HIGH                     ");
  if (v & W_PWR_SRC_DONT_READY)                      LOGs(0, 0, SEVERITY_DEFAULT,"W: PWR_SRC_DONT_READY                    ");
  if (v & W_ACCUMULATOR_DISCONNECTED)                LOGs(0, 0, SEVERITY_DEFAULT,"W: ACCUMULATOR_DISCONNECTED              ");
}


/*-----------------------------------------------------------------------------------------------------
  Детктор окончания заряда для режима с фиксированным напряжением и ограничением по току по признаку отсутствия уменьшения тока

  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Full_Charge_detector(float accum_i, float accum_v)
{
  if (Time_elapsed_sec(&fcd.ref_time) > 0)
  {
    Get_time(&fcd.ref_time);

    fcd.int_v = (int32_t)(accum_i * 100000.0f);

    // Дифференциатор
    fcd.int_dif = fcd.int_v - fcd.prev_int_v;
    fcd.prev_int_v = fcd.int_v;

    // Ограничитель
    if (fcd.int_dif > 2000)
    {
      fcd.int_dif = 2000;
    }
    else if (fcd.int_dif < -2000)
    {
      fcd.int_dif = -2000;
    }

    // Экспоненциальный фильтр
    fcd.int_dif = fcd.int_dif + fcd.exflt_state;
    fcd.exflt_state = fcd.int_dif - (fcd.int_dif >> 8);
    fcd.int_dif = fcd.int_dif >> 8;

    // Компаратор с накопителем
    if ((fcd.int_dif <= 5) && (fcd.int_dif >= -5))
    {
      fcd.cnt++;
    }
    else
    {
      fcd.cnt = 0;
    }

    // Ток должен сохранять значение не менее 20 мин при максимальном напряжении
    if ((accum_v >= wvar.max_charge_voltage) && (fcd.cnt > (60*20)))
    {
      return 1;
    }
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Выключение зарядки

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Goto_discharging_state(void)
{
  Set_EN_DCDC(0);          // Запрещаем работу DCDC преобразователя
  DELAY_8us;               // Делаем задержку чтобы DCDC успел отключиться
  Set_PIDS_SHDN(0);        // Источник питания подключен
  Set_AIDS_SHDN(1);        // Аккумулятор через идеальный диод к системе подключен. Подключение произойдет не раньше чем через 1.5 мс

  g_charger_dac_code = DCDC_MIN_VOLTAGE_CODE;
  Set_DCDC_DAC_code(g_charger_dac_code);  // Устанвливаем минимальное референсное напряжение зарядника
  sys.accum_status = ACCUM_DISCHARGING;
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Time elapsed = %d sec.", Time_elapsed_sec(&state_ref_time));
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"!!! Accumulator state: DISCHARGING !!!");
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Charging break status: emerg.mask=%08X, disabl.mask=%08X", sys.last_emergency_signals, sys.last_disabling_signals);
  Log_Emergency_signals(sys.last_emergency_signals);
  Log_Disable_signals(sys.last_disabling_signals);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Accumulator: Voltage %0.1f V, Current = %0.2f A, Charge = %0.2f Ah",Get_float_val(&sys.accum_v), Get_float_val(&sys.accum_sum_i), Get_float_val(&sys.accum_charge));
  Get_time(&state_ref_time);
}

/*-----------------------------------------------------------------------------------------------------
  Включение зарядки

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Goto_charging_state(void)
{
  ULONG actual_flags;

  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Time elapsed = %d sec.", Time_elapsed_sec(&state_ref_time));
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"!!! Accumulator state: CHARGING !!!");
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Accumulator: Voltage %0.1f V, Current = %0.2f A, Charge = %0.2f Ah",Get_float_val(&sys.accum_v), Get_float_val(&sys.accum_sum_i), Get_float_val(&sys.accum_charge));

  g_block_check_reverse = 1; // Блокируем проверку реверсного тока поскольку на этом этаме он допустим
  sys.accum_status = ACCUM_CHARGING;


  g_charger_dac_code =  DCDC_DAC_Volt_to_code(Get_float_val(&sys.accum_v) + 0.1f); // Устанавливаем напряжение DCDC меньше на 0.3В от напряжения аккумулятора

  Set_DCDC_MODE(1);                           // Устанавливаем 1 - pulse-skipping mode. В этом режиме нет обратного тока от выхода на вход
  Set_DCDC_DAC_code(g_charger_dac_code);      // Устанвливаем минимальное референсное напряжение зарядника
  // Задержка для установления уровня напряжений. Одновременно реакция на сигнал отбоя процеса зарядки
  if (tx_event_flags_get(&charger_flags, DCDC_OFF_EVENT, TX_AND_CLEAR,&actual_flags,  MS_TO_TICKS(50)) == TX_SUCCESS) goto _evt;

  Set_AIDS_FBC(0);    // Снижаем уровень ограничителя напряжения идеального диода аккмулятора в систему до 22.9 В
  Set_AIDS_SHDN(0);   // Выключаем идеальный диод от аккмулятора в систему

  // Задержка для установления уровня напряжений. Одновременно реакция на сигнал отбоя процеса зарядки
  if (tx_event_flags_get(&charger_flags, DCDC_OFF_EVENT, TX_AND_CLEAR,&actual_flags,  MS_TO_TICKS(50)) == TX_SUCCESS) goto _evt;

  g_block_check_reverse = 0; // Разлокируем проверку реверсного тока поскольку на этом этаме он недопустим

  Set_EN_DCDC(1);     // Разрешаем работу DCDC преобразователя

  sys.last_emergency_signals = 0;
  sys.last_disabling_signals = 0;
  full_charge_detector       = 0;
  disconnect_detector        = 0;
  memset(&fcd ,0 , sizeof(fcd));
  Get_time(&state_ref_time);
  return;

_evt:
  g_block_check_reverse = 0;
  Goto_discharging_state();
  return;

}

/*-----------------------------------------------------------------------------------------------------
  Перевод аккумулятора в режим IDLE

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Goto_IDLE_state(void)
{
  Set_DCDC_MODE(1);  // Устанавливаем 1 - pulse-skipping mode. В этом режиме нет обратного тока от выхода на вход
  sys.accum_status = ACCUM_IDLE;
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Time elapsed = %d sec.", Time_elapsed_sec(&state_ref_time));
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"!!! Accumulator state: IDLE !!!");
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Accumulator: Voltage %0.1f V, Current = %0.2f A, Charge = %0.2f Ah",Get_float_val(&sys.accum_v), Get_float_val(&sys.accum_sum_i), Get_float_val(&sys.accum_charge));
  Get_time(&state_ref_time);
}


/*-----------------------------------------------------------------------------------------------------
  Отключение аккумлятора от питания системы

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Goto_off_state(void)
{
  Set_PIDS_SHDN(0);      // Источник питания подключен
  Set_AIDS_SHDN(0);      // Аккумулятор через идеальный диод от системы отключен
  Set_INVSW_ON(0);       // Отключаем аккумулятор от инвертера
  g_charger_dac_code = DCDC_MIN_VOLTAGE_CODE;
  Set_DCDC_DAC_code(g_charger_dac_code);  // Устанвливаем минимальное референсное напряжение зарядника
  sys.accum_status = ACCUM_OFF;
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Time elapsed = %d sec.", Time_elapsed_sec(&state_ref_time));
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"!!! Accumulator state: OFF !!!");
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Accumulator: Voltage %0.1f V, Current = %0.2f A, Charge = %0.2f Ah",Get_float_val(&sys.accum_v), Get_float_val(&sys.accum_sum_i), Get_float_val(&sys.accum_charge));
  App_set_events(FLAG_ACCUM_SWITCHED_OFF);
  Get_time(&state_ref_time);
}

/*-----------------------------------------------------------------------------------------------------
  Состояние в котором аккмулятор отдает ток в систему и DCDC преобразователь выключен

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Do_Discharging_state(void)
{
  static uint8_t      low_v_flag = 0;
  static  TIME_STRUCT low_v_ref_time;

  float psrc_v       = Get_float_val(&sys.psrc_v);
  float accum_v      = Get_float_val(&sys.accum_v);
  float accum_charge = Get_float_val(&sys.accum_charge);
  float accum_id_i   = Get_float_val(&sys.accum_id_i);

  Set_EN_DCDC(0); // Держим выключеным DCDC преобразователь
  if (Time_elapsed_sec(&state_ref_time) > 10)
  {
    // Зарядку включаем не раньше чем через 10 сек после последнего отключения зарядки
    if (Is_charging_disallowed() == 0)
    {
      // Переходим к заряду если заряд аккумулятора меньше 90% от номинального значения
      if (accum_charge < (wvar.accum_capacity*0.9f))
      {
        Goto_charging_state();
        return;
      }
      // Переходим к заряду если напряжение аккумулятора ниже заданного граничного уровня
      if (accum_v < (wvar.accum_idle_voltage-1.0))
      {
        Goto_charging_state();
        return;
      }
    }
  }

  if (accum_v > MIN_ACCUM_VOLTAGE)
  {
    low_v_flag = 0;

    // Управление сигналом AIDS_FBC для установки ограничения напряжения передаваемого аккумулятором в систему через идеальный диод
    Set_AIDS_SHDN(1);   // Держим включенным идеальный диод от аккмулятора в систему
    if (psrc_v > ACCUM_IDIODE_LOW_BOUND + 0.5f)
    {
      Set_AIDS_FBC(0); // Если источник питания дает достаточно напряжения. Ограничитель напряжения аккумулятора ставим на 22.9 В. Потреблять ток аккумулятора не надо
    }
    else if (psrc_v < (ACCUM_IDIODE_LOW_BOUND - 0.5f))
    {
      Set_AIDS_FBC(1); // Если источник питания дает недостаточно напряжения. Ограничитель напряжения аккумулятора ставим на 26.3 В. Подключаем аккумулатор без ограничения
    }
    else
    {
      // В промежуточных значениях напряжения источника питания ориентируемся по току через идеальный диод
      if  (accum_id_i > ACCUM_TO_SYSTEM_BOUND_CURRENT)
      {
        Set_AIDS_FBC(1); // Ограничитель напряжения аккумулятора на 26.3 В
      }
      else
      {
        Set_AIDS_FBC(0); // Ограничитель напряжения аккумулятора на 22.9 В
      }
    }
  }
  else
  {
    // Если напряжение акумулятора слишком низкое, то отключаем аккумулятор, но не раньше чем это состояние продержиться дольше 10 сек
    if (low_v_flag == 0)
    {
      low_v_flag = 1;
      Get_time(&low_v_ref_time);
    }
    else
    {
      if (Time_elapsed_sec(&low_v_ref_time) > 10)
      {
        Goto_off_state();
        return;
      }
    }
  }

}


/*-----------------------------------------------------------------------------------------------------
  Состояние в котором идет зарадка аккумулятора

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Do_Charging_state(void)
{
  static  TIME_STRUCT  full_charge_ref_time;
  static  TIME_STRUCT  disconnect_ref_time;

  float accum_i      = Get_float_val(&sys.accum_i);
  float accum_v      = Get_float_val(&sys.accum_v);
  float accum_charge = Get_float_val(&sys.accum_charge);

  // Заряжаем.
  // Определить не стоит ли отключить зарядку
  if (Is_charging_disallowed() != 0)
  {
    Goto_discharging_state();
    return;
  }
  else
  {
    // Продолжаем зарядку
    // Если ток заряда меньше заданного, то увеличиваем напряжение на аккмуляторе
    // Если ток заряда больше, то уменьшаем напряжение на аккумуляторе
    if ((-accum_i < wvar.max_charge_current) && (accum_v <= wvar.max_charge_voltage))
    {
      if (g_charger_dac_code != 0)
      {
        g_charger_dac_code--; // Поднимаем напряжение
      }
      Set_DCDC_DAC_code(g_charger_dac_code);
    }
    else if (-accum_i > (wvar.max_charge_current+0.5f))
    {
      if (g_charger_dac_code != DCDC_MIN_VOLTAGE_CODE)
      {
        g_charger_dac_code++; // Снижаем напряжение
      }
      Set_DCDC_DAC_code(g_charger_dac_code);
    }

    //  Данный код вызывает осциллирующее переключение между forced continuous mode и  pulse-skipping mode
    //  так как при переходе на pulse-skipping mode подскакивает ток заряда выше границы
    //  Также режим pulse-skipping mode вызывает сильное возрастание шумов
    //
    //  if (-accum_i > 0.1f)
    //  {
    //    Set_DCDC_MODE(0);   // Устанавливаем  0 - forced continuous mode. Поскольку в этом режиме меньше шум
    //  }
    //  else
    //  {
    //    Set_DCDC_MODE(1);   // Устанавливаем  1 - pulse-skipping mode. Чтобы предотвратить реверисивное течение тока
    //  }

    // Детекция полного заряда аккумулятора
    // Аккумулятор полностью зарядился если его ток заряда меньше заданного значения более 1 мин.
    // Стандартно граничным током считается 0.01С , т.е. для 12Ач аккумулятора будет 0.12 А
    if ((-accum_i < end_charge_current) && (accum_v > (wvar.max_charge_voltage - 0.1f)))
    {
      if (full_charge_detector == 0)
      {
        full_charge_detector = 1;
        Get_time(&full_charge_ref_time);
      }
      else
      {
        // Проверяем сколько времени мы уже детектируем полный заряд
        if (Time_elapsed_sec(&full_charge_ref_time) > 60)
        {
          App_set_events(FLAG_ACCUM_FULLY_CHARGED);
          Goto_discharging_state();
          return;
        }
      }
    }
    else
    {
      full_charge_detector = 0;
    }

    // Обнаружитель отключения аккумулятора
    if (-accum_i < ACCUM_DISCONECT_DETECT_CURRENR)
    {
      if (disconnect_detector == 0)
      {
        disconnect_detector  =1;
        Get_time(&disconnect_ref_time);
      }
      else
      {
        // Проверяем сколько времени мы уже детектируем полный заряд
        if (Time_elapsed_sec(&disconnect_ref_time) >= 1)
        {
          sys.last_disabling_signals |= W_ACCUMULATOR_DISCONNECTED;
          Goto_discharging_state();
          return;
        }
      }
    }
    else
    {
      disconnect_detector = 0;
    }


    // Аккумулятор полностью зарядился если суммарный накопленный заряд превышает установленную емкость аккумулятора на 20%
    if (accum_charge > (wvar.accum_capacity*1.2f))
    {
      App_set_events(FLAG_ACCUM_FULLY_CHARGED);
      Goto_discharging_state();
      return;
    }
    // Аккумулятор полностью зарядился если это определил детектор отсутствия уменьшения тока
    if (Full_Charge_detector(accum_i, accum_v)!=0)
    {
      App_set_events(FLAG_ACCUM_FULLY_CHARGED);
      Goto_discharging_state();
      return;
    }
    // Прервать заряд если превышен лимит времени
    if (Time_elapsed_sec(&state_ref_time) > (wvar.max_charge_dur_h * 60.0f * 60.0f))
    {
      Goto_discharging_state();
      return;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  IDLE - состояние в котором поддерживается на аккумуляторе постоянное напряжение равное значению из wvar.accum_idle_voltage

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Do_IDLE_state(void)
{
  float accum_v      = Get_float_val(&sys.accum_v);

  // Определить не стоит ли отключить зарядку
  if (Is_charging_disallowed() != 0)
  {
    Goto_discharging_state();
    return;
  }
  else
  {
    if (accum_v > (wvar.accum_idle_voltage+0.1f))
    {
      if (g_charger_dac_code != DCDC_MIN_VOLTAGE_CODE)
      {
        g_charger_dac_code++; // Снижаем напряжение
      }
      Set_DCDC_DAC_code(g_charger_dac_code);
    }
    else if (accum_v < wvar.accum_idle_voltage)
    {
      if (g_charger_dac_code != 0)
      {
        g_charger_dac_code--; // Поднимаем напряжение
      }
      Set_DCDC_DAC_code(g_charger_dac_code);
    }
  }
  if (Time_elapsed_sec(&state_ref_time) > (60*60*8))  // Выходим из состояния  через 8 часов
  {
    Goto_discharging_state();
    return;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Do_OFF_state(void)
{
  float accum_v      = Get_float_val(&sys.accum_v);

  // Проверять напряжение на аккмуляторе периодически на предмет смены аккумулятора
  if (accum_v > (MIN_ACCUM_VOLTAGE + 1.0f))
  {
    if (Time_elapsed_sec(&state_ref_time) > 60)  // Выходим из состояния не раньше чем через минуту
    {
      Goto_discharging_state();
      return;
    }
  }
  else
  {
    Get_time(&state_ref_time);
  }
}

int32_t Get_task_enable(void)       { return sys.charger_task_active;     }
void   Set_task_enable(int32_t val) {  sys.charger_task_active = val;     }


/*-----------------------------------------------------------------------------------------------------

  Автомат состояний зарядника

-----------------------------------------------------------------------------------------------------*/
static void Charger_state_machine(void)
{
  switch (sys.accum_status)
  {
  case ACCUM_DISCHARGING: Do_Discharging_state(); break;
  case ACCUM_CHARGING   : Do_Charging_state();    break;
  case ACCUM_IDLE       : Do_IDLE_state();        break;
  case ACCUM_OFF        : Do_OFF_state();         break;
  default:
    // Неопределенное состояние. Не должно появляться.
    Goto_discharging_state();
    break;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param p_arg
-----------------------------------------------------------------------------------------------------*/
static  void  Charger_task(ULONG initial_input)
{
  ULONG    actual_flags;

  if (tx_event_flags_create(&charger_flags, "Charger") != TX_SUCCESS)
  {
    return;
  }
  sys.charger_task_active = 1;

  min_pwr_src_voltage_code       = (int32_t)MIN_PWR_SRC_VOLTAGE_CODE;
  max_pwr_src_voltage_code       = (int32_t)MAX_PWR_SRC_VOLTAGE_CODE;
  min_accum_voltage_code         = (int32_t)MIN_ACCUM_VOLTAGE_CODE;
  max_accum_voltage_code         = (int32_t)MAX_ACCUM_VOLTAGE_CODE;
  max_pulse_charge_current_code  = (int32_t)MAX_PULSE_CHARGE_CURRENT_CODE;
  max_chrg_reverse_current_code  = (int32_t)MAX_CHRG_REVERSE_CURRENT_CODE;
  max_pwrsrc_charge_current_code = (int32_t)MAX_PWRSRC_CHARGE_CURRENT_CODE;
  end_charge_current             = wvar.accum_capacity*MIN_CHARGING_CURRENT_SCALE;

  Set_EN_DCDC(0);          // Запрещаем работу DCDC преобразователя
  Set_PIDS_SHDN(0);        // Источник питания подключен
  Set_AIDS_SHDN(1);        // Аккумулятор через идеальный диод к системе подключен.

  g_charger_dac_code = DCDC_MIN_VOLTAGE_CODE;
  Set_DCDC_DAC_code(g_charger_dac_code);  // Устанвливаем минимальное референсное напряжение зарядника
  sys.accum_status = ACCUM_DISCHARGING;

  Wait_ms(100);
  Get_time(&state_ref_time);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Charger task started.");
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Accumulator: Voltage %0.1f V, Current = %0.2f A, Charge = %0.2f Ah",Get_float_val(&sys.accum_v), Get_float_val(&sys.accum_sum_i), Get_float_val(&sys.accum_charge));


  do
  {
    // Ожидаем все возможные события
    if (tx_event_flags_get(&charger_flags, 0xFFFFFFFF, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(CHARGER_TASK_PERIOD_MS)) == TX_SUCCESS)
    {
      if ((actual_flags & DCDC_OFF_EVENT) && (sys.charger_task_active == 1))
      {
        Goto_discharging_state();
      }
    }
    else
    {
      if (sys.charger_task_active == 1)
      {
        Charger_state_machine();
      }
    }
  }while (1);

}




