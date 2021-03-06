// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-05-20
// 10:14:05 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define ACS37800_CPOL    1
#define ACS37800_CPHA    1
#define ACS37800_SPEED   SPI_SPEED_15000

#define U15_CS     1
#define U17_CS     3


uint32_t ACS37800_CS             [SPI_INTFS_NUM] = {U15_CS, U17_CS};

uint8_t  tx_buf[16];
uint8_t  rx_buf[16];

uint32_t reg_U15_0x1C_new =(128<<6) | 16; // rms_avg_1 = 16 , rms_avg_2 = 128;
uint32_t reg_U15_0x1C     = 0;
uint32_t reg_U17_0x1C_new =(128<<6) | 16; // rms_avg_1 = 16 , rms_avg_2 = 128;
uint32_t reg_U17_0x1C     = 0;


/*-----------------------------------------------------------------------------------------------------
  Получаем слово команды

  \param chip_select
  \param cpol
  \param cpha
  \param speed
  \param data_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t ACS37800_get_SPI_TCR(uint8_t chip_select, uint8_t cpol, uint8_t cpha, uint8_t speed, uint16_t data_sz)
{
  return 0
         + LSHIFT(cpol, 31)  // CPOL     | Clock Polarity | 0 The inactive state value of SCK is low,
                             //                           | 1 The inactive state value of SCK is high.
         + LSHIFT(cpha, 30)  // CPHA     | Clock Phase    | 0 Data is captured on the leading edge of SCK and changed on the following edge
                             //                           | 1 Data is changed on the leading edge of SCK and captured on the following edge
         + LSHIFT(speed, 27) // PRESCALE | Prescaler Value
         + LSHIFT(chip_select, 24)  // PCS      | Peripheral Chip Select
         + LSHIFT(1, 23) // LSBF     | LSB First      | 0 Data is transferred MSB first. 1 Data is transferred LSB first.
         + LSHIFT(1, 22) // BYSW     | Byte Swap      | 0 Byte swap disabled. 1 Byte swap enabled.
         + LSHIFT(0, 21) // CONT     | Continuous Transfer | In master mode, continuous transfer will keep the PCS asserted at the end of the frame size, until a
                         //                                | command word is received that starts a new frame
         + LSHIFT(0, 20) // CONTC    | Continuing Command  | In master mode, this bit allows the command word to be changed within a continuous transfer.
         + LSHIFT(0, 19) // RXMSK    | Receive Data Mask   | When set, receive data is masked (receive data is not stored in receive FIFO).
         + LSHIFT(0, 18) // TXMSK    | Transmit Data Mask  | When set, transmit data is masked (no data is loaded from transmit FIFO and output pin is tristated)
         + LSHIFT(0, 16) // WIDTH    | Transfer Width      | 00 Single bit transfer.
         + LSHIFT(data_sz * 8-1,  0) // FRAMESZ  | Frame Size          | Configures the frame size in number of bits equal to (FRAMESZ + 1)
         ;
}


/*-----------------------------------------------------------------------------------------------------
  Проверочная передача и прием на чипы ACS37800

  \param void
-----------------------------------------------------------------------------------------------------*/
static void ACS37800_SPI_write_read_bufs(uint32_t chip_id, uint8_t  *tx_buf, uint8_t  *rx_buf, uint32_t data_sz)
{
  LPSPI_Type  *SPI;
  uint32_t     rx_cnt;
  uint32_t     tx_cnt;
  uint32_t     swrd;

  SPI=Get_SPI_type(chip_id);

  SPI->TCR = ACS37800_get_SPI_TCR(ACS37800_CS[chip_id], ACS37800_CPOL, ACS37800_CPHA, ACS37800_SPEED, data_sz);

  rx_cnt = 0;
  tx_cnt = 0;
  do
  {
    // Ждем появления места в FIFO передатчика
    while  ((SPI->SR & BIT(0)) == 0) __no_operation();

    // Наполняем 32-х битное слов данными
    swrd = tx_buf[tx_cnt];
    tx_cnt++;
    if (tx_cnt < data_sz)
    {
      swrd <<= 8;
      swrd |= tx_buf[tx_cnt];
      tx_cnt++;
      if (tx_cnt < data_sz)
      {
        swrd <<= 8;
        swrd |= tx_buf[tx_cnt];
        tx_cnt++;
        if (tx_cnt < data_sz)
        {
          swrd <<= 8;
          swrd |= tx_buf[tx_cnt];
          tx_cnt++;
        }
        else swrd <<= 8;
      }
      else swrd <<= 16;
    }
    else swrd <<= 24;

    SPI->TDR = swrd;

    // Проверить есть ли принятые данные
    if ((SPI->RSR & BIT(1)) == 0)
    {
      swrd = SPI->RDR;
      if (rx_cnt < data_sz)
      {
        rx_buf[rx_cnt] =(swrd >> 24) & 0xFF;
        rx_cnt++;
        if (rx_cnt < data_sz)
        {
          rx_buf[rx_cnt] =(swrd >> 16) & 0xFF;
          rx_cnt++;
          if (rx_cnt < data_sz)
          {
            rx_buf[rx_cnt] =(swrd >> 8) & 0xFF;
            rx_cnt++;
            if (rx_cnt < data_sz)
            {
              rx_buf[rx_cnt] =(swrd >> 0) & 0xFF;
              rx_cnt++;
            }
          }
        }
      }
    }
  }while (tx_cnt < data_sz);

  while (rx_cnt < data_sz)
  {
    // Ждем появления данных в FIFO приемника
    while  ((SPI->RSR & BIT(1)) != 0) __no_operation();

    swrd = SPI->RDR;
    if (rx_cnt < data_sz)
    {
      rx_buf[rx_cnt] =(swrd >> 24) & 0xFF;
      rx_cnt++;
      if (rx_cnt < data_sz)
      {
        rx_buf[rx_cnt] =(swrd >> 16) & 0xFF;
        rx_cnt++;
        if (rx_cnt < data_sz)
        {
          rx_buf[rx_cnt] =(swrd >> 8) & 0xFF;
          rx_cnt++;
          if (rx_cnt < data_sz)
          {
            rx_buf[rx_cnt] =(swrd >> 0) & 0xFF;
            rx_cnt++;
          }
        }
      }
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
  Чтение регистра из ACS37800

  \param chip_id
  \param reg_addr
  \param p_reg_val
-----------------------------------------------------------------------------------------------------*/
static void ACS37800_SPI_read_cmd(uint32_t chip_id, uint8_t reg_addr, uint32_t *p_reg_val)
{
  LPSPI_Type  *SPI;
  uint32_t     swrd;
  uint32_t     val;


  SPI=Get_SPI_type(chip_id);

  SPI->TCR = ACS37800_get_SPI_TCR(ACS37800_CS[chip_id], ACS37800_CPOL, ACS37800_CPHA, ACS37800_SPEED, 5);

  swrd =((reg_addr | ACS37800_RD_CMD)<< 24);
  while  ((SPI->SR & BIT(0)) == 0) __no_operation(); // Ждем появления места в FIFO передатчика
  SPI->TDR = swrd;
  swrd = 0;
  while  ((SPI->SR & BIT(0)) == 0) __no_operation(); // Ждем появления места в FIFO передатчика
  SPI->TDR = swrd;

  // Ждем сигнала о завершении передачи
  if (SPI_wait_frame_complete(chip_id) == TX_SUCCESS)  // Ожидание длится 11-16 мкс
  {
    while  ((SPI->RSR & BIT(1)) != 0) __no_operation(); // Ждем появления данных в FIFO приемника
    swrd = SPI->RDR;
    //          3-и байт              2-й байт             1-й байт
    val =((swrd >> 16) & 0xFF) | (swrd & 0x0000FF00) | ((swrd & 0xFF)<< 16);


    while  ((SPI->RSR & BIT(1)) != 0) __no_operation(); // Ждем появления данных в FIFO приемника
    swrd = SPI->RDR;
    //  4-й байт
    val = val | (swrd & 0xFF000000);
    *p_reg_val = val;
  }
  else
  {
    *p_reg_val = 0;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Запись регистра в ACS37800

  \param chip_id
  \param reg_addr
  \param p_reg_val
-----------------------------------------------------------------------------------------------------*/
static void ACS37800_SPI_write_cmd(uint32_t chip_id, uint8_t reg_addr, uint32_t reg_val)
{
  LPSPI_Type  *SPI;
  uint32_t     swrd;
  volatile     uint32_t dummy;


  SPI=Get_SPI_type(chip_id);

  SPI->TCR = ACS37800_get_SPI_TCR(ACS37800_CS[chip_id], ACS37800_CPOL, ACS37800_CPHA, ACS37800_SPEED, 5);

  swrd =((reg_addr | ACS37800_WR_CMD)<< 24) | ((reg_val & 0xFF)<< 16) | (reg_val & 0xFF00) | ((reg_val & 0x00FF0000)>> 16);
  while  ((SPI->SR & BIT(0)) == 0)
  {
    __no_operation(); // Ждем появления места в FIFO передатчика
  }
  SPI->TDR = swrd;

  swrd = reg_val & 0xFF000000;
  while  ((SPI->SR & BIT(0)) == 0)
  {
    __no_operation(); // Ждем появления места в FIFO передатчика
  }
  SPI->TDR = swrd;

  // Ждем сигнала о завершении передачи и извлекаем принятые данные из FIFO, чтобы его очистить
  SPI_wait_frame_complete(chip_id);
  while  ((SPI->RSR & BIT(1)) != 0) __no_operation(); // Ждем появления данных в FIFO приемника
  dummy = SPI->RDR;
  while  ((SPI->RSR & BIT(1)) != 0) __no_operation(); // Ждем появления данных в FIFO приемника
  dummy = SPI->RDR;

}

/*-----------------------------------------------------------------------------------------------------


  \param chip_id
  \param reg_addr
  \param p_reg_val
-----------------------------------------------------------------------------------------------------*/
static void ACS37800_SPI_read_reg(uint32_t chip_id, uint8_t reg_addr, uint32_t *p_reg_val)
{
  uint32_t dummy_val;
  ACS37800_SPI_read_cmd(chip_id, reg_addr,&dummy_val);
  ACS37800_SPI_read_cmd(chip_id, reg_addr,p_reg_val);
}

/*-----------------------------------------------------------------------------------------------------


  \param chip_id
  \param reg_addr
  \param p_reg_val
-----------------------------------------------------------------------------------------------------*/
static void ACS37800_SPI_write_reg(uint32_t chip_id, uint8_t reg_addr, uint32_t reg_val)
{
  ACS37800_SPI_write_cmd(chip_id, reg_addr, reg_val);
}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void ACS37800_read_register(uint8_t reg_addr, uint32_t *p_reg_val)
{

  uint32_t sz;

  tx_buf[0] = reg_addr | ACS37800_RD_CMD; // Команда чтения
  tx_buf[1] = 0;
  tx_buf[2] = 0;
  tx_buf[3] = 0;
  tx_buf[4] = 0;
  sz = 5;
  ACS37800_SPI_write_read_bufs(U15_SPI_ID,tx_buf, rx_buf, sz);

  memcpy(p_reg_val,&rx_buf[1], 4);

  return;
}


/*-----------------------------------------------------------------------------------------------------
  Проверка работоспособности измерителей параме

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t ACS37800_test(void)
{
  uint8_t  err= 0;
  ACS37800_SPI_write_reg(U15_SPI_ID, 0x2F, 0x4F70656E);   // Разрешение на запись в U15
  ACS37800_SPI_write_reg(U17_SPI_ID, 0x2F, 0x4F70656E);   // Разрешение на запись в U17


  ACS37800_SPI_write_reg(U15_SPI_ID , 0x1C, 0);
  ACS37800_SPI_write_reg(U17_SPI_ID , 0x1C, 0);
  ACS37800_SPI_read_reg(U15_SPI_ID, 0x1C,&reg_U15_0x1C_new);
  ACS37800_SPI_read_reg(U17_SPI_ID, 0x1C,&reg_U17_0x1C_new);

  if (reg_U15_0x1C_new != 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U15 write 0x00 error");
    err++;
  }
  if (reg_U17_0x1C_new != 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U17 write 0x00 error");
    err++;
  }

  ACS37800_SPI_write_reg(U15_SPI_ID , 0x1C, 0x0A);
  ACS37800_SPI_write_reg(U17_SPI_ID , 0x1C, 0x0A);
  ACS37800_SPI_read_reg(U15_SPI_ID, 0x1C,&reg_U15_0x1C_new);
  ACS37800_SPI_read_reg(U17_SPI_ID, 0x1C,&reg_U17_0x1C_new);

  if (reg_U15_0x1C_new != 0x0A)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U15 write 0x0A error");
    err++;
  }
  if (reg_U17_0x1C_new != 0x0A)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chip U17 write 0x0A error");
    err++;
  }

  if (err > 0)
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chips U17 and U15 test error");
    return RES_ERROR;
  }
  else
  {
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Chips U17 and U15 tested successfully");

    Wait_ms(50);
    ACS37800_read();

    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"U15: Vrms=%0.1f V, Irms=%0.2f A", Get_float_val(&sys.vrms_u15), Get_float_val(&sys.irms_u15));
    LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"U17: Vrms=%0.1f V, Irms=%0.2f A", Get_float_val(&sys.vrms_u17), Get_float_val(&sys.irms_u17));
    return RES_OK;

  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void ACS37800_read(void)
{
  uint32_t w;
  float    f;

  // Переменная reg_0x1C_new введена для возможности изменения регистра 0x1С  в реальном времени из окна отладчика
  if (reg_U15_0x1C_new != reg_U15_0x1C)
  {
    ACS37800_SPI_write_reg(U15_SPI_ID, 0x1C, reg_U15_0x1C_new);
    ACS37800_SPI_read_reg(U15_SPI_ID, 0x1C,&reg_U15_0x1C);
  }
  if (reg_U17_0x1C_new != reg_U17_0x1C)
  {
    ACS37800_SPI_write_reg(U17_SPI_ID, 0x1C, reg_U17_0x1C_new);
    ACS37800_SPI_read_reg(U17_SPI_ID, 0x1C,&reg_U17_0x1C);
  }


  // Чтение действующих токов и напряжений
  ACS37800_SPI_read_reg(U15_SPI_ID, 0x20,&w);
  f = (float)(w & 0xFFFF) * 0.250f / (55050.0f * (0.91f / (330.0f * 6.0f + 0.91f)));
  Set_float_val(&sys.vrms_u15, f);
  f = (float)((w >> 16) & 0xFFFF) * 30.0f / 55050.0f;
  Set_float_val(&sys.irms_u15, f);

  ACS37800_SPI_read_reg(U17_SPI_ID, 0x20,&w);
  f  = (float)(w & 0xFFFF) * 0.250f / (55050.0f * (0.91f / (330.0f * 6.0f + 0.91f)));
  Set_float_val(&sys.vrms_u17, f);
  f  = (float)((w >> 16) & 0xFFFF) * 30.0f / 55050.0f;
  Set_float_val(&sys.irms_u17, f);

}


float Value_VRMS_U15(int32_t v)
{
  return Get_float_val(&sys.vrms_u15);
}
float Value_IRMS_U15(int32_t v)
{
  return Get_float_val(&sys.irms_u15);
}
float Value_VRMS_U17(int32_t v)
{
  return Get_float_val(&sys.vrms_u17);
}
float Value_IRMS_U17(int32_t v)
{
  return Get_float_val(&sys.irms_u17);
}

