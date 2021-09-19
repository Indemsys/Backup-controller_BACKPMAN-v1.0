#ifndef BCKUPC10_ANALOG_INPUTS_H
#define BCKUPC10_ANALOG_INPUTS_H

#define ADC_DMA_DEST_MODULO       5    // Количество младших бит адреса разрешенных к изменению в адресе приемника результатов ADC. 5 бит соответствует адресации  2-х буферов по 8 16-и битных слов - 32 байта
#define ADC_DMA_SRC_MODULO        3    // Количество младших бит адреса разрешенных к изменению в адресе источника настроек ADC. Для 8-и регистров это будет 3-и бита


#define ADC_PREC                   (4095.0f)
#define VREF                       (3.3f)
#define AIN_SCALE                  (VREF/ADC_PREC)


#define ACCUM_ID_I_SCALE           ((AIN_SCALE / 50.0f ) / 0.002f)
#define ACCUM_I_SCALE              ((AIN_SCALE / 20.0f)  / 0.005f)
#define PSRC_I_SCALE               ((AIN_SCALE / 50.0f ) / 0.002f)
#define INVERT_I_SCALE             ((AIN_SCALE / 50.0f ) / 0.001f)
#define INVERT_V_SCALE             (((AIN_SCALE * ( 100.0f + 4.7f )) / 4.7f) * 1.006552f)
#define SYS_V_SCALE                (((AIN_SCALE * ( 100.0f + 4.7f )) / 4.7f) * 1.004972f)
#define MAINS_PATH_STATE_SCALE     ((AIN_SCALE * ( 100.0f + 4.7f )) / 4.7f)
#define INV_PATH_STATE_SCALE       ((AIN_SCALE * ( 100.0f + 4.7f )) / 4.7f)
#define PSRC_V_SCALE               (((AIN_SCALE * ( 100.0f + 4.7f )) / 4.7f) * 1.007942f)
#define ACCUM_V_SCALE              (((AIN_SCALE * 1000.0f ) / 82.0f) * 1.005685f)

#define EXT_AIN_SCALE              (4.7f/104.7f)

#define VTEMP_S                    (1.564f)
#define VTEMP25                    (740.5f)

#define CPU_TEMP(x)                (25.0f - ((x*AIN_SCALE*1000.0f-VTEMP25)/VTEMP_S))


typedef struct
{
    uint16_t smpl_ACCUM_ID_I        ;  // ACCUM_ID_I         - Ток аккумулятора через идеальный диод в систему
    uint16_t smpl_ACCUM_I           ;  // ACCUM_I            - Ток от аккумулятора через идеадьный диод в систему и ток из зарядника в аккумулятор
    uint16_t smpl_PSRC_I            ;  // PSRC_I             - Ток источника питания
    uint16_t smpl_INVERT_I          ;  // INVERT_I           - Ток в инвертер от аккумулятора
    uint16_t smpl_INVERT_V          ;  // INVERT_V           - Напряжение на входе инвертера
    uint16_t smpl_rsv1              ;  //
    uint16_t smpl_rsv2              ;  //
    uint16_t smpl_rsv3              ;  //

    uint16_t smpl_PWR24_V           ;  // PWR24_V            - Напряжение уходящее в систему
    uint16_t smpl_MAINS_PATH_STATE  ;  // MAINS_PATH_STATE   - Сигнал с обмотки реле включающего инвертер на выход 220 платы
    uint16_t smpl_INV_PATH_STATE    ;  // INV_PATH_STATE     - Сигнал с обмотки реле включающего сеть на выход 220 платы
    uint16_t smpl_PSRC_V            ;  // PSRC_V             - Напряжение на источнике питания
    uint16_t smpl_rsv4              ;  //
    uint16_t smpl_rsv5              ;  //
    uint16_t smpl_rsv6              ;  //
    uint16_t smpl_rsv7              ;  //

    uint16_t smpl_VREF_1_65V        ;  // VREF_1_65V         - Напряжение с референсного источника
    uint16_t smpl_ACCUM_V           ;  // ACCUM_V            - Напряжение на аккумуляторе
    uint16_t smpl_Tempe1            ;  // Tempe1             - Внутренний температурный сенсор
    uint16_t smpl_Tempe2            ;  // Tempe2             - Внутренний температурный сенсор
    uint16_t smpl_Tempe3            ;  // Tempe3             - Внутренний температурный сенсор
    uint16_t smpl_VREFH1            ;  // VREFH1             - Верхний уровень опорного напряжения
    uint16_t smpl_VREFH2            ;  // VREFH2             - Верхний уровень опорного напряжения
    uint16_t smpl_VREFL1            ;  // VREFL1             - Нижний уровень опорного напряжения
}
T_ADC_raw_results;


