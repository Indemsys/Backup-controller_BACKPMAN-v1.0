#define  _MAIN_GLOBALS_
#include "App.h"



#define DEMO_LPUART          LPUART0
#define DEMO_LPUART_CLKSRC   kCLOCK_ScgSysPllAsyncDiv2Clk
#define DEMO_LPUART_CLK_FREQ CLOCK_GetIpFreq(kCLOCK_Lpuart0)

uint8_t                       recv_buffer[1];

uint8_t                       background_buffer[32];


#define uart_task_PRIORITY   (configMAX_PRIORITIES - 1)

TX_THREAD                     main_thread;
#pragma data_alignment=8
uint8_t                       thread_main_stack[THREAD_MAIN_STACK_SIZE];
static void                   Thread_main(ULONG initial_input);
static TX_EVENT_FLAGS_GROUP   app_flags_grp;

/*-----------------------------------------------------------------------------------------------------
  Вызывается сразу после старта по цепочке:  Reset_Handler -> SystemInit -> SystemInitHook

  \param void
-----------------------------------------------------------------------------------------------------*/
void SystemInitHook(void)
{
  BCKUPC10_init_pins();
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int
-----------------------------------------------------------------------------------------------------*/
int main(void)
{
  BOARD_BootClockRUN();
  Init_DMA();
  Init_outputs_states();
  tx_kernel_enter();
}


/*-----------------------------------------------------------------------------------------------------
  Вызывается из tx_kernel_enter

  \param first_unused_memory
-----------------------------------------------------------------------------------------------------*/
void    tx_application_define(void *first_unused_memory)
{
  tx_thread_create(&main_thread, "Main", Thread_main,
    0,
    (void *)thread_main_stack, // stack_start
    THREAD_MAIN_STACK_SIZE,   // stack_size
    THREAD_MAIN_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
    THREAD_MAIN_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}

/*-----------------------------------------------------------------------------------------------------
  Функция передачи флагов в главный поток

  \param flags
-----------------------------------------------------------------------------------------------------*/
void App_set_events(uint32_t flags)
{
  tx_event_flags_set(&app_flags_grp, flags, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Periodic_PIT_callback(void)
{
  App_set_events(FLAG_PERIODIC_TICK);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Time_PIT_callback(void)
{
  system_sec_counter++;
}


/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_main(ULONG initial_input)
{
  uint32_t err = 0;

  SIM->CHIPCTL &= ~((3 << 6) | (3 << 4));
  SIM->CHIPCTL |=(1 << 6) | (2 << 4); // На CLKOUT (pin 24) выводим частоту PLL деленную на 4
  SCG->CLKOUTCNFG =(6 << 24);

  // Выполняем контрольный замер времени одновременно эта задержка служит для обеспечения установления референсных напряжений в чипе
  ref_time = Measure_reference_time_interval(REF_TIME_INTERVAL);

  App_create_pool_memry();
  tx_event_flags_create(&app_flags_grp, "Main");

  Thread_idle_create();
  AppLogg_init();

  PIT_init_module();
  // Запускаем генератор прерываний для отсчета времени
  PIT_init_channel(TIME_PIT_TIMER, TIME_TICK_PERIOD, TIME_PIT_PRIO, 1,  Time_PIT_callback);
  ADCs_subsystem_init();                 // Инициализируем работу многоканального АЦП
  Outputs_init();

  // Запускаем генератор прерываний для генерации события FLAG_PERIODIC_TICK
  PIT_init_channel(SCAN_TASK_PIT_TIMER, OUTPUT_TICK_PERIOD, PIT1_PRIO, 1,  Periodic_PIT_callback);

  BCKUPC10_I2C1_Init();
  Set_DCDC_DAC_code(DCDC_DAC_Volt_to_code(0.0f));

  SPI_Init(U15_SPI_ID);
  SPI_Init(U17_SPI_ID);

  Set_output_off(OUTP_LED_GREEN);
  Set_output_off(OUTP_LED_RED);

  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"################### Main task started ###################");

  FLASH_set_EEPROM_condition();          // Проверяем и устанавливаем если надо нужную конфигурацию размера EEPROM
  Restore_NV_parameters();               // Переписывает в RAM параметры и установки из EEPROM

  Thread_CAN_create();

  //wvar.vcom_mode = 0;
  if (wvar.vcom_mode == 0)
  {
    if (Get_ENC_SW()==0 )
    {
      FreeMaster_task_create();
    }
    else
    {
      Task_VT100_create(&uart0_driver, 0);
    }
  }
  else
  {
    if (Get_ENC_SW()==0 )
    {
      Task_VT100_create(&uart0_driver, 0);
    }
    else
    {
      FreeMaster_task_create();
    }
  }

  if (ACS37800_test() != RES_OK) err++;
  if (DCDC_DAC_initializing() != RES_OK) err++;

  if (err> 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"System stopped. Failure state");
    while (1)
    {
      time_delay(10);
    }
  }

  Mem_man_log_statistic();

  while (1)
  {
    ULONG  actual_flags;
    if (tx_event_flags_get(&app_flags_grp, FLAG_ADC_DONE | FLAG_PERIODIC_TICK, TX_OR_CLEAR ,&actual_flags, MS_TO_TICKS(100)) == TX_SUCCESS)
    {

      // Обработка флага завершения заполнения новых отсчетов ADC.
      // Флаг FLAG_ADC_DONE устанавливается в процедуре прерывания по окончании цикла DMA
      // Передачи DMA вызываются по окончании выборок ADC
      // Выборки ADC вызываются по сигналу таймера PIT0
      if (actual_flags & FLAG_ADC_DONE)          // Период 1024 мкс
      {
        //ITM_EVENT8(1,1);

        BCKUPC10_Inputs_handling();              //
        Manual_Encoder_handling();
        Signals_conditioning();
        OLED_control_function();                 // 0.69 мкс
        Outputs_state_automat();                 // 4.9 мкс
        Synchronize_output_signals();            // 1.3 мкс
        App_set_events(FLAG_POWER_CONTROL_TICK); // 3.15 мкс
        ACS37800_read();                         // 58 мкс
        DCDC_DAC_refresh();                      // 117 мкс
                                                 //
        //ITM_EVENT8(1,0);
      }


      // Флаг FLAG_PERIODIC_TICK выставляется в процедуре прерывания от таймера PIT2
      if (actual_flags & FLAG_PERIODIC_TICK)     // Период 1000 мкс
      {
      }
    }
    else
    {
      time_delay(10);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Расчет значений с плавающей точкой.
  Время выполнения без оптимизации 3.69 мкс, с оптимизацией - 2.8 мкс


  \param void
-----------------------------------------------------------------------------------------------------*/
void Signals_conditioning(void)
{
  TX_INTERRUPT_SAVE_AREA
  static uint32_t cn    = 0;
  static float    sum_i = 0;
  static uint32_t cpu_overheat_cnt = 0;
  static uint8_t  prev_overheat_state = 0;


  //ITM_EVENT8(1,1);
  TX_DISABLE;
  sys.accum_id_i       =  Value_fp_ACCUM_ID_I(sys.fp_smpl_accum_id_i);
  sys.accum_i          =  Value_fp_ACCUM_I(sys.fp_smpl_accum_i);
  sys.psrc_i           =  Value_fp_PSRC_I(sys.fp_smpl_psrc_i);
  sys.invert_i         =  Value_fp_INVERT_I(sys.fp_smpl_invert_i);
  sys.invert_v         =  Value_fp_INVERT_V(sys.fp_smpl_invert_v);
  sys.sys_v            =  Value_fp_SYS_V(sys.fp_smpl_sys_v);
  sys.psrc_v           =  Value_fp_PSRC_V(sys.fp_smpl_psrc_v);
  sys.accum_v          =  Value_fp_ACCUM_V(sys.fp_smpl_accum_v);
  sys.accum_sum_i      =  sys.invert_i - sys.accum_i;
  sys.cpu_temper       =  CPU_TEMP(sys.fp_smpl_cpu_temper);

  if (Value_INV_PATH_STATE(Get_INV_PATH_STATE()) < 3.0f)
  {
    sys.mains_path_state = 1;
  }
  else
  {
    sys.mains_path_state = 0;
  }

  if (Value_MAINS_PATH_STATE(Get_MAINS_PATH_STATE()) < 3.0f)
  {
    sys.inv_path_state   = 1;
  }
  else
  {
    sys.inv_path_state   = 0;
  }

  sys.pwr_src_power = sys.psrc_i * sys.psrc_v;
  if (sys.accum_sum_i > 0)
  {
    sys.accum_charge_power = -sys.accum_i * sys.accum_v;
    sys.accum_discharge_power = 0;
    sys.chaging_pwr_loss = sys.pwr_src_power - sys.accum_charge_power;
    sys.charging_efficiency =(sys.accum_charge_power / sys.pwr_src_power) * 100;
  }
  else
  {
    sys.accum_charge_power = 0;
    sys.accum_discharge_power = sys.accum_sum_i * sys.accum_v;
    sys.chaging_pwr_loss = 0;
    sys.charging_efficiency = 0;
  }

  // Расчет количества заряда в аккумуляторе
  if (cn < 1000)
  {
    cn++;
    sum_i += sys.accum_sum_i;
  }
  else
  {
    cn = 0;
    sys.accum_charge += sum_i * ((float)MAIN_CYCLE_PERIOD_US)/(1000000.0f*60.0f*60.0f);
    sum_i = 0;
  }
  TX_RESTORE;

  // Обнаружитель перегрева
  if (sys.cpu_temper > MAX_CPU_TEMPERATURE)
  {
    if (prev_overheat_state == 1)
    {
      uint32_t bound = MAIN_CYCLE_PERIOD_US;
      bound =  1000000ul/bound ;
      cpu_overheat_cnt++;
      if (cpu_overheat_cnt > bound) // Фиксируем перегрев если температура продержалась выше границы более 1 сек
      {
        sys.cpu_overheat = 1;
        cpu_overheat_cnt--;
      }
    }
    else
    {
      cpu_overheat_cnt = 0;
      prev_overheat_state = 1;
    }
  }
  else
  {
    if (prev_overheat_state == 0)
    {
      uint32_t bound = MAIN_CYCLE_PERIOD_US;
      bound =  10000000ul/bound ;
      cpu_overheat_cnt++;
      if (cpu_overheat_cnt > bound) // Сбрасываем перегрев если температура продержалась ниже границы более 10 сек
      {
        sys.cpu_overheat = 0;
        cpu_overheat_cnt--;
      }
    }
    else
    {
      cpu_overheat_cnt=0;
      prev_overheat_state = 0;
    }
  }


  if (Get_float_val(&sys.vrms_u15) > MIN_MAINS_VOLTAGE)
  {
    sys.mains_voltage_ok = 1;
  }
  else
  {
    sys.mains_voltage_ok = 0;
  }

  if (Get_float_val(&sys.vrms_u17) > MIN_OUTPUT_VOLTAGE)
  {
    sys.output_voltage_ok = 1;
  }
  else
  {
    sys.output_voltage_ok = 0;
  }

  sys.out220v_power = Get_float_val(&sys.vrms_u17)*Get_float_val(&sys.irms_u17);



  //ITM_EVENT8(1,0);
}

/*-----------------------------------------------------------------------------------------------------
  Запаковать состояние системы в 8-и байтный пакет CAN
  Функция вызывается из задачи CAN

  \param buf
-----------------------------------------------------------------------------------------------------*/
void Pack_sytem_state_to_can_msg(uint8_t* out_buf)
{
  T_sys_state_packet pkt;

  memset(out_buf, 0, 8);
  memset(&pkt, 0, sizeof(pkt));

  pkt.charging_active  = 0;
  pkt.accum_charge = 0;

  pkt.mains_voltage_ok  = sys.mains_voltage_ok;
  pkt.output_voltage_ok = sys.output_voltage_ok;
  pkt.iverter_is_on     = Get_INVSW_ON();
  pkt.iverter_relay     = sys.inv_path_state;
  pkt.mains_relay       = sys.mains_path_state;
  pkt.iverter_power     = (uint32_t)sys.out220v_power;

  memcpy(out_buf, &pkt, sizeof(pkt));
}

/*-----------------------------------------------------------------------------------------------------
  Выполнить команду поступившую в 8-и байтном пакете CAN
  Функция вызывается из задачи CAN

  \param cmd_buf
-----------------------------------------------------------------------------------------------------*/
void Execute_can_command(uint8_t* cmd_buf)
{


}


