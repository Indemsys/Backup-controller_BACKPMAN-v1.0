// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.12.19
// 15:03:18
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
#define    ADC_GLOBAL
#include   "App.h"
#include   "freemaster.h"
/*
  Подколючение аналоговых сигналов к микроконтроллеру

  pin 54, PORTB0,  ACCUM_ID_I       : ADC0_SE4
  pin 53, PORTB1,  ACCUM_I          : ADC0_SE5
  pin 40, PORTC0,  PSRC_I           : ADC0_SE8
  pin 39, PORTC1,  INVERT_I         : ADC0_SE9
  pin 43, PORTC17, INVERT_V         : ADC0_SE15

  pin 71, PORTD2,  PWR24_V          : ADC1_SE2
  pin 70, PORTD3,  MAINS_PATH_STATE : ADC1_SE3
  pin 69, PORTD4,  INV_PATH_STATE   : ADC1_SE6
  pin 68, PORTB12, PSRC_V           : ADC1_SE7

  pin 3 , PORTD1,  VREF_1.65V       : ADC2_SE1
  pin 75, PORTB10, ACCUM_V          : ADC2_SE9
*/


#pragma data_alignment= 64 // Выравнивание необязательно
const uint8_t  adc0_ch_cfg[ADC_SCAN_SZ] =
{
  // Сигнал                   Канал АЦП
  //
  5,   // CHARGE_I            ADC0_SE5         Ток зарядника
  8,   // PSRC_I              ADC0_SE8         Ток источника питания
  9,   // INV_I               ADC0_SE9         Ток инвертера
  15,  // INVERT_V            ADC0_SE15        Напряжение инвертера
  16,  // reserved            reserved
  17,  // reserved            reserved
  18,  // reserved            reserved
  // В структуре T_ADC_res этот сигнал идет первым, а здесь должен быть последним !!!
  4,   // ACC_I               ADC0_SE4         Ток аккумулятора
};


#pragma data_alignment= 64 // Выравнивание необязательно
const uint8_t  adc1_ch_cfg[ADC_SCAN_SZ] =
{
  // Сигнал                   Канал АЦП
  //
  3,    // MAINS_PATH_STATE   ADC1_SE3         Сигнал с обмотки реле включающего инвертер на выход 220 платы
  6,    // INV_PATH_STATE     ADC1_SE6         Сигнал с обмотки реле включающего сеть на выход 220 платы
  7,    // PSRC_V             ADC1_SE7         Напряжение на источнике питания
  7,    // PSRC_V             ADC1_SE7
  16,   // reserved            reserved
  17,   // reserved            reserved
  18,   // reserved            reserved
  // В структуре T_ADC_res этот сигнал идет первым, а здесь должен быть последним !!!
  2,    // PWR24_V            ADC1_SE2         Напряжение уходящее в систему
};

#pragma data_alignment= 64 // Выравнивание необязательно
const uint8_t  adc2_ch_cfg[ADC_SCAN_SZ] =
{
  // Сигнал        Канал АЦП             Аналоговые функции
  //
  9,    // ACCUM_V             ADC2_SE9
  26,   // Temperature Sensor (S.E)     Внутренний температурный сенсор
  26,   // Temperature Sensor (S.E)     Внутренний температурный сенсор
  26,   // Temperature Sensor (S.E)     Внутренний температурный сенсор
  29,   // VREFH (S.E)                  Верхний уровень опорного напряжения
  29,   // VREFH (S.E)                  Верхний уровень опорного напряжения
  30,   // VREFL                        Нижний уровень опорного напряжения
  // В структуре T_ADC_res этот сигнал идет первым, а здесь должен быть последним !!!
  1,    // VREF_1.65V           ADC2_SE1
};


#pragma data_alignment= 64 // Массивы результатов выравниваем по границе с пятью младшими битами равными нулю
volatile uint16_t adc0_results[2][ADC_SCAN_SZ];

#pragma data_alignment= 64 // Массивы результатов выравниваем по границе с пятью младшими битами равными нулю
volatile uint16_t adc1_results[2][ADC_SCAN_SZ];

#pragma data_alignment= 64 // Массивы результатов выравниваем по границе с пятью младшими битами равными нулю
volatile uint16_t adc2_results[2][ADC_SCAN_SZ];


