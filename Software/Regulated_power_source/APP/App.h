
#define MAX_CPU_TEMPERATURE                    (65.0f)


#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include "arm_itm.h"

#include "fsl_common.h"
#include "fsl_smc.h"
#include "fsl_device_registers.h"
#include "fsl_lpi2c.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_lpi2c_edma.h"

#include "tx_api.h"

#undef   PARAMS_TABLES_ENABLED
#include "BKMAN1_2_Params.h"
#include "BKMAN1_2_Params_dyn_tables.h"
#include "BKMAN1_2_PERIPHERIAL.h"

#include  "Inputs.h"
#include  "Analog_inputs.h"
#include  "Outputs.h"
#include  "Mem_man.h"
#include  "MonitorVT100.h"
#include  "BKMAN1_2_monitor.h"
#include  "Monitor_serial_drv.h"
#include  "Time_utils.h"
#include  "Params_editor.h"
#include  "NV_store.h"
#include  "CRC_utils.h"
#include  "App_utils.h"
#include  "FreeMaster_task.h"
#include  "Filters.h"
#include  "OLED_control.h"
#include  "DCDC_DAC80501.h"
#include  "ACS37800.h"
#include  "logger.h"
#include  "IDLE_task.h"


#include "..\..\..\common\CAN_bus_defs.h"


#define  BIT(n) (1u << n)
#define  LSHIFT(v,n) (((unsigned int)(v) << n))


#define APP_MEM_POOL_SIZE                  (0x8000-0x100)

#define THREAD_MAIN_STACK_SIZE             1024
#define VT100_TASK_STACK_SIZE              2048
#define THREAD_FREEMASTER_STACK_SIZE       1024
#define THREAD_IDLE_STACK_SIZE             512
#define THREAD_POWER_CONTROL_STACK_SIZE    1024
#define THREAD_CHARGER_STACK_SIZE          1024
#define THREAD_CAN_STACK_SIZE              512


#define THREAD_MAIN_PRIORITY               10
#define THREAD_CHARGER_PRIORITY            10
#define THREAD_POWER_CONTROL_PRIORITY      11
#define THREAD_CAN_PRIORITY                12
#define THREAD_FREEMASTER_PRIORITY         13
#define VT100_TASK_PRIO                    14
#define THREAD_IDLE_PRIORITY               20


// Системная частота (Core clock, System clock) 120 МГц
// Частота системной шины (Bus Clock) 60 МГц
#define PLLDIV1_CLK                        (120000000ul)                  // Частота сигнала PLLDIV1_CLK
#define PLLDIV2_CLK                        (60000000ul)                   // Частота сигнала PLLDIV2_CLK
#define MIN_PWM_PULSE                      ((uint32_t)(0.0000001f*PLLDIV1_CLK))

// Все LPUART тактируются от частоты 60 МГц
#define UART_UNIT_CLOCK                    (60000000ul)  // Чатота тактирования UART
// Все LPSPI тактируются от частоты 60 МГц
#define SPI_UNIT_CLOCK                     (60000000ul)  // Чатота тактирования SPI

#define CAN_UNIT_CLOCK                     (60000000ul)  // Частота тактирования CAN
#define CAN0_BITRATE                       (555555ul)

#define ADC_SCAN_SZ                        8ul    // Количество сканируемых каналов
#define ADC_ONE_SAMPL_PERIOD               4ul    // Время между соседними выборками ADC
#define ADC_FULL_SAMPL_PERIOD              ADC_ONE_SAMPL_PERIOD*ADC_SCAN_SZ  // Период выборки цифро-анлогового преобразователя (ADC) в мкс. за это время происходит выборка всех переменных включенных в структуру
#define ADC_EVENT_PERIOD_DIV               32ul   // Делитель частоты приема результатов ADC. С итоговым периодом генерируется событие задаче управления
#define ADC_EVENT_PERIOD_US                ADC_FULL_SAMPL_PERIOD*ADC_EVENT_PERIOD_DIV
#define OUTPUT_TICK_PERIOD                 1000   // Период событий FLAG_SCAN_TICK используемых для формирования выходных сигналов в мкс
#define REF_TIME_INTERVAL                  100    // Задержка в мс для замера калибровочного интервала времени


#define MAIN_CYCLE_PERIOD_US                ADC_EVENT_PERIOD_US
#define TIME_TICK_PERIOD                   (1000000ul) // Период счетчика времени в микросекундах


#define  FREEMASTER_UART_SPEED             256000  // Скорость работы UART в режиме FreeMaster

#define FLASH_DEFAULT_EE_RST_FLAG          1                            // Дефолтное действие после сброса - FlexRAM загружается данными EEPROM из Backup Flash
#define FLASH_DEFAULT_EE_SIZE              4096                         // Дефолтный размер EEPROM
#define FLASH_DEFAULT_EE_BACKUP_SZ         64                           // Дефолтный размер Backup Flash для EEPROM