typedef struct
{
  int16_t  smpl_ACCUM_ID_I        ;  // ACCUM_ID_I         - Знаковое целое. Ток аккумулятора через идеальный диод в систему (с плюсом) и из системы (с минусом)
  int16_t  smpl_ACCUM_I           ;  // ACCUM_I            - Знаковое целое. Ток от аккумулятора в систему (с плюсом) и из системв (с минусом). Не считая тока в инвертер
  int16_t  smpl_PSRC_I            ;  // PSRC_I             - Знаковое целое. Ток источника питания в систему (с плюсом) и из системы (с минусом)
  int16_t  smpl_INVERT_I          ;  // INVERT_I           - Знаковое целое. Ток в инвертер от аккумулятора (с плюсом) и обратно (с минусом)
  uint16_t smpl_INVERT_V          ;  // INVERT_V           - Напряжение на входе инвертера
  uint16_t smpl_PWR24_V           ;  // PWR24_V            - Напряжение уходящее в систему
  uint16_t smpl_MAINS_PATH_STATE  ;  // MAINS_PATH_STATE   - Сигнал с обмотки реле включающего инвертер на выход 220 платы
  uint16_t smpl_INV_PATH_STATE    ;  // INV_PATH_STATE     - Сигнал с обмотки реле включающего сеть на выход 220 платы
  uint16_t smpl_PSRC_V            ;  // PSRC_V             - Напряжение на источнике питания
  uint16_t smpl_VREF_1_65V        ;  // VREF_1_65V         - Напряжение с референсного источника
  uint16_t smpl_ACCUM_V           ;  // ACCUM_V            - Напряжение на аккумуляторе
  uint16_t smpl_Temper            ;  // Tempe1             - Внутренний температурный сенсор
}
T_ADC_results;


#ifdef ADC_GLOBAL

T_ADC_raw_results   g_ADC_raw_results;
T_ADC_results       g_ADC_results;

#else

extern T_ADC_raw_results   g_ADC_raw_results;
extern T_ADC_results       g_ADC_results;

#endif

int32_t Get_MAINS_PATH_STATE      (void);
int32_t Get_INV_PATH_STATE        (void);


float   Value_ACCUM_ID_I          (int32_t v);
float   Value_ACCUM_I             (int32_t v);
float   Value_PSRC_I              (int32_t v);
float   Value_INVERT_I            (int32_t v);
float   Value_INVERT_V            (int32_t v);
float   Value_SYS_V               (int32_t v);
float   Value_MAINS_PATH_STATE    (int32_t v);
float   Value_INV_PATH_STATE      (int32_t v);
float   Value_PSRC_V              (int32_t v);
float   Value_ACCUM_V             (int32_t v);


float   Value_fp_ACCUM_ID_I       (float v);
float   Value_fp_ACCUM_I          (float v);
float   Value_fp_PSRC_I           (float v);
float   Value_fp_INVERT_I         (float v);
float   Value_fp_INVERT_V         (float v);
float   Value_fp_SYS_V            (float v);
float   Value_fp_MAINS_PATH_STATE (float v);
float   Value_fp_INV_PATH_STATE   (float v);
float   Value_fp_PSRC_V           (float v);
float   Value_fp_ACCUM_V          (float v);



#endif // ANALOG_INPUTS_H