// Массив структур для инициализации каналов DMA обслуживающих 3-и модуля ADC.
// Всего занимаем для нужд ADC 8-мь каналов DMA: 0,1,2,3,4,5,6,7 и вектор прерывания INT_DMA0
const T_init_ADC_DMA_cbl init_ADC_DMA_cbl[3] =
{
  { ADC0, adc0_results[0], adc0_ch_cfg, DMA_ADC0_RES_CH, DMA_ADC0_CFG_CH, DMA_ADC0_DMUX_SRC },
  { ADC1, adc1_results[0], adc1_ch_cfg, DMA_ADC1_RES_CH, DMA_ADC1_CFG_CH, DMA_ADC1_DMUX_SRC },
  { ADC2, adc2_results[0], adc2_ch_cfg, DMA_ADC2_RES_CH, DMA_ADC2_CFG_CH, DMA_ADC2_DMUX_SRC },
};



static uint8_t    g_res_bank;
static uint32_t   g_adc_cnt;

volatile uint16_t s_VREF_1_65V;
volatile int16_t  s_ACCUM_ID_I;
volatile int16_t  s_ACCUM_I;
volatile int16_t  s_PSRC_I;
volatile int16_t  s_INVERT_I;
volatile int16_t  s_PSRC_V;
volatile int16_t  s_ACCUM_V;
volatile int16_t  s_CPU_temper;


T_median_filter_uint16   mfltr_VREF_1_65V;
T_median_filter_uint16   mfltr_ACCUM_ID_I;
T_median_filter_uint16   mfltr_ACCUM_I;
T_median_filter_uint16   mfltr_PSRC_I;
T_median_filter_uint16   mfltr_INVERT_I;
T_median_filter_uint16   mfltr_PSRC_V;
T_median_filter_uint16   mfltr_ACCUM_V;
T_median_filter_uint16   mfltr_CPU_temper;

static T_LPF_But_100_31250     lpflt_ACCUM_ID_I;
static T_LPF_But_100_31250     lpflt_ACCUM_I;
static T_LPF_But_100_31250     lpflt_PSRC_I;
static T_LPF_But_100_31250     lpflt_INVERT_I;
static T_LPF_But_100_31250     lpflt_INVERT_V;
static T_LPF_But_100_31250     lpflt_PWR24_V;
static T_LPF_But_100_31250     lpflt_PSRC_V;
static T_LPF_But_100_31250     lpflt_ACCUM_V;

/*-------------------------------------------------------------------------------------------------------------
 Включаем тактирование всех ADC
-------------------------------------------------------------------------------------------------------------*/
void ADC_switch_on_all(void)
{
  //

  PCC->CLKCFG[PCC_ADC0_INDEX] = 0
                               + LSHIFT(1, 30) // CGC | Clock Control                  | 1b - Clock enabled
                               + LSHIFT(6, 24) // PCS | Peripheral Clock Source Select | 110b - System PLL clock
  ;

  PCC->CLKCFG[PCC_ADC1_INDEX]  = 0
                                + LSHIFT(1, 30) // CGC | Clock Control                  | 1b - Clock enabled
                                + LSHIFT(6, 24) // PCS | Peripheral Clock Source Select | 110b - System PLL clock
  ;

  PCC->CLKCFG[PCC_ADC2_INDEX]  = 0
                                + LSHIFT(1, 30) // CGC | Clock Control                  | 1b - Clock enabled
                                + LSHIFT(6, 24) // PCS | Peripheral Clock Source Select | 110b - System PLL clock
  ;

}