#define FLASH_SET_DEFAULT_EE_RST_FLAG      FLEXRAM_IS_LOADED_ON_RESET   // Коды команд передаваемые в регистры для установки дефолтных параметров EEPROM
#define FLASH_SET_DEFAULT_EE_SIZE          EE_SIZE_CODE__4096           //
#define FLASH_SET_DEFAULT_EE_BACKUP_SZ     DF_PART_CODE__DF00_EE64      //


#define EEPROM_ADDR                        0x14000000
#define WVAR_OFFSET_IN_EEPROM              0
#define MAX_WVAR_SIZE                      (1024-2)
#define WVAR_CRC_OFFSET_IN_EEPROM          (1024-2)



#define MIN_MAINS_VOLTAGE                  (190.0f)   // Минимальное сетевое напряжение при ктором выставляется флаг отсутствия сетевого напряжения
#define MIN_OUTPUT_VOLTAGE                 (200.0f)   // Минимальное выходное напряжение при ктором выставляется флаг отсутствия выходного напряжения

// -----------------------------------------------------------------------------
//  Статическое конфигурирование каналов DMA и DMA MUX для работы с ADC

#define DMA_ADC0_RES_CH       0                      // Занятый канал DMA для обслуживания ADC
#define DMA_ADC1_RES_CH       1                      // Занятый канал DMA для обслуживания ADC
#define DMA_ADC2_RES_CH       2                      // Занятый канал DMA для обслуживания ADC
#define DMA_ADC0_CFG_CH       3                      // Занятый канал DMA для обслуживания ADC
#define DMA_ADC1_CFG_CH       4                      // Занятый канал DMA для обслуживания ADC
#define DMA_ADC2_CFG_CH       5                      // Занятый канал DMA для обслуживания ADC
#define DMA_ADC_DMUX_PTR      DMAMUX                 // Указатель на модуль DMUX который используется для передачи сигналов от ADC к DMA
#define DMA_ADC0_DMUX_SRC     DMUX_SRC_ADC0          // Входы DMUX используемые для выбранного ADC
#define DMA_ADC1_DMUX_SRC     DMUX_SRC_ADC1          // Входы DMUX используемые для выбранного ADC
#define DMA_ADC2_DMUX_SRC     DMUX_SRC_ADC2          // Входы DMUX используемые для выбранного ADC
#define DMA_ADC_INT_NUM       DMA0_IRQn               // Номер вектора прерывания используемый в DMA для обслуживания ADC

#define DMA_ADC_PRIO          1
#define PIT0_PRIO             1
#define PIT1_PRIO             2
#define TIME_PIT_PRIO         2
#define CAN_PRIO              2

#define OS_PRIORITY_LEVEL      14
#define DISABLE_OS_PRI_LEV    (OS_PRIORITY_LEVEL << (8 - __NVIC_PRIO_BITS))  // Маска приоритета в регистре BASEPRI запрещающая прерывания PendSV и  SysTick
#define ENABLE_OS_PRI_LEV     0   // Маска приоритета в регистре BASEPRI разрешающая прерывания с любыми приоритетами

#define ADC_PIT_TIMER         0   // Канал модуля PIT для выполения выборок ADC
#define SCAN_TASK_PIT_TIMER   1   // Канал модуля PIT для генератор события FLAG_PERIODIC_TICK вызывающего выполнение периодических процедур (установка выходов, чтение измерителей мощности, управление и т.д.)
#define TIME_PIT_TIMER        2   // Канал модуля PIT для счета времени

// Определения статуса записей для логгера приложения
#define   SEVERITY_RED             0
#define   SEVERITY_GREEN           1
#define   SEVERITY_YELLOW          2
#define   SEVERITY_DEFAULT         4

#define   RES_OK                   0
#define   RES_ERROR                1


#define FMSTR_SMPS_ADC_ISR                 0       // Источником сэмплов для рекордера FreeMaster является процедура прерывания ADC

#define FLAG_ADC_DONE                      BIT( 0 ) // Флаг собьтия окончания цикла измерений многоканального АЦП
#define FLAG_PERIODIC_TICK                 BIT( 1 ) // Флаг цикла сканирования
#define FLAG_POWER_CONTROL_TICK            BIT( 2 ) // Флаг цикла задачи power control
#define FLAG_ACCUM_SWITCHED_OFF            BIT( 3 ) // Флаг слишком низкого заряда аккумулятора и его отключения
#define FLAG_ACCUM_FULLY_CHARGED           BIT( 4 ) // Флаг полного зарада аккумулятора

#define CH_DSCH_OFF               0 // тест выключен
#define CH_DSCH_DISCHARGE         1 // идет разряд
#define CH_DSCH_IDLE1             2 // пауза после разряда
#define CH_DSCH_CHARGE_START      3 // задержка ожидания старта заряда
#define CH_DSCH_CHARGE            4 // идет заряд
#define CH_DSCH_IDLE2             5 // пауза после заряда


#define  DISABLE_OS_INTERRUPTS __set_BASEPRI(DISABLE_OS_PRI_LEV)
#define  ENABLE_OS_INTERRUPTS  __set_BASEPRI(ENABLE_OS_PRI_LEV)

