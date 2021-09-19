// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.11.26
// 14:29:36
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "app.h"

#define UART_INTFS_NUM  3


static void UART_default_rx_isr(uint32_t intf_num);
static void UART_default_tx_isr(uint32_t intf_num);


#define UART0_PRIO  6
#define UART1_PRIO  6
#define UART2_PRIO  6


static T_UART_cbl         UART_cbls           [UART_INTFS_NUM]; // Контрольные блоки I2C
static LPUART_Type       *UART_base_addrs     [UART_INTFS_NUM] = {LPUART0, LPUART1, LPUART2};
static uint32_t           UART_PCC_index      [UART_INTFS_NUM] = {PCC_LPUART0_INDEX,PCC_LPUART1_INDEX,PCC_LPUART2_INDEX};

static int32_t            UART_priots         [UART_INTFS_NUM] = {UART0_PRIO, UART1_PRIO, UART2_PRIO};
static IRQn_Type          UART_rx_int_nums    [UART_INTFS_NUM] = {LPUART0_RX_IRQn, LPUART1_RX_IRQn, LPUART2_RX_IRQn };
static IRQn_Type          UART_tx_int_nums    [UART_INTFS_NUM] = {LPUART0_TX_IRQn, LPUART1_TX_IRQn, LPUART2_TX_IRQn };
static T_UART_isr         UART_rx_isrs        [UART_INTFS_NUM] = {0, 0, 0 };
static T_UART_isr         UART_tx_isrs        [UART_INTFS_NUM] = {0, 0, 0 };

#define BIT_TIE   23
#define BIT_TCIE  22
#define BIT_RIE   21
#define BIT_TE    19
#define BIT_RE    18

#define BIT_TDRE  23
#define BIT_TC    22
#define BIT_RDRF  21


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void LPUART0_RX_DriverIRQHandler(void *ptr)
{
  if (UART_rx_isrs[0]!= 0) (UART_rx_isrs[0])(0);

}
void LPUART1_RX_DriverIRQHandler(void *ptr)
{
  if (UART_rx_isrs[1]!= 0) (UART_rx_isrs[1])(1);

}
void LPUART2_RX_DriverIRQHandler(void *ptr)
{
  if (UART_rx_isrs[2]!= 0) (UART_rx_isrs[2])(2);

}

void LPUART0_TX_DriverIRQHandler(void *ptr)
{
  if (*UART_tx_isrs[0]!= 0) (*UART_tx_isrs[0])(0);

}
void LPUART1_TX_DriverIRQHandler(void *ptr)
{
  if (*UART_tx_isrs[1]!= 0) (*UART_tx_isrs[1])(1);

}
void LPUART2_TX_DriverIRQHandler(void *ptr)
{
  if (*UART_tx_isrs[2]!= 0) (*UART_tx_isrs[2])(2);

}