/*-------------------------------------------------------------------------------------------------------------
  Первоначальная инициализация ADC

  В режиме RUN c системной частотой 120 МГц тактирование ADC ведется от частоты PLL деленой на 2 = 60 MHz
  Максимальная частота тактирования ADC = 50 МГц
  Устанавливаем частоту = 30 МГц

  ADC TOTAL CONVERSION TIME = Sample Phase Time (set by SMPLTS+1) + Hold Phase (1 ADC Cycle) + Compare Phase Time (8-bit Mode=20 ADC Cycles, 10-bit Mode=24 ADC Cycles, 12-bit Mode=28 ADC Cycles)
  + Single or First continuous time adder (5 ADC cycles + 5 bus clock cycles)

  Длительность преобразования (без учета времени выборки) при ADCLK 30 МГц и частоте шины 60 МГц
  В тактах шины = (1 + 28 + 5)*2 + 5 = 73 -> 1.2 мкс
  Вместе с временем выборки равном 1 мкс общее время преобразования = 2.2 мкс
-------------------------------------------------------------------------------------------------------------*/
void ADC_converter_init(ADC_Type *ADC)
{
  ADC->CFG1 = 0
             + LSHIFT(1, 5) // ADIV.   Clock divide select. | 01 The divide ratio is 2 and the clock rate is (input clock)/2
             + LSHIFT(1, 2) // MODE.   Conversion mode selection. 01 12-bit conversion.
             + LSHIFT(0, 0) // ADICLK. 00 Alternate clock 1 (ADC_ALTCLK1). Другие источники не определены
  ;

  ADC->CFG2 = 0
             + LSHIFT(59, 0) // SMPLTS.  Sample Time Select. Время сэмплирования = 2 мкс
  ;

  // Регистр статуса и управления. Выбор типа тригера, управление функцией сравнения, разрешение DMA, выбор типа опоры
  // Выбор источника сигнала  тригера для ADC производится в регистре SIM_SOPT7.
  // Источниками сигнала триггера могут быть PDB 0-3, High speed comparator 0-3, PIT 0-3, FTM 0-3, RTC, LP Timer,

  ADC->SC2 = 0
            + LSHIFT(0, 7) // ADACT.   Read only. 1 Conversion in progress.
            + LSHIFT(1, 6) // ADTRG.   Conversion trigger select. 0 Software trigger selected.
            + LSHIFT(0, 5) // ACFE.    Compare function enable. 0 Compare function disabled.
            + LSHIFT(0, 4) // ACFGT.   Compare function greater than enable
            + LSHIFT(0, 3) // ACREN.   Compare function range enable
            + LSHIFT(1, 2) // DMAEN.   DMA enable
            + LSHIFT(0, 0) // REFSEL.  Voltage reference selection. 00 Default voltage reference pin pair (external pins VREFH and VREFL)
  ;
  // Регистр статуса и управления. Управление и статус калибровки, управление усреднением, управление непрерывным преобразованием
  ADC->SC3 = 0
            + LSHIFT(0, 7) // CAL.     CAL begins the calibration sequence when set.
            + LSHIFT(0, 3) // ADCO.    Continuous conversion enable
            + LSHIFT(0, 2) // AVGE.    Hardware Average Enable
            + LSHIFT(0, 0) // AVGS.    Hardware Average Select
  ;


  ;
  // Используем только управляющий регистр A
  // Результат управляемый этим регистром будет находиться в регистре ADC0->RA
  // Запись в данный регист либо прерывает текущее преобразование либо инициирует новое (если установлен флаг софтварныого тригера - ADTRG = 0)
  ADC->SC1[0] = 0
               + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
               + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
               + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                              //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;

}