typedef struct
{
    // Отфильтованые значения сэмплов соответствующих сигналов взятых с АЦП
    volatile float      fp_smpl_accum_id_i;
    volatile float      fp_smpl_accum_i;
    volatile float      fp_smpl_psrc_i;
    volatile float      fp_smpl_invert_i;
    volatile float      fp_smpl_invert_v;
    volatile float      fp_smpl_sys_v;
    volatile float      fp_smpl_psrc_v;
    volatile float      fp_smpl_accum_v;
    volatile float      fp_smpl_cpu_temper;


    volatile float      accum_id_i;             // Ток поступающий от аккумулятора через идеального диод в систему
    volatile float      accum_i;                // Ток втекающий в систему или выткающий из системы в аккумулятор (без учета тока в инвертер). Со знаком минус ток заряда аккмулятора, со знаком плюс - ток разряда
    volatile float      accum_sum_i;            // Суммарный ток аккумулятора. Вытекающий с минусом. Втекающмй в аккумулятор с плюсом
    volatile float      accum_charge;           // Уровень заряда аккумулятора, Ah
    volatile float      psrc_i;                 // Ток источника питания 24V. Со знаком плюс вытекающий из источника, со знаком минус - втекающий в источник
    volatile float      invert_i;               // Ток питания инвертера. Со знаком плюс втекающий в инвертер, со знаком минус - вытекающий из инвертера
    volatile float      invert_v;               // Наряжение на входе инвертера
    volatile float      sys_v;                  // Напряжение поступающее в систему
    volatile uint8_t    mains_path_state;       // Флаг включенного реле между сетевым напряжением и выходом 220V
    volatile uint8_t    inv_path_state;         // Флаг включенного реле между инвертером и выходом 220V
    volatile float      psrc_v;                 // Напряжение на источнике питания 24V
    volatile float      accum_v;                // Напряжение на аккумуляторе
    volatile float      accum_charge_power;     // Мощность заряда акккумулятора
    volatile float      accum_discharge_power;  // Мощность разряда акккумулятора
    volatile float      pwr_src_power;          // Мощность выдаваемая источником питания
    volatile float      chaging_pwr_loss;       // Потери в плате при заряде аккумулятора при отсутствии питания внешней системы
    volatile float      charging_efficiency;    // КПД платы при заряде аккумулятора при отсутствии питания внешней системы
    volatile float      cpu_temper;
    volatile uint8_t    cpu_overheat;
    volatile int8_t     pwr_src_condition;      // 1 - внешний источник питания готов, 0 - внешний источник питания не готов к началу процесса заряда аккумулятора

    volatile uint16_t   DCDC_DAC_code;

    volatile uint8_t    dcdc_dac_flag_fail;     // Ошибка ЦАП преобразователя DCDC

    volatile uint8_t    dcdc_dac_error_flag;

    volatile uint8_t    mains_voltage_ok;
    volatile uint8_t    output_voltage_ok;

    volatile float      vrms_u15;
    volatile float      vrms_u17;
    volatile float      irms_u15;
    volatile float      irms_u17;
    volatile float      out220v_power;

} T_sys_state;


typedef __packed struct
{
  uint32_t charging_active   : 1;  // Флаг работы зарядника
  uint32_t accum_charge      : 7;  // Уровень заряда аккумулятора в процентах
  uint32_t mains_voltage_ok  : 1;  // Флаг присутствия сетевого напряжения
  uint32_t output_voltage_ok : 1;  // Флаг присутствия выходного напряжения 220V
  uint32_t iverter_is_on     : 1;  // Флаг включенного инвертера
  uint32_t iverter_relay     : 1;  // Флаг включенного реле инвертер - выход
  uint32_t mains_relay       : 1;  // Флаг включенного реле сеть 220 - выход
  uint32_t iverter_power     : 12; // Мощность выдаваемая инвертером  в ваттах
} T_sys_state_packet;


#ifdef _MAIN_GLOBALS_

volatile uint32_t             system_sec_counter;
WVAR_TYPE                     wvar;
uint64_t                      ref_time;             // Калибровочная константа предназначенная для измерения нагрузки микропроцессора
volatile uint32_t             g_cpu_usage;            // Процент загрузки процессора
uint8_t                       eeprom_ok;
uint8_t                       nv_store_ok;
uint8_t                       request_save_to_NV_mem;
T_sys_state                   sys;

#else

extern volatile uint32_t      system_sec_counter;
extern WVAR_TYPE              wvar;
extern uint64_t               ref_time;
extern volatile uint32_t      g_cpu_usage;
extern uint8_t                eeprom_ok;
extern uint8_t                nv_store_ok;
extern uint8_t                request_save_to_NV_mem;
extern T_sys_state            sys;


#endif


extern const   T_work_params dwvar[];


void           App_set_events(uint32_t flags);
void           Signals_conditioning(void);
void           Pack_sytem_state_to_can_msg(uint8_t* out_buf);
void           Execute_can_command(uint8_t* cmd_buf);