/*-----------------------------------------------------------------------------------------------------
  Инициализация UART с номером intf_num
  Каждый UART имеет FIFO глубиной 4 слова


  \param intf_num  - номер порта
  \param baud
  \param parity
  \param stops
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Init(uint32_t intf_num, uint32_t  baud, uint8_t  parity, uint8_t  stops, uint8_t en_hw_flow_ctrl, T_UART_isr rx_isr_ptr, T_UART_isr tx_isr_ptr)
{
  LPUART_Type        *UART;
  uint32_t            ovrs = 4; // Коэффициент oversampling
  uint32_t            sbr;
  uint8_t             pe,pt;

  switch (parity)
  {
  case UART_NO_PARITY:
    pe = 0;
    pt = 0;
    break;
  case UART_EVEN_PARITY:
    pe = 1;
    pt = 0;
    break;
  case UART_ODD_PARITY:
    pe = 1;
    pt = 1;
    break;
  }

  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  // Включаем тактирование UART-а
  PCC->CLKCFG[UART_PCC_index[intf_num]] = 0
                                         + LSHIFT(0, 31) // PR    | ro | This bit shows whether the peripheral is present on this device.
                                         + LSHIFT(1, 30) // CGC   |    | Clock Control. 1b - Clock enabled
                                         + LSHIFT(0, 29) // INUSE | ro | This read-only bit shows that this peripheral is being used
                                         + LSHIFT(6, 24) // PCS   |    | Peripheral Clock Source Select. 110b - System PLL clock
  ;


  // Сбрасываем UART
  UART->GLOBAL = BIT(1);
  UART->GLOBAL = 0;

  UART->PINCFG = 0; // Входной триггер не используется

  sbr = UART_UNIT_CLOCK / baud;
  sbr = sbr / ovrs;


  UART->BAUD = 0
              + LSHIFT(0,     31) // MAEN1    | Match Address Mode Enable 1            | 0 - Normal operation
              + LSHIFT(0,     30) // MAEN2    | Match Address Mode Enable 2            | 0 - Normal operation
              + LSHIFT(0,     29) // M10      | 10-bit Mode select                     | 0 - Receiver and transmitter use 7-bit to 9-bit data characters.
              + LSHIFT(ovrs-1,24) // OSR      | Oversampling Ratio                     | 4 - оверсэмплинг
              + LSHIFT(0,     23) // TDMAE    | Transmitter DMA Enable                 |
              + LSHIFT(0,     22) // Reserved |                                        |
              + LSHIFT(0,     21) // RDMAE    | Receiver Full DMA Enable               |
              + LSHIFT(0,     20) // RIDMAE   | Receiver Idle DMA Enable               |
              + LSHIFT(0,     18) // MATCFG   | Match Configuration                    |
              + LSHIFT(1,     17) // BOTHEDGE | Both Edge Sampling                     |
              + LSHIFT(0,     16) // RESYNCDIS| Resynchronization Disable              |
              + LSHIFT(0,     15) // LBKDIE   | LIN Break Detect Interrupt Enable      |
              + LSHIFT(0,     14) // RXEDGIE  | RX Input Active Edge Interrupt Enable  |
              + LSHIFT(stops, 13) // SBNS     | Stop Bit Number Select                 |
              + LSHIFT(sbr,    0) // SBR      | Baud Rate Modulo Divisor.              |
  ;

  //
  UART->STAT = 0
              + LSHIFT(1, 31) // LBKDIF  | w1c| LIN Break Detect Interrupt Flag
              + LSHIFT(1, 30) // RXEDGIF | w1c| RXD Pin Active Edge Interrupt Flag
              + LSHIFT(0, 29) // MSBF    |    | MSB First
              + LSHIFT(0, 28) // RXINV   |    | Receive Data Inversion
              + LSHIFT(0, 27) // RWUID   |    | Receive Wake Up Idle Detect
              + LSHIFT(0, 26) // BRK13   |    | Break Character Generation Length
              + LSHIFT(0, 25) // LBKDE   |    | LIN Break Detection Enable
              + LSHIFT(0, 24) // RAF     | ro | Receiver Active Flag
              + LSHIFT(0, 23) // TDRE    | ro | Transmit Data Register Empty Flag
              + LSHIFT(0, 22) // TC      | ro | Transmission Complete Flag
              + LSHIFT(0, 21) // RDRF    | ro | Receive Data Register Full Flag
              + LSHIFT(1, 20) // IDLE    | w1c| Idle Line Flag
              + LSHIFT(1, 19) // OR      | w1c| Receiver Overrun Flag
              + LSHIFT(1, 18) // NF      | w1c| Noise Flag
              + LSHIFT(1, 17) // FE      | w1c| Framing Error Flag
              + LSHIFT(1, 16) // PF      | w1c| Parity Error Flag
              + LSHIFT(1, 15) // MA1F    | w1c| Match 1 Flag
              + LSHIFT(1, 14) // MA2F    | w1c| Match 2 Flag
  ;

  //
  UART->CTRL = 0
              + LSHIFT(0, 31) // R8T9    | Receive Bit 8 / Transmit Bit 9
              + LSHIFT(0, 30) // R9T8    | Receive Bit 9 / Transmit Bit 8
              + LSHIFT(0, 29) // TXDR    | TXD Pin Direction in Single-Wire Mode
              + LSHIFT(0, 28) // TXINV   | Transmit Data Inversion
              + LSHIFT(0, 27) // ORIE    | Overrun Interrupt Enable
              + LSHIFT(0, 26) // NEIE    | Noise Error Interrupt Enable
              + LSHIFT(0, 25) // FEIE    | Framing Error Interrupt Enable
              + LSHIFT(0, 24) // PEIE    | Parity Error Interrupt Enable
              + LSHIFT(0, 23) // TIE     | Transmit Interrupt Enable
              + LSHIFT(0, 22) // TCIE    | Transmission Complete Interrupt Enable for
              + LSHIFT(0, 21) // RIE     | Receiver Interrupt Enable
              + LSHIFT(0, 20) // ILIE    | Idle Line Interrupt Enable
              + LSHIFT(0, 19) // TE      | Transmitter Enable
              + LSHIFT(0, 18) // RE      | Receiver Enable
              + LSHIFT(0, 17) // RWU     | Receiver Wakeup Control
              + LSHIFT(0, 16) // SBK     | Send Break
              + LSHIFT(0, 15) // MA1IE   | Match 1 Interrupt Enable
              + LSHIFT(0, 14) // MA2IE   | Match 2 Interrupt Enable
              + LSHIFT(0, 11) // M7      | 7-Bit Mode Select
              + LSHIFT(0,  8) // IDLECFG | Idle Configuration
              + LSHIFT(0,  7) // LOOPS   | Loop Mode Select
              + LSHIFT(0,  6) // DOZEEN  | Doze Enable
              + LSHIFT(0,  5) // RSRC    | Receiver Source Select
              + LSHIFT(0,  4) // M       | 9-Bit or 8-Bit Mode Select
              + LSHIFT(0,  3) // WAKE    | Receiver Wakeup Method Select
              + LSHIFT(0,  2) // ILT     | Idle Line Type Select
              + LSHIFT(pe, 1) // IPE     | Parity Enable
              + LSHIFT(pt, 0) // PT      | Parity Type
  ;
  //
  if (en_hw_flow_ctrl != 0)
  {
    UART->MODIR = 0
                 + LSHIFT(0, 18) // IREN     | Infrared enable                     | Enables/disables the infrared modulation/demodulation.
                 + LSHIFT(0, 16) // TNP      | Transmitter narrow pulse            | Configures whether the LPUART transmits a 1/OSR, 2/OSR, 3/OSR or 4/OSR narrow pulse when IR is enabled.
                 + LSHIFT(1,  8) // RTSWATER | Receive RTS Configuration           | Configures the point at which the RX RTS output negates based on the number of additional characters that can be stored in the Receive FIFO
                 + LSHIFT(0,  5) // TXCTSSRC | Transmit CTS Source                 | Configures the source of the CTS input. 0 - CTS input is the CTS_B pin.
                 + LSHIFT(0,  4) // TXCTSC   | Transmit CTS Configuration          | Configures if the CTS state is checked at the start of each character or only when the transmitter is idle. 0 - CTS input is sampled at the start of each character.
                 + LSHIFT(1,  3) // RXRTSE   | Receiver request-to-send enable     | Allows the RTS output to control the CTS input of the transmitting device to prevent receiver overrun. 1 - RTS is deasserted if the receiver data register is ful
                 + LSHIFT(0,  2) // TXRTSPOL | Transmitter request-to-send polarity| 0 - Transmitter RTS is active low.
                 + LSHIFT(0,  1) // TXRTSE   | Transmitter request-to-send enable  | Controls RTS before and after a transmission. 0 - The transmitter has no effect on RTS.
                 + LSHIFT(1,  0) // TXCTSE   | Transmitter clear-to-send enable    | 1 - Enables clear-to-send operation.
    ;
  }
  else
  {
    UART->MODIR = 0
                 + LSHIFT(0, 18) // IREN     | Infrared enable
                 + LSHIFT(0, 16) // TNP      | Transmitter narrow pulse
                 + LSHIFT(0,  8) // RTSWATER | Receive RTS Configuration
                 + LSHIFT(0,  5) // TXCTSSRC | Transmit CTS Source
                 + LSHIFT(0,  4) // TXCTSC   | Transmit CTS Configuration
                 + LSHIFT(0,  3) // RXRTSE   | Receiver request-to-send enable
                 + LSHIFT(0,  2) // TXRTSPOL | Transmitter request-to-send polarity
                 + LSHIFT(0,  1) // TXRTSE   | Transmitter request-to-send enable
                 + LSHIFT(0,  0) // TXCTSE   | Transmitter clear-to-send enable
    ;
  }

  //
  UART->FIFO = 0
              + LSHIFT(0, 23) // TXEMPT     | ro | Transmit Buffer/FIFO Empty             | Asserts when there is no data in the Transmit FIFO/buffer. This field does not take into account data that is in the transmit shift register.
              + LSHIFT(0, 22) // RXEMPT     | ro | Receive Buffer/FIFO Empty              | Asserts when there is no data in the receive FIFO/Buffer. This field does not take into account data that is in the receive shift register
              + LSHIFT(1, 17) // TXOF       | w1c| Transmitter Buffer Overflow Flag       | Indicates that more data has been written to the transmit buffer than it can hold
              + LSHIFT(1, 16) // RXUF       | w1c| Receiver Buffer Underflow Flag         | Indicates that more data has been read from the receive buffer than was present.
              + LSHIFT(0, 15) // TXFLUSH    | wo | Transmit FIFO/Buffer Flush             | Writing to this field causes all data that is stored in the transmit FIFO/buffer to be flushed.
              + LSHIFT(0, 14) // RXFLUSH    | wo | Receive FIFO/Buffer Flush              | Writing to this field causes all data that is stored in the receive FIFO/buffer to be flushed.
              + LSHIFT(0, 10) // RXIDEN     |    | Receiver Idle Empty Enable             | When set, enables the assertion of RDRF when the receiver is idle for a number of idle characters and the FIFO is not empty.
              + LSHIFT(0,  9) // TXOFE      |    | Transmit FIFO Overflow Interrupt Enable| When this field is set, the TXOF flag generates an interrupt to the host.
              + LSHIFT(0,  8) // RXUFE      |    | Receive FIFO Underflow Interrupt Enable| When this field is set, the RXUF flag generates an interrupt to the host
              + LSHIFT(1,  7) // TXFE       |    | Transmit FIFO Enable                   | When this field is set, the built in FIFO structure for the transmit buffer is enabled.
              + LSHIFT(0,  4) // TXFIFOSIZE | ro | Transmit FIFO. Buffer Depth            | The maximum number of transmit datawords that can be stored in the transmit buffer.
              + LSHIFT(1,  3) // RXFE       |    | Receive FIFO Enable                    | When this field is set, the built in FIFO structure for the receive buffer is enabled.
              + LSHIFT(0,  0) // RXFIFOSIZE | ro | Receive FIFO. Buffer Depth             | The maximum number of receive datawords that can be stored in the receive buffer before an overrun occurs.
  ;

  UART->WATER = 0
               + LSHIFT(0, 24) // RXCOUNT | ro| Receive Counter. The value in this register indicates the number of datawords that are in the receive FIFO/buffer
               + LSHIFT(0, 16) // RXWATER |   | Receive Watermark. When the number of datawords in the receive FIFO/buffer is greater than the value in this register field, an interrupt or a DMA request is generated
               + LSHIFT(0,  8) // TXCOUNT | ro| Transmit Counter. The value in this register indicates the number of datawords that are in the transmit FIFO/buffer
               + LSHIFT(3,  0) // TXWATER |   | Transmit Watermark. When the number of datawords in the transmit FIFO/buffer is equal to or less than the value in this register field, an interrupt or a DMA request is generated.
  ;

  if (rx_isr_ptr == DEFAULT_RX_ISR)
  {
    UART_rx_isrs[intf_num]  = UART_default_rx_isr;
  }
  else
  {
    UART_rx_isrs[intf_num] = rx_isr_ptr;
  }

  if (tx_isr_ptr == DEFAULT_TX_ISR)
  {
    UART_tx_isrs[intf_num]  = UART_default_tx_isr;
  }
  else
  {
    UART_tx_isrs[intf_num]  = tx_isr_ptr;
  }

  NVIC_SetPriority(UART_rx_int_nums[intf_num], UART_priots[intf_num]);
  NVIC_SetPriority(UART_tx_int_nums[intf_num], UART_priots[intf_num]);
  NVIC_EnableIRQ(UART_rx_int_nums[intf_num]);
  NVIC_EnableIRQ(UART_tx_int_nums[intf_num]);

}



/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
static void UART_default_rx_isr(uint32_t intf_num)
{
  uint32_t stat_reg;
  uint32_t dataw;
  LPUART_Type*  UART;
  T_UART_cbl     *pcbl;

  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];
  pcbl =&UART_cbls[intf_num];

  stat_reg = UART->STAT;

  if (stat_reg & BIT(BIT_RDRF))
  {
    // Есть данные в FIFO приемника, можно принять байт
    dataw  = UART->DATA;
    if (UART->CTRL & BIT(BIT_RIE))
    {
      if (pcbl->uart_rx_callback != 0) pcbl->uart_rx_callback(intf_num, (uint8_t)dataw);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерываний по флагам
  TDRE - Transmit Data Register Empty Flag
  TC   - Transmission Complete Flag

  \param intf_num
-----------------------------------------------------------------------------------------------------*/
static void UART_default_tx_isr(uint32_t intf_num)
{
  uint32_t stat_reg;
  uint32_t ctrl_reg;
  LPUART_Type*  UART;
  T_UART_cbl     *pcbl;

  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];
  pcbl =&UART_cbls[intf_num];

  stat_reg = UART->STAT;
  ctrl_reg = UART->CTRL;

  if (ctrl_reg & BIT(BIT_TIE))
  {
    if (stat_reg & BIT(BIT_TDRE))
    {
      // Овободилось место в FIFO передатчика, можно передать следующий байт
      if (pcbl->uart_tx_callback != 0) pcbl->uart_tx_callback(intf_num);
    }
  }


  if (ctrl_reg & BIT(BIT_TCIE))
  {
    if (stat_reg & BIT(BIT_TC))
    {
      if (pcbl->uart_tx_compl_callback != 0) pcbl->uart_tx_compl_callback(intf_num);
    }
    else
    {
      UART->CTRL = UART->CTRL & (~BIT(BIT_TCIE)); // Реагируем на прерывание только один раз, после чего запрещаем его
    }
  }

}