/*-------------------------------------------------------------------------------------------------------------


   Устанавливаем ADICLK = 1, т.е. тактирование от  Bus clock он же Peripheral Clock поделенной на 2 = 30 Mhz

   Во время калибровки рекомендуется установить чатоту тактирования меньшей или равной 25 МГц
    и установить максимальную глубину усреднения
-------------------------------------------------------------------------------------------------------------*/
int32_t ADC_calibrating(ADC_Type *ADC)
{

  ADC->SC1[0] = 0
               + LSHIFT(0, 7) // COCO. Read only. 1 Conversion completed.
               + LSHIFT(0, 6) // AIEN. 1 Conversion complete interrupt enabled.
               + LSHIFT(0, 0) // ADCH. Input channel select. 11111 Module disabled.
                              //                             00000 When DIFF=0, DADP0 is selected as input; when DIFF=1, DAD0 is selected as input.
  ;
  // Устанавливить частоту ADC в два раза меньше чем максимально разрешенная. Выставляем 60/8 = 7.5 МГц
  ADC->CFG1 = 0
             + LSHIFT(3, 5) // ADIV.   Clock divide select. | 11 The divide ratio is 8 and the clock rate is (input clock)/8.
             + LSHIFT(1, 2) // MODE.   Conversion mode selection. 01 12-bit conversion.
             + LSHIFT(0, 0) // ADICLK. 00 Alternate clock 1 (ADC_ALTCLK1). Другие источники не определены
  ;

  // Устанавливаем максимальное время выборки для получения максимальной точности
  ADC->CFG2 = 0
             + LSHIFT(255, 0) // SMPLTS.  Sample Time Select. Время сэмплирования = 255/7500000 = 34 мкс
  ;
  ;
  // Регистр статуса и управления. Выбор типа тригера, управление функцией сравнения, разрешение DMA, выбор типа опоры
  ADC->SC2 = 0
            + LSHIFT(0, 7) // ADACT.   Read only. 1 Conversion in progress.
            + LSHIFT(0, 6) // ADTRG.   Conversion trigger select. 0 Software trigger selected.
            + LSHIFT(0, 5) // ACFE.    Compare function enable. 0 Compare function disabled.
            + LSHIFT(0, 4) // ACFGT.   Compare function greater than enable
            + LSHIFT(0, 3) // ACREN.   Compare function range enable
            + LSHIFT(0, 2) // DMAEN.   DMA enable
            + LSHIFT(0, 0) // REFSEL.  Voltage reference selection. 00 Default voltage reference pin pair (external pins VREFH and VREFL)
  ;

  ADC->CLPS = 0;
  ADC->CLP3 = 0;
  ADC->CLP2 = 0;
  ADC->CLP1 = 0;
  ADC->CLP0 = 0;
  ADC->CLPX = 0;
  ADC->CLP9 = 0;

  // Статруем процесс калибровки
  ADC->SC3 = 0
            + LSHIFT(1, 7) // CAL.     CAL begins the calibration sequence when set.
            + LSHIFT(0, 3) // ADCO.    Continuous conversion enable
            + LSHIFT(1, 2) // AVGE.    1 Hardware average function enabled.
            + LSHIFT(3, 0) // AVGS.    11 32 samples averaged.
  ;

  // Ожидать завершения калибровки
  while (ADC->SC3 & BIT(7));

  return RES_OK;
}

/*-------------------------------------------------------------------------------------------------------------
  Включить запуск заданного модуля ADC от аппаратного тригера , программный тригер перестает работать
-------------------------------------------------------------------------------------------------------------*/
void ADC_activate_hrdw_trig(ADC_Type *ADC)
{
  ADC->SC2 |= BIT(6); // ADTRG.   Conversion trigger select. 0 Software trigger selected.
}

/*-------------------------------------------------------------------------------------------------------------
  Включить запуск заданного модуля ADC от программного тригера , аппаратный тригер перестает работать
-------------------------------------------------------------------------------------------------------------*/
void ADC_activate_soft_trig(ADC_Type *ADC)
{
  ADC->SC2 &= ~BIT(6); // ADTRG.   Conversion trigger select. 0 Software trigger selected.
}

