// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-05-19
// 10:48:20 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define SPI0_PRIO      6
#define SPI1_PRIO      6

static LPSPI_Type        *SPI_base_addrs     [SPI_INTFS_NUM] = {LPSPI0, LPSPI1};
static uint32_t           SPI_PCC_index      [SPI_INTFS_NUM] = {PCC_LPSPI0_INDEX,PCC_LPSPI1_INDEX};
static int32_t            SPI_priots         [SPI_INTFS_NUM] = {SPI0_PRIO, SPI1_PRIO};
static IRQn_Type          SPI_int_nums       [SPI_INTFS_NUM] = {LPSPI0_IRQn, LPSPI1_IRQn};
//static T_SPI_isr          SPI_isrs           [SPI_INTFS_NUM] = {0, 0};
uint32_t                  SPI_tx_errors      [SPI_INTFS_NUM];
uint32_t                  SPI_rx_errors      [SPI_INTFS_NUM];

TX_EVENT_FLAGS_GROUP      SPI_flags          [SPI_INTFS_NUM];


/*-----------------------------------------------------------------------------------------------------
  Предопределенная в файле startup_MKE18F16.s функция обработчика прерывания

  \param ptr
-----------------------------------------------------------------------------------------------------*/
void LPSPI0_DriverIRQHandler(void *ptr)
{
  volatile uint32_t  status;
  LPSPI_Type        *SPI;
  SPI=SPI_base_addrs[0];

  status = SPI->SR;
  if (status & BIT(12)) SPI_rx_errors[0]++;
  if (status & BIT(11)) SPI_tx_errors[0]++;
  if (status & BIT(9))
  {
    // Сигнал завершения передачи фрейма
    if (SPI->IER & BIT(9))
    {
      tx_event_flags_set(&SPI_flags[0], SPI_FLAG_FRAME_COMPLETE, TX_OR); // Взводим флаг
    }
  }
  SPI->SR = status;
}