/*-----------------------------------------------------------------------------------------------------


  \param intf_num

  \return UART_MemMapPtr
-----------------------------------------------------------------------------------------------------*/
LPUART_Type*  BCKUPC10_get_UART(uint32_t intf_num)
{
  if (intf_num >= UART_INTFS_NUM) return 0;
  return UART_base_addrs[intf_num];
}

/*-----------------------------------------------------------------------------------------------------
  Управление разрешением запрещение передатчика

  \param intf_num  - номер порта
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_TX_state(uint32_t intf_num, uint8_t flag)
{
  LPUART_Type* UART;
  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  if (flag == 0)
  {
    UART->CTRL &= ~BIT(BIT_TE); // TE      | Transmitter Enable
  }
  else
  {
    UART->CTRL |= BIT(BIT_TE); // TE      | Transmitter Enable
  }
}

/*-----------------------------------------------------------------------------------------------------
  Управление разрешением запрещение приемника

  \param intf_num  - номер порта
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_RX_state(uint32_t intf_num, uint8_t flag)
{
  LPUART_Type* UART;
  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  if (flag == 0)
  {
    UART->CTRL &= ~BIT(BIT_RE); // RE      | Receiver Enable
  }
  else
  {
    UART->CTRL |= BIT(BIT_RE); // RE      | Receiver Enable
  }
}

/*-----------------------------------------------------------------------------------------------------
  Запрещение или разрешение прерываний от приемника

  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_RX_Int(uint32_t intf_num, uint8_t flag)
{
  LPUART_Type* UART;
  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  if (flag == 0)
  {
    UART->CTRL &= ~BIT(BIT_RIE); // RIE     | Receiver Interrupt Enable
  }
  else
  {
    UART->CTRL |= BIT(BIT_RIE); // RIE     | Receiver Interrupt Enable
  }
}

/*-----------------------------------------------------------------------------------------------------
  Запрещение или разрешение прерываний по флагу готовности передавать данные

  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_TX_Int(uint32_t intf_num, uint8_t flag)
{
  LPUART_Type* UART;
  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  if (flag == 0)
  {
    UART->CTRL &= ~BIT(BIT_TIE); // TIE     | Transmit Interrupt Enable
  }
  else
  {
    UART->CTRL |= BIT(BIT_TIE); // TIE     | Transmit Interrupt Enable
  }
}

/*-----------------------------------------------------------------------------------------------------
  Запрещение или разрешение прерываний по флагу завершения отправки данных

  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_TXC_Int(uint32_t intf_num, uint8_t flag)
{
  LPUART_Type* UART;
  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  if (flag == 0)
  {
    UART->CTRL &= ~BIT(BIT_TCIE); // TCIE    | Transmission Complete Interrupt Enable for
  }
  else
  {
    UART->CTRL |= BIT(BIT_TCIE); // TCIE    | Transmission Complete Interrupt Enable for
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_clear_rx_fifo(uint32_t intf_num)
{

  LPUART_Type* UART = UART_base_addrs[intf_num];


  UART->FIFO |= BIT(14); // RXFLUSH    | wo | Receive FIFO/Buffer Flush              | Writing to this field causes all data that is stored in the receive FIFO/buffer to be flushed.

}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t BCKUPC10_UART_Get_TXEMPT_flag(uint32_t intf_num)
{
  uint8_t flag = 0;
  LPUART_Type* UART = UART_base_addrs[intf_num];

  // TXEMPT     | ro | Transmit Buffer/FIFO Empty. Asserts when there is no data in the Transmit FIFO/buffer. This field does not take into account data that is in the transmit shift register.
  flag =((UART->FIFO & BIT(23))>> 23); // TXEMPT

  return flag;

}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_enable_tx_DMA(uint32_t intf_num)
{
  LPUART_Type* UART = UART_base_addrs[intf_num];
  UART->BAUD |= BIT(23); // TDMAE    | Transmitter DMA Enable
}
/*-----------------------------------------------------------------------------------------------------


  \param intf_num
  \param func
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_txrx_callback(uint32_t intf_num, T_uart_rx_callback rx_callback, T_uart_tx_callback tx_callback)
{
  T_UART_cbl     *pcbl;
  if (intf_num >= UART_INTFS_NUM) return;
  pcbl =&UART_cbls[intf_num];

  pcbl->uart_rx_callback = rx_callback;
  pcbl->uart_tx_callback = tx_callback;
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num
  \param tc_callback
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_Set_tc_callback(uint32_t intf_num, T_uart_tx_compl_callback tc_callback)
{
  T_UART_cbl     *pcbl;
  if (intf_num >= UART_INTFS_NUM) return;
  pcbl =&UART_cbls[intf_num];
  pcbl->uart_tx_compl_callback = tc_callback;
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_send_byte(uint32_t intf_num, uint8_t b)
{
  LPUART_Type* UART = UART_base_addrs[intf_num];
  UART->DATA = b;
}


/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
void BCKUPC10_UART_DeInit(uint32_t intf_num)
{
  volatile uint8_t dummy;
  LPUART_Type* UART;
  T_UART_cbl     *pcbl;

  if (intf_num >= UART_INTFS_NUM) return;
  UART=UART_base_addrs[intf_num];

  DisableIRQ(UART_rx_int_nums[intf_num]);
  DisableIRQ(UART_tx_int_nums[intf_num]);

  pcbl =&UART_cbls[intf_num];


  // Сбрасываем UART
  UART->GLOBAL = BIT(1);
  UART->GLOBAL = 0;


  pcbl->uart_rx_callback = 0;
  pcbl->uart_tx_callback = 0;
}