/*-------------------------------------------------------------------------------------------------------------
  Подготовка подсистемы  ADC к работе.
  Калибровка и старт работы всех конвертеров ADCР

  dma_en = 1 указывает на использование DMA при сканировании ADC
-------------------------------------------------------------------------------------------------------------*/
void ADCs_subsystem_init(void)
{
  g_res_bank = 0;
  g_adc_cnt  = 0;
  ADC_switch_on_all();
  ADC_calibrating(ADC0);        // Проводим процедуру калибровки модулей ADC
  ADC_calibrating(ADC1);        // Проводим процедуру калибровки модулей ADC
  ADC_calibrating(ADC2);        // Проводим процедуру калибровки модулей ADC
  ADC_converter_init(ADC0);
  ADC_converter_init(ADC1);
  ADC_converter_init(ADC2);

  // Назначаем триггер для ADC.
  // Сигнал триггер будет приходить на все 3 ADC от модуля TRGMUX.
  // Пре-триггер не используем
  SIM->ADCOPT  = 0
                + LSHIFT(1, 20) // ADC2PRETRGSEL | ADC2 pre-trigger source select     | 01 TRGMUX pre-trigger
                + LSHIFT(0, 17) // ADC2SWPRETRG  | ADC2 software pre-trigger sources  | 000 software pre-trigger disabled
                + LSHIFT(1, 16) // ADC2TRGSEL    | ADC2 trigger source select         | 1 TRGMUX output
                + LSHIFT(1, 12) // ADC1PRETRGSEL | ADC1 pre-trigger source select     | 01 TRGMUX pre-trigger
                + LSHIFT(0,  9) // ADC1SWPRETRG  | ADC1 software pre-trigger sources  | 000 software pre-trigger disabled
                + LSHIFT(1,  8) // ADC1TRGSEL    | ADC1 trigger source select         | 1 TRGMUX output
                + LSHIFT(1,  4) // ADC0PRETRGSEL | ADC0 pre-trigger source select     | 01 TRGMUX pre-trigger
                + LSHIFT(0,  1) // ADC0SWPRETRG  | ADC0 software pre-trigger sources  | 000 software pre-trigger disabled
                + LSHIFT(1,  0) // ADC0TRGSEL    | ADC0 trigger source select         | 1 TRGMUX output
  ;
  // Конфигурируем направление на ADC сигнала триггера от PIT0
  TRGMUX0->TRGCFG[TRGMUX_ADC0_INDEX] = 0
                                      + LSHIFT(0x1F, 24) // SEL3 | Description
                                      + LSHIFT(0x1F, 16) // SEL2 | Description
                                      + LSHIFT(0x1F, 8)  // SEL1 | Description
                                      + LSHIFT(0x07, 0)  // SEL0 | 000_0111 - (0x07) LPIT_CH0 is selected
  ;

  TRGMUX0->TRGCFG[TRGMUX_ADC1_INDEX]  = 0
                                       + LSHIFT(0x1F, 24) // SEL3 | Description
                                       + LSHIFT(0x1F, 16) // SEL2 | Description
                                       + LSHIFT(0x1F, 8)  // SEL1 | Description
                                       + LSHIFT(0x07, 0)  // SEL0 | 000_0111 - (0x07) LPIT_CH0 is selected
  ;

  TRGMUX0->TRGCFG[TRGMUX_ADC2_INDEX]  = 0
                                       + LSHIFT(0x1F, 24) // SEL3 | Description
                                       + LSHIFT(0x1F, 16) // SEL2 | Description
                                       + LSHIFT(0x1F, 8)  // SEL1 | Description
                                       + LSHIFT(0x07, 0)  // SEL0 | 000_0111 - (0x07) LPIT_CH0 is selected
  ;
  ADC_activate_hrdw_trig(ADC0); // Запускаем работу ADC от аппаратного триггера
  ADC_activate_hrdw_trig(ADC1); // Запускаем работу ADC от аппаратного триггера
  ADC_activate_hrdw_trig(ADC2); // Запускаем работу ADC от аппаратного триггера

  Init_ADC_by_DMA();
  PIT_init_channel(ADC_PIT_TIMER, ADC_ONE_SAMPL_PERIOD , PIT0_PRIO, 0, 0); // Запускаем работу АЦП по периодическому сигналу
}



