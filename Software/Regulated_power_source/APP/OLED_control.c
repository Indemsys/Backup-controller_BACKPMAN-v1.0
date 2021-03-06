// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-02-21
// 16:42:25 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


uint8_t  inited;
uint32_t cycle_count;
uint8_t  pattern;

/*-----------------------------------------------------------------------------------------------------


  \param data
  \param len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static void BCKUPC10_write_OLED_cmd(uint8_t *data, uint8_t len)
{
  uint8_t b;
  Set_pin_OLED_DC(0); // Устанавливаем сигнал передачи команды
  Set_pin_OLED_CS(0); // Держим в единице

  // Передаем биты
  for (uint32_t i=0; i < len; i++)
  {
    b =*data;
    data++;
    for (uint32_t n=0; n < 8; n++)
    {
      Set_pin_OLEDCLK(0);
      if (b & 0x80)
      {
        Set_pin_OLEDDIN(1);
      }
      else
      {
        Set_pin_OLEDDIN(0);
      }
      b = b << 1;
      Set_pin_OLEDCLK(1);
    }
  }

  Set_pin_OLED_CS(1); // Держим в единице

}

/*-----------------------------------------------------------------------------------------------------


  \param data
  \param len

  Передача одного байта длиться 2.8 мкс при максимальной оптимизации
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_write_OLED_data(uint8_t *data, uint8_t len)
{
  uint8_t b;
  Set_pin_OLED_DC(1); // Устанавливаем сигнал передачи данных
  Set_pin_OLED_CS(0); // Держим в единице

  // Передаем биты
  for (uint32_t i=0; i < len; i++)
  {
    b =*data;
    data++;
    for (uint32_t n=0; n < 8; n++)
    {
      Set_pin_OLEDCLK(0);
      if (b & 0x80)
      {
        Set_pin_OLEDDIN(1);
      }
      else
      {
        Set_pin_OLEDDIN(0);
      }
      b = b << 1;
      Set_pin_OLEDCLK(1);
    }
  }
  Set_pin_OLED_CS(1); // Держим в единице
}

/*-----------------------------------------------------------------------------------------------------
  OLED  дисплей управляется по интерфейсу I2C0

  \param void
-----------------------------------------------------------------------------------------------------*/
void OLED_control_function(void)
{
  uint8_t   cmd[2];

  if (inited == 0)
  {
    // Инициализация
    inited = 1;

    Set_pin_OLED_CS(1); // Держим в единице
    //BCKUPC10_I2C0_Init();

    // Формируем сигнал сброса
    Set_pin_OLED_RES(1);
    Wait_ms(10);
    Set_pin_OLED_RES(0);
    Wait_ms(10);
    Set_pin_OLED_RES(1);

    cmd[0] = 0xFD; // Command Lock
    cmd[1] = 0x12;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xAE; // Set Display Off
    BCKUPC10_write_OLED_cmd(cmd,1);

    cmd[0] = 0xD5; // Set Display Clock Divide Ratio/Oscillator Frequency
    cmd[1] = 0xA0;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xA8; // Set Multiplex Ratio
    cmd[1] = 0x3F;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xD3; // Set Display Offset
    cmd[1] = 0x00;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0x40; // Set Display Start Line
    BCKUPC10_write_OLED_cmd(cmd,1);

    cmd[0] = 0x20; // Set Memory Addressing Mode
    cmd[1] = 0x00; // A[1:0] = 00b, Horizontal Addressing Mode
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xA1; // Set Segment Re-Map
    BCKUPC10_write_OLED_cmd(cmd,1);

    cmd[0] = 0xC8; // Set COM Output Scan Direction
    BCKUPC10_write_OLED_cmd(cmd,1);

    cmd[0] = 0xDA; // Set COM Pins Hardware Configuration
    cmd[1] = 0x12;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0x81; // Set Current Control
    cmd[1] = 0xCF;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xD9; // Set Pre-Charge Period
    cmd[1] = 0x22;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xDB; // Set VCOMH Deselect Level
    cmd[1] = 0x34;
    BCKUPC10_write_OLED_cmd(cmd,2);

    cmd[0] = 0xA4; // Set Entire Display On/Off
    BCKUPC10_write_OLED_cmd(cmd,1);

    cmd[0] = 0xA6; // Set Normal/Inverse Display
    BCKUPC10_write_OLED_cmd(cmd,1);

    Set_pin_OLEDP(1);
    Wait_ms(10);
    // Включаем подачу 10 V
    Set_pin_OLEDV(1);

    Wait_ms(100);

    cmd[0] = 0xAF; // Set Display On
    BCKUPC10_write_OLED_cmd(cmd,1);

    Wait_ms(100);

    pattern = 0;
  }
  else
  {
    // Основной цикл

    //    if (cycle_count > 20)
    //    {
    //      cycle_count = 0;
    //
    //      // Заполнение экрана длится <3.5 мс при максимальной оптимизации
    //      // Передача одного байта длиться 2.8 мкс
    //      for (uint32_t i=0; i < (128); i++)
    //      {
    //        for (uint32_t n=0; n < 8; n++)
    //        {
    //          BCKUPC10_write_OLED_data(&pattern,1);
    //        }
    //        pattern = rand();
    //      }
    //    }
    //    else
    //    {
    //      cycle_count++;
    //    }
  }

}


