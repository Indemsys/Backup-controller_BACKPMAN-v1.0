#include "App.h"

typedef void (*T_PIT_isr)(void  *user_isr_ptr);

static IRQn_Type       PIT_int_nums[PIT_CHANNELS_NUM] = {LPIT0_Ch0_IRQn, LPIT0_Ch1_IRQn, LPIT0_Ch2_IRQn, LPIT0_Ch3_IRQn};
static T_PIT_callback  PIT_callbacks[PIT_CHANNELS_NUM] = {0};

/*-------------------------------------------------------------------------------------------------------------
  Обработчик прерывания от модуля PIT с номером modn
-------------------------------------------------------------------------------------------------------------*/
static void PIT_isr(uint8_t chnl)
{
  LPIT0->MSR =(1 << chnl); // Сбрасываем прерывание

  if (PIT_callbacks[chnl] != 0) PIT_callbacks[chnl](); // Вызываем callback функцию еслиона зарегистрирована

}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void LPIT0_Ch0_IRQHandler(void *user_isr_ptr)
{
  PIT_isr(0);
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void LPIT0_Ch1_IRQHandler(void *user_isr_ptr)
{
  PIT_isr(1);
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void LPIT0_Ch2_IRQHandler(void *user_isr_ptr)
{
  PIT_isr(2);
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void LPIT0_Ch3_IRQHandler(void *user_isr_ptr)
{
  PIT_isr(3);
}



/*-------------------------------------------------------------------------------------------------------------
  Инициализация модуля Periodic Interrupt Timer (PIT)
  Один из каналов PIT используется как источник сигналов триггера для запука ADC

  Тактируется модуль от Bus Clock -> CPU_BUS_CLK_HZ = 60 МГц
-------------------------------------------------------------------------------------------------------------*/
void PIT_init_module(void)
{
  PCC->CLKCFG[PCC_LPIT0_INDEX] = 0
                                + BIT(30)
                                + LSHIFT(6, 24) // PCS |  Clock selection | 110b - System PLL clock
  ;

  LPIT0->MCR = 0
              + LSHIFT(0, 3) // DBG_EN   | Allows the timer channels to be stopped when the device enters the Debug mode
              + LSHIFT(0, 2) // DOZE_EN  | Allows the timer channels to be stopped or continue to run when the device enters the DOZE mode
              + LSHIFT(0, 1) // SW_RST   | Software Reset Bit
              + LSHIFT(1, 0) // FM_CENRZ | Module Clock Enable
  ;
}

/*-----------------------------------------------------------------------------------------------------
  Инициализация канала PIT с формированием прерываний и вызовом callback функции

  \param chnl
  \param period  - период таймера в микросекундах
  \param prio
  \param en_int
  \param callback_func
-----------------------------------------------------------------------------------------------------*/
void PIT_init_channel(uint8_t chnl, uint32_t period, uint8_t prio, uint8_t en_int,  T_PIT_callback callback_func)
{
  if (chnl >= PIT_CHANNELS_NUM) return;

  LPIT0->MIER &= ~(1 << chnl); // Запрещаем прерывания

  LPIT0->CHANNEL[chnl].TVAL =((PLLDIV2_CLK / 1000000ul) * period); // Загружаем период таймера

  LPIT0->CHANNEL[chnl].TCTRL = 0
                              + LSHIFT(0, 24) // TRG_SEL | Trigger Select          | 0 Timer channel 0 trigger source is selected
                              + LSHIFT(0, 23) // TRG_SRC | Trigger Source          | 0 Trigger source selected in external
                              + LSHIFT(0, 18) // TROT    | Timer Reload On Trigger | 0 Timer will not reload on selected trigger
                              + LSHIFT(0, 17) // TSOI    | Timer Stop On Interrupt | 0 Timer does not stop after timeout
                              + LSHIFT(0, 16) // TSOT    | Timer Start On Trigger  | 0 Timer starts to decrement immediately based on restart condition (controlled by TSOI bit)
                              + LSHIFT(0, 2)  // MODE    | Timer Operation Mode    | 00 32-bit Periodic Counter
                              + LSHIFT(0, 1)  // CHAIN   | Chain Channel           | 0 Channel Chaining is disabled. Channel Timer runs independently.
                              + LSHIFT(1, 0)  // T_EN    | Timer Enable            | 1 Timer Channel is enabled
  ;
  if (en_int != 0)
  {
    if (callback_func != 0) PIT_callbacks[chnl] = callback_func;

    NVIC_SetPriority(PIT_int_nums[chnl], prio);
    NVIC_EnableIRQ(PIT_int_nums[chnl]);



    LPIT0->MIER |=(1 << chnl); // Разрешаем  прерывания
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param chnl
-----------------------------------------------------------------------------------------------------*/
void PIT_disable(uint8_t chnl)
{
  if (chnl >= PIT_CHANNELS_NUM) return;

  LPIT0->CHANNEL[chnl].TCTRL = 0
                              + LSHIFT(0, 24) // TRG_SEL | Trigger Select          | 0 Timer channel 0 trigger source is selected
                              + LSHIFT(0, 23) // TRG_SRC | Trigger Source          | 0 Trigger source selected in external
                              + LSHIFT(0, 18) // TROT    | Timer Reload On Trigger | 0 Timer will not reload on selected trigger
                              + LSHIFT(0, 17) // TSOI    | Timer Stop On Interrupt | 0 Timer does not stop after timeout
                              + LSHIFT(0, 16) // TSOT    | Timer Start On Trigger  | 0 Timer starts to decrement immediately based on restart condition (controlled by TSOI bit)
                              + LSHIFT(0, 2)  // MODE    | Timer Operation Mode    | 00 32-bit Periodic Counter
                              + LSHIFT(0, 1)  // CHAIN   | Chain Channel           | 0 Channel Chaining is disabled. Channel Timer runs independently.
                              + LSHIFT(0, 0)  // T_EN    | Timer Enable            | 1 Timer Channel is enabled
  ;
  PIT_callbacks[chnl] = 0;
}

/*-------------------------------------------------------------------------------------------------------------
   Получить текущее значение таймера. Таймер ведет счет до 0 и обносляет свое занчение величиной из PIT->CHANNEL[0].LDVAL
 -------------------------------------------------------------------------------------------------------------*/
uint32_t PIT_get_curr_val(uint8_t chnl)
{
  if (chnl >= PIT_CHANNELS_NUM) return 0;
  return LPIT0->CHANNEL[chnl].CVAL;
}

/*-------------------------------------------------------------------------------------------------------------
   Получить загрузочное  значение таймера.
 -------------------------------------------------------------------------------------------------------------*/
uint32_t PIT_get_load_val(uint8_t chnl)
{
  if (chnl >= PIT_CHANNELS_NUM) return 0;
  return LPIT0->CHANNEL[chnl].TVAL;
}