/*-------------------------------------------------------------------------------------------------------------
  Прерывание после окончания мажорного цикла канала  DMA
  Период повторения - 32 мкс

  С максимальной оптимизацией время выполнения данной процедуры c установкой флагов  10.57 мкс
  без установки флагов 6.93 мкс
-------------------------------------------------------------------------------------------------------------*/
void DMA0_IRQHandler(void *user_isr_ptr)
{
  //ITM_EVENT8(1,1);
  // Время выполнения этого участка  1.39 мкс

  DMA0->INT = BIT(DMA_ADC0_RES_CH); // Сбрасываем флаг прерываний  канала

  uint32_t block_sz = ADC_SCAN_SZ * 2; // Размер блока данных результата ADC.
  uint32_t addr = (uint32_t)&g_ADC_raw_results;

  memcpy((void *)addr,  (uint16_t *)adc0_results[g_res_bank], block_sz);
  addr += block_sz;
  memcpy((void *)addr,  (uint16_t *)adc1_results[g_res_bank], block_sz);
  addr += block_sz;
  memcpy((void *)addr,  (uint16_t *)adc2_results[g_res_bank], block_sz);

  g_res_bank ^= 1;

  //ITM_EVENT8(1,2);
  // Время выполнения этого участка  1.66 мкс

  s_VREF_1_65V = MedianFilter_3uint16(g_ADC_raw_results.smpl_VREF_1_65V,  &mfltr_VREF_1_65V );
  s_ACCUM_ID_I = MedianFilter_3uint16(g_ADC_raw_results.smpl_ACCUM_ID_I,  &mfltr_ACCUM_ID_I ) - s_VREF_1_65V;
  s_ACCUM_I    = MedianFilter_3uint16(g_ADC_raw_results.smpl_ACCUM_I   ,  &mfltr_ACCUM_I    ) - s_VREF_1_65V;
  s_PSRC_I     = MedianFilter_3uint16(g_ADC_raw_results.smpl_PSRC_I    ,  &mfltr_PSRC_I     ) - s_VREF_1_65V;
  s_INVERT_I   = MedianFilter_3uint16(g_ADC_raw_results.smpl_INVERT_I  ,  &mfltr_INVERT_I   ) - s_VREF_1_65V;
  s_ACCUM_V    = MedianFilter_3uint16(g_ADC_raw_results.smpl_ACCUM_V   ,  &mfltr_ACCUM_V    );
  s_PSRC_V     = MedianFilter_3uint16(g_ADC_raw_results.smpl_PSRC_V    ,  &mfltr_PSRC_V     );
  s_CPU_temper = MedianFilter_3uint16(g_ADC_raw_results.smpl_Tempe1    ,  &mfltr_CPU_temper );

  //ITM_EVENT8(1,3);
  // Время выполнения этого участка  2.73 мкс

  sys.fp_smpl_accum_id_i              = LPF_But_100_31250_step(&lpflt_ACCUM_ID_I  , s_ACCUM_ID_I);
  sys.fp_smpl_accum_i                 = LPF_But_100_31250_step(&lpflt_ACCUM_I     , s_ACCUM_I);
  sys.fp_smpl_psrc_i                  = LPF_But_100_31250_step(&lpflt_PSRC_I      , s_PSRC_I);
  sys.fp_smpl_invert_i                = LPF_But_100_31250_step(&lpflt_INVERT_I    , s_INVERT_I);
  sys.fp_smpl_invert_v                = LPF_But_100_31250_step(&lpflt_INVERT_V    , g_ADC_raw_results.smpl_INVERT_V);
  sys.fp_smpl_sys_v                   = LPF_But_100_31250_step(&lpflt_PWR24_V     , g_ADC_raw_results.smpl_PWR24_V);
  sys.fp_smpl_psrc_v                  = LPF_But_100_31250_step(&lpflt_PSRC_V      , g_ADC_raw_results.smpl_PSRC_V);
  sys.fp_smpl_accum_v                 = LPF_But_100_31250_step(&lpflt_ACCUM_V     , g_ADC_raw_results.smpl_ACCUM_V);
  sys.fp_smpl_cpu_temper              = s_CPU_temper;

  //ITM_EVENT8(1,4);
  // Время выполнения этого участка  0.13 мкс

  g_ADC_results.smpl_MAINS_PATH_STATE = g_ADC_raw_results.smpl_MAINS_PATH_STATE;
  g_ADC_results.smpl_INV_PATH_STATE   = g_ADC_raw_results.smpl_INV_PATH_STATE  ;

  //ITM_EVENT8(1,5);
  // Время выполнения этого участка  0.38 мкс

  //Emergency_observer();

  //ITM_EVENT8(1,6);
  // Время выполнения этого участка  2.73 мкс

  FMSTR_Recorder(0); // Вызываем функцию записи сигнала для инструмента FreeMaster

  //ITM_EVENT8(1,7);
  // Время выполнения этого участка  3.43 мкс

  g_adc_cnt++;
  if (g_adc_cnt >= ADC_EVENT_PERIOD_DIV)
  {
    g_adc_cnt = 0;
    App_set_events(FLAG_ADC_DONE);
  }

  //ITM_EVENT8(1,0);

}


