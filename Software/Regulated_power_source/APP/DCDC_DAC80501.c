// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.09.27
// 15:30:42
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define DCDC_DAC_I2C     1
#define DCDC_DAC_ADDR_CHARGER (0x48)


uint16_t        DCDC_DAC_charger_U_ID;
uint16_t        DCDC_DAC_charger_U_status;

// Установка напряжения на выходе зарядника

/*-----------------------------------------------------------------------------------------------------
  65535  - 1.664
  32768  - 16.8
  10000  - 27.33
  0      - 31.91


  \param v
-----------------------------------------------------------------------------------------------------*/
void     Set_DCDC_DAC_code(int32_t v)
{
  sys.DCDC_DAC_code = (uint16_t) v;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  Get_DCDC_DAC_code(void)
{
  return sys.DCDC_DAC_code;
}


/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Value_Charger_Code(int32_t v)
{
  return DCDC_DAC_Code_to_Volt((uint16_t)v);
}

/*-----------------------------------------------------------------------------------------------------


  \param code

  \return float
-----------------------------------------------------------------------------------------------------*/
float DCDC_DAC_Code_to_Volt(uint16_t code)
{
  float v;

  code =   0xFFFF - code;
  v = (float)code * (MAX_DCDC_VOLTAGE - MIN_DCDC_VOLTAGE) / (float)0xFFFF + MIN_DCDC_VOLTAGE;
  return v;
}

/*-----------------------------------------------------------------------------------------------------
  Преобразование напряжения в код для DCDC DAC

  DAC имеет разрешение 16 бит
  Согласно расчетам

  \param v

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint16_t DCDC_DAC_Volt_to_code(float v)
{
  uint32_t n;

  if (v <= MIN_DCDC_VOLTAGE) return 0xFFFF;
  if (v >= MAX_DCDC_VOLTAGE) return 0;

  v =(v - MIN_DCDC_VOLTAGE) / (MAX_DCDC_VOLTAGE - MIN_DCDC_VOLTAGE);
  n = (uint32_t)round((1 - v) * 0xFFFF);

  return n;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t DCDC_DAC_read_status(void)
{
  uint8_t buf[4];
  // Читаем статус
  buf[0] = 0x07;
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,1, buf, 0 , 0, 10) != RES_OK) return RES_ERROR;


  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,0, 0, 2 ,(uint8_t *)&DCDC_DAC_charger_U_status, 10) != RES_OK) return RES_ERROR;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _DCDC_DAC_setup(void)
{
  uint8_t buf[4];


  // Инициализация DAC-ов
  // DIV и GAIN равняются 1, DAC_SYNC_EN = 0

  if (DCDC_DAC_read_status() != RES_OK) goto error_;

  // Сброс в начальное состояние
  buf[0] = 0x05;
  buf[1] = 0x0A;
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,2, buf, 0 , 0, 10) != RES_OK) goto error_;

  if (DCDC_DAC_read_status() != RES_OK) goto error_;

  // Читаем идентификаторы
  buf[0] = 0x01;
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,1, buf, 0 , 0, 10) != RES_OK) goto error_;
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,0, 0, 2 , buf, 10) != RES_OK) goto error_;
  DCDC_DAC_charger_U_ID =(buf[0] << 8) | buf[1]; // Должно быть 0x0195

  if (DCDC_DAC_read_status() != RES_OK) goto error_;

  buf[0] = 0x02; // Адрес регистра SYNC
  buf[1] = 0;
  buf[2] = 0;    // When cleared to 0 ,the DAC output is set to update immediately (asynchronous mode), default
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,3, buf, 0 , 0, 10) != RES_OK) goto error_;

  if (DCDC_DAC_read_status() != RES_OK) goto error_;

  buf[0] = 0x04; // Адрес регистра GAIN
  buf[1] = 1;    // REF-DIV = 1
  buf[2] = 0;    // GAIN = 1
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,3, buf, 0 , 0, 10) != RES_OK) goto error_;

  if (DCDC_DAC_read_status() != RES_OK) goto error_; // Здесь DCDC_DAC_charger_U_status должен равняться 0 если все правильно работает


  if (DCDC_DAC_charger_U_status == 0) sys.dcdc_dac_flag_fail  = 0;
  else sys.dcdc_dac_flag_fail  = 1;
  return RES_OK;

error_:
  BCKUPC10_I2C1_DeInit();
  Wait_ms(50);
  BCKUPC10_I2C1_Init();
  Wait_ms(50);
  sys.dcdc_dac_flag_fail  = 1;

  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t DCDC_DAC_initializing(void)
{
  for (uint32_t i=0; i < 5; i++)
  {
    if (_DCDC_DAC_setup() == RES_OK)
    {
      if (sys.dcdc_dac_flag_fail != 0)
      {
        LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U24 (DCDC DAC) status error");
        return RES_ERROR;
      }
      LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U24 (DCDC DAC) initialized successfully");
      return RES_OK;
    }
  }
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U24 (DCDC DAC) communication error");
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
  Функция вызывается из Thread_main с периодом 1024 мкс


  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t DCDC_DAC_refresh(void)
{
  uint8_t buf[4];
  static  TIME_STRUCT  state_ref_time;


  // Сразу выходим если была зафиксирована ошибка
  if (sys.dcdc_dac_flag_fail != 0) return RES_ERROR;

  if (sys.dcdc_dac_error_flag != 0)
  {
    if (Time_elapsed_sec(&state_ref_time) > 60) // Через 60 сек сбрасываем флаг и пытаемся снова обращаться к DAC
    {
      sys.dcdc_dac_error_flag = 0;
    }
    else return RES_ERROR;
  }

  // Цикл записи длится 100 мкс
  buf[0] = 0x08;  // Адрес регистра DAC_DATA
  buf[1] =(sys.DCDC_DAC_code >> 8) & 0xFF;
  buf[2] = sys.DCDC_DAC_code & 0xFF;
  if (I2C1_WriteRead(I2C_7BIT_ADDR, DCDC_DAC_ADDR_CHARGER ,3, buf, 0 , 0, 10) != RES_OK)
  {
    sys.dcdc_dac_error_flag = 1;
    Get_time(&state_ref_time);
    BCKUPC10_I2C1_DeInit();
    BCKUPC10_I2C1_Init();
    return RES_ERROR;
  }

  return RES_OK;
}