/*-----------------------------------------------------------------------------------------------------
  Предопределенная в файле startup_MKE18F16.s функция обработчика прерывания

  \param ptr
-----------------------------------------------------------------------------------------------------*/
void LPSPI1_DriverIRQHandler(void *ptr)
{
  volatile uint32_t  status;
  LPSPI_Type        *SPI;
  SPI=SPI_base_addrs[1];

  status = SPI->SR;
  if (status & BIT(12)) SPI_rx_errors[1]++;
  if (status & BIT(11)) SPI_tx_errors[1]++;
  if (status & BIT(9))
  {
    // Сигнал завершения передачи фрейма
    if (SPI->IER & BIT(9))
    {
      tx_event_flags_set(&SPI_flags[1], SPI_FLAG_FRAME_COMPLETE, TX_OR); // Взводим флаг
    }
  }

  SPI->SR = status;
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SPI_wait_frame_complete(uint32_t intf_num)
{
  ULONG actual_flags;
  return tx_event_flags_get(&SPI_flags[intf_num], SPI_FLAG_FRAME_COMPLETE, TX_OR_CLEAR ,&actual_flags, MS_TO_TICKS(100));
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num

  \return LPSPI_Type*
-----------------------------------------------------------------------------------------------------*/
LPSPI_Type* Get_SPI_type(uint32_t intf_num)
{
  if (intf_num >= SPI_INTFS_NUM) return  SPI_base_addrs[0];
  return  SPI_base_addrs[intf_num];
}

/*-----------------------------------------------------------------------------------------------------
  Инициализация SPI
  DMA не используем
  Разрешаем прерывания в случае ошибок приема и передачи
  Частота до предделителя 30 МГц
  Размер FIFO для передачи и приема равен 4-ем 32-х битным словам

  Скрость передачи, выбор сигнала CS, уровни CPOL, CPHA здесь не инициализируются и указываются непосредственно на старте приема-передачи данных

  \param intf_num  - номер интерфейса ( 0 или 1)
-----------------------------------------------------------------------------------------------------*/
void SPI_Init(uint32_t intf_num)
{
  LPSPI_Type        *SPI;
  char              flag_name[8];

  if (intf_num >= SPI_INTFS_NUM) return;
  SPI=SPI_base_addrs[intf_num];

  sprintf(flag_name, "SPI%d", intf_num);
  tx_event_flags_create(&SPI_flags[intf_num], flag_name);

  // Включаем тактирование SPI
  PCC->CLKCFG[SPI_PCC_index[intf_num]] = 0
                                        + LSHIFT(0, 31) // PR    | ro | This bit shows whether the peripheral is present on this device.
                                        + LSHIFT(1, 30) // CGC   |    | Clock Control. 1b - Clock enabled
                                        + LSHIFT(0, 29) // INUSE | ro | This read-only bit shows that this peripheral is being used
                                        + LSHIFT(6, 24) // PCS   |    | Peripheral Clock Source Select. 110b - System PLL clock
  ;

  // Выполняем сброс модуля
  SPI->CR = 0
           + LSHIFT(1,  9) // RRF   | Reset Receive FIFO
           + LSHIFT(1,  8) // RTF   | Reset Transmit FIFO
           + LSHIFT(1,  3) // DBGEN | Debug Enable     | 1 Module is enabled in debug mode
           + LSHIFT(1,  2) // DOZEN | Doze mode enable | 1 Module is disabled in Doze mode.
           + LSHIFT(1,  1) // RST   | Software Reset   | 1 Master logic is reset.
           + LSHIFT(1,  0) // MEN   | Module Enable    | 1 Module is enabled
  ;
  __DSB();

  SPI->CR = 0
           + LSHIFT(0,  9) // RRF   | Reset Receive FIFO
           + LSHIFT(0,  8) // RTF   | Reset Transmit FIFO
           + LSHIFT(1,  3) // DBGEN | Debug Enable     | 1 Module is enabled in debug mode
           + LSHIFT(1,  2) // DOZEN | Doze mode enable | 1 Module is disabled in Doze mode.
           + LSHIFT(0,  1) // RST   | Software Reset   | 1 Master logic is reset.
           + LSHIFT(1,  0) // MEN   | Module Enable    | 1 Module is enabled
  ;

  __DSB();

  // Сброс статусов
  SPI->SR = 0
           + LSHIFT(0, 24) // ro | MBF    | Module Busy Flag       | 0 LPSPI is idle. 1 LPSPI is busy
           + LSHIFT(1, 13) // w1c| DMF    | Data Match Flag        | 1 Have received matching data.
           + LSHIFT(1, 12) // w1c| REF    | Receive Error Flag     | 1 Receive FIFO has overflowed.
           + LSHIFT(1, 11) // w1c| TEF    | Transmit Error Flag    | 1 Transmit FIFO underrun has occurred
           + LSHIFT(1, 10) // w1c| TCF    | Transfer Complete Flag | 1 All transfers have completed.
           + LSHIFT(1,  9) // w1c| FCF    | Frame Complete Flag    | 1 Frame transfer has completed
           + LSHIFT(1,  8) // w1c| WCF    | Word Complete Flag     | 1 Transfer word completed
           + LSHIFT(0,  1) // ro | RDF    | Receive Data Flag      | 1 Receive data is ready.
           + LSHIFT(0,  0) // ro | TDF    | Transmit Data Flag     | 1 Transmit data is requested
  ;

  // Разрешение прерываний
  SPI->IER = 0
            + LSHIFT(0, 13) // DMIE | Data Match Interrupt Enable
            + LSHIFT(1, 12) // REIE | Receive Error Interrupt Enable
            + LSHIFT(1, 11) // TEIE | Transmit Error Interrupt Enable
            + LSHIFT(0, 10) // TCIE | Transfer Complete Interrupt Enable
            + LSHIFT(1,  9) // FCIE | Frame Complete Interrupt Enable
            + LSHIFT(0,  8) // WCIE | Word Complete Interrupt Enable
            + LSHIFT(0,  1) // RDIE | Receive Data Interrupt Enable
            + LSHIFT(0,  0) // TDIE | Transmit Data Interrupt Enable
  ;

  // Разрешение сигналов для DMA
  SPI->DER = 0
            + LSHIFT(0, 1) // RDDE | Receive Data DMA Enable  | 1 DMA request enabled.
            + LSHIFT(0, 0) // TDDE | Transmit Data DMA Enable | 1 DMA request enabled
  ;

  //
  SPI->CFGR0 = 0
              + LSHIFT(0,  9) // RDMO    | Receive Data Match Only | 1 Received data is discarded unless the DMF is set
              + LSHIFT(0,  8) // CIRFIFO | Circular FIFO Enable    | 1 Circular FIFO is enabled.
              + LSHIFT(0,  2) // HRSEL   | Host Request Select     | 0 Host request input is pin LPSPI_HREQ, 1 Host request input is input trigger
              + LSHIFT(0,  1) // HRPOL   | Host Request Polarity   | 0 Active low, 1 Active high
              + LSHIFT(0,  0) // HREN    | Host Request Enable     | 0 Host request is disabled, 1 Host request is enabled
  ;


  //
  SPI->CFGR1 = 0
              + LSHIFT(0, 27) // PCSCFG  | Peripheral Chip Select Configuration | 0 PCS[3:2] are enabled , 1 PCS[3:2] are disabled
              + LSHIFT(0, 26) // OUTCFG  | Output Config     | 0 Output data retains last value when chip select is negated, 1 Output data is tristated when chip select is negated.
              + LSHIFT(0, 24) // PINCFG  | Pin Configuration |
                              // 00 SIN is used for input data and SOUT for output data
              // 01 SIN is used for both input and output data
              // 10 SOUT is used for both input and output data
              // 11 SOUT is used for input data and SIN for output data.
              + LSHIFT(0, 16) // MATCFG  | Match Configuration | 000 Match disabled.
              + LSHIFT(0,  8) // PCSPOL  | Peripheral Chip Select Polarity | 0 The PCSx is active low, 1 The PCSx is active high.
              + LSHIFT(0,  3) // NOSTAL  | No Stall      | 0 Transfers will stall when transmit FIFO is empty or receive FIFO is full
              + LSHIFT(0,  2) // AUTOPCS | Automatic PCS | This bit is ignored in master mode. 0 Automatic PCS generation disabled, 1 Automatic PCS generation enabled
              + LSHIFT(0,  1) // SAMPLE  | Sample Point  | 0 Input data sampled on SCK edge, 1 Input data sampled on delayed SCK edge
              + LSHIFT(1,  0) // MASTER  | Master Mode   | 0 Slave mode, 1 Master mode
  ;

  //
  SPI->CCR = 0
            + LSHIFT(0, 24) // SCKPCS | SCK to PCS Delay        | The delay is equal to (SCKPCS + 1) cycles of the LPSPI functional clock divided by the PRESCALE configuration
            + LSHIFT(0, 16) // PCSSCK | PCS to SCK Delay        | The delay is equal to (PCSSCK + 1) cycles of the LPSPI functional clock divided by the PRESCALE configuration
            + LSHIFT(0,  8) // DBT    | Delay Between Transfers | The delay is equal to (DBT + 2) cycles of the LPSPI functional clock divided by the PRESCALE configuration
            + LSHIFT(0,  0) // SCKDIV | SCK Divider             | The SCK period is equal to (SCKDIV+2) cycles of the LPSPI functional clock divided by the PRESCALE configuration
  ;


  //
  SPI->FCR = 0
            + LSHIFT(0, 16) // RXWATER | Receive FIFO Watermark  | The Receive Data Flag is set whenever the number of words in the receive FIFO is greater than RXWATER
            + LSHIFT(3,  0) // TXWATER | Transmit FIFO Watermark | The Transmit Data Flag is set whenever the number of words in the transmit FIFO is equal or less than TXWATER
  ;

  NVIC_SetPriority(SPI_int_nums[intf_num], SPI_priots[intf_num]);
  NVIC_EnableIRQ(SPI_int_nums[intf_num]);
}