/*-------------------------------------------------------------------------------------------------------------
  Инициализируем DMA для автоматической выборки из заданных ADC каналов
  По мотивам Document Number: AN4590 "Using DMA to Emulate ADC Flexible Scan Mode on Kinetis K Series"

  Здесь используется механизм связанного вызова минорного цикла одного канала DMA по окончании минорного цикла другого канала DMA


  n     -  номер конфигурационной записи в массиве init_ADC_DMA_cbl
  enint - флаг разрешения прерывания от данной конфигурации
-------------------------------------------------------------------------------------------------------------*/
void Config_DMA_for_ADC(uint8_t n, uint8_t enint)
{
  ADC_Type *ADC    = init_ADC_DMA_cbl[n].ADC;

  uint8_t            res_ch = init_ADC_DMA_cbl[n].results_channel; // Канал res_ch DMA (более приоритетный) считывает данные из ADC
  uint8_t            cfg_ch = init_ADC_DMA_cbl[n].config_channel;  // Канал cfg_ch DMA (менее приоритетный) загружает новым значением регистр ADC->SC1[0]


  // Конфигурируем первую выборку из ADC по корректному номеру канала согласно заданной конфигурации
  ADC->SC1[0] = init_ADC_DMA_cbl[n].config_vals[ ADC_SCAN_SZ - 1 ];

  // Конфигурируем более приоритетный канал пересылки данных из ADC
  DMA0->TCD[res_ch].SADDR = (uint32_t)&ADC->R[0];    // Источник - регистр данных ADC
  DMA0->TCD[res_ch].SOFF = 0;                        // Адрес источника не изменяется после чтения
  DMA0->TCD[res_ch].SLAST = 0;                       // Не корректируем адрес источника после завершения всего цикла DMA (окончания мажорного цикла)
  DMA0->TCD[res_ch].DADDR = (uint32_t)init_ADC_DMA_cbl[n].results_vals; // Адрес приемника данных из ADC - массив отсчетов
  DMA0->TCD[res_ch].DOFF = 2;                        // После каждой записи смещаем указатель приемника на два байта
  DMA0->TCD[res_ch].DLAST_SGA = 0;                   // Здесь не корректируем адреса приемника после мажорного цикла. Для коррекции используем поле DMOD в регистре ATTR
  // DMA->TCD[res_ch].DLAST_SGA = (uint32_t)(-ADC_SCAN_SZ*2);  // Здесь корректируем адреса приемника после мажорного цикла.
  DMA0->TCD[res_ch].NBYTES_MLNO = 2;                 // Количество байт пересылаемых за один запрос DMA (в минорном цикле)
  DMA0->TCD[res_ch].BITER_ELINKNO = 0
                                   + LSHIFT(1, 15)           // ELINK  | Включаем линковку к минорному циклу другого канала
                                   + LSHIFT(cfg_ch, 9)       // LINKCH | Линкуемся к каналу cfg_ch
                                   + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[res_ch].CITER_ELINKNO = 0
                                   + LSHIFT(1, 15)           // ELINK  | Включаем линковку к минорному циклу другого канала
                                   + LSHIFT(cfg_ch, 9)       // LINKCH | Линкуемся к каналу cfg_ch
                                   + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[res_ch].ATTR = 0
                          + LSHIFT(0, 11) // SMOD  | Модуль адреса источника не используем
                          + LSHIFT(1, 8)  // SSIZE | 16-и битная пересылка из источника
                          + LSHIFT(ADC_DMA_DEST_MODULO, 3)  // DMOD  | Модуль адреса приемника
                                                            //  + LSHIFT(0, 3)  // DMOD  | Модуль адреса приемника
                          + LSHIFT(1, 0)  // DSIZE | 16-и битная пересылка в приемник
  ;
  DMA0->TCD[res_ch].CSR = 0
                         + LSHIFT(3, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
                         + LSHIFT(cfg_ch, 8)  // MAJORLINKCH | Поскольку минорный линк не срабатывает на последней минорной пересылке, то линк надо настроить и для мажорного цикла
                         + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
                         + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
                         + LSHIFT(1, 5)  // MAJORELINK  | Линкуемся к каналу cfg_ch после завершения мажорного цикла
                         + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
                         + LSHIFT(0, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
                         + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
                         + LSHIFT(enint, 1)  // INTMAJOR    | Разрешаем прерывание после мажорного цикла
                         + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;

  // Конфигурируем менее приоритетный канала пересылки настройки в ADC
  DMA0->TCD[cfg_ch].SADDR = (uint32_t)init_ADC_DMA_cbl[n].config_vals;     // Источник - массив настроек для ADC
  DMA0->TCD[cfg_ch].SOFF = 1;                        // Адрес источника после прочтения смещается на 1 байт вперед
  DMA0->TCD[cfg_ch].SLAST = 0;                       // Не корректируем адрес источника. Используем модуль адреса для этого
  DMA0->TCD[cfg_ch].DADDR = (uint32_t)&ADC->SC1[0];  // Адрес приемника данных - регистр управления ADC
  DMA0->TCD[cfg_ch].DOFF = 0;                        // Указатель приемника неменяем после записи
  DMA0->TCD[cfg_ch].DLAST_SGA = 0;                   // Коррекцию адреса приемника не производим после окончания всей цепочки сканирования (окончания мажорного цикла)
  DMA0->TCD[cfg_ch].NBYTES_MLNO = 1;                 // Количество байт пересылаемых за один запрос DMA (в минорном цикле)
  DMA0->TCD[cfg_ch].BITER_ELINKNO = 0
                                   + LSHIFT(0, 15)           // ELINK  | Линковка выключена
                                   + LSHIFT(0, 9)            // LINKCH |
                                   + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[cfg_ch].CITER_ELINKNO = 0
                                   + LSHIFT(0, 15)           // ELINK  | Линковка выключена
                                   + LSHIFT(0, 9)            // LINKCH |
                                   + LSHIFT(ADC_SCAN_SZ, 0)  // BITER  | Количество итераций в мажорном цикле
  ;
  DMA0->TCD[cfg_ch].ATTR = 0
                          + LSHIFT(ADC_DMA_SRC_MODULO, 11) // SMOD  | Модуль адреса источника используется для возврата в начало массива настроек
                          + LSHIFT(0, 8)  // SSIZE | 8-и битная пересылка из источника
                          + LSHIFT(0, 3)  // DMOD  | Модуль адреса приемника не используем
                          + LSHIFT(0, 0)  // DSIZE | 8-и битная пересылка в приемник
  ;
  DMA0->TCD[cfg_ch].CSR = 0
                         + LSHIFT(3, 14) // BWC         | Bandwidth Control. 00 No eDMA engine stalls
                         + LSHIFT(0, 8)  // MAJORLINKCH | Link Channel Number
                         + LSHIFT(0, 7)  // DONE        | This flag indicates the eDMA has completed the major loop.
                         + LSHIFT(0, 6)  // ACTIVE      | This flag signals the channel is currently in execution
                         + LSHIFT(0, 5)  // MAJORELINK  | Линкуемся к каналу 0  после завершения мажорного цикла
                         + LSHIFT(0, 4)  // ESG         | Enable Scatter/Gather Processing
                         + LSHIFT(0, 3)  // DREQ        | Disable Request. If this flag is set, the eDMA hardware automatically clears the corresponding ERQ bit when the current major iteration count reaches zero.
                         + LSHIFT(0, 2)  // INTHALF     | Enable an interrupt when major counter is half complete
                         + LSHIFT(0, 1)  // INTMAJOR    | Прерывания не используем
                         + LSHIFT(0, 0)  // START       | Channel Start. If this flag is set, the channel is requesting service.
  ;
  DMAMUX->CHCFG[res_ch] = init_ADC_DMA_cbl[n].req_src + BIT(7); // Через мультиплексор связываем сигнал от внешней периферии (здесь от канала ADC) с входом выбранного канала DMA
                                                                // BIT(7) означает DMA Channel Enable
  DMA0->SERQ = res_ch;                                           // Разрешаем запросы от внешней периферии для канала DMA с номером res_ch
}


/*-------------------------------------------------------------------------------------------------------------
  Инициализируем DMA для автоматической выборки из заданных ADC каналов

  Назначаем прерывание только от одного канала DMA
  Поскольку он самый низкоприоритетный, то должен сработать позже всех когда все данные из всех ADC уже будут в памяти
-------------------------------------------------------------------------------------------------------------*/
void Init_ADC_by_DMA(void)
{
  Config_DMA_for_ADC(2, 0);
  Config_DMA_for_ADC(1, 0);
  Config_DMA_for_ADC(0, 1);

  NVIC_SetPriority(DMA0_IRQn, DMA_ADC_PRIO);
  NVIC_EnableIRQ(DMA0_IRQn);
}

