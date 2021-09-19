#ifndef BCKPC01_ADC_H
#define BCKPC01_ADC_H


#define ADC_RESULTS_READY_FLAG    BIT(0)  //



#define ADC_DMA_DEST_MODULO       5    // Количество младших бит адреса разрешенных к изменению в адресе приемника результатов ADC. 5 бит соответствует адресации  2-х буферов по 8 16-и битных слов - 32 байта
#define ADC_DMA_SRC_MODULO        3    // Количество младших бит адреса разрешенных к изменению в адресе источника настроек ADC. Для 8-и регистров это будет 3-и бита





typedef struct
{
  const char   *name;
  float        (*int_converter)(int);
  float        (*flt_converter)(float);

} T_vals_scaling;

typedef struct
{
  ADC_Type                      *ADC;            //  Указатель на модуль ADC
  unsigned short volatile       *results_vals;   //  Указатель на буффер с результатами ADC
  unsigned char const           *config_vals;    //  Указатель на массив констант содержащих конфигурацию ADC
  uint8_t                       results_channel; //  Номер канала DMA пересылающий резульат ADC.     Более приоритетный. По умолчанию канал с большим номером имеет больший приоритет.
  uint8_t                       config_channel;  //  Номер канала DMA пересылающий конфигурацию ADC. Менее приоритетный
  uint8_t                       req_src;         //  Номер источника запросов для конфигурирования мультиплексора DMAMUX
} T_init_ADC_DMA_cbl;


extern volatile uint16_t s_VREF_1_65V;
extern volatile int16_t  s_ACCUM_ID_I;
extern volatile int16_t  s_ACCUM_I;
extern volatile int16_t  s_PSRC_I;
extern volatile int16_t  s_INVERT_I;
extern volatile int16_t  s_PSRC_V;
extern volatile int16_t  s_ACCUM_V;



void          ADCs_subsystem_init(void);
void          ADC_converter_init(ADC_Type *ADC);
int32_t       ADC_calibrating(ADC_Type *ADC);
void          ADC_activate_hrdw_trig(ADC_Type *ADC);
void          ADC_activate_soft_trig(ADC_Type *ADC);
void          Init_ADC_by_DMA(void);


#endif // BCKPC01_ADC_H



