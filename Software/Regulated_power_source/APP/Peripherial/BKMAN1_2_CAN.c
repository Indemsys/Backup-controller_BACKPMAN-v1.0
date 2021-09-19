#include "App.h"



TX_THREAD                      can_thread;
#pragma data_alignment=8
uint8_t                        thread_can_stack[THREAD_CAN_STACK_SIZE];
static TX_EVENT_FLAGS_GROUP    can_flags_grp;

TX_QUEUE                       can_queue;
#define                        CAN_TX_QUEUE_SZ  10
#pragma data_alignment=8
T_can_tx_message               can_tx_queue_buf[CAN_TX_QUEUE_SZ];


volatile CAN_Type              *g_ptr_CAN;
T_can_statistic                can_stat;
static int8_t                  can_tx_ready;

const T_can_rx_config         rx_mboxes[RX_MBOX_CNT] =
{
  { CAN_RX_MB1, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB2, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB3, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB4, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB5, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB6, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB7, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
  { CAN_RX_MB8, 0xFFFFFFFF, 0x00000000 }, // Принимаем все пакеты
};

#define FLAG_PACKET_QUEUED     BIT(31)
#define FLAG_TX_DONE           (BIT(CAN_TX_MB1))

/*-------------------------------------------------------------------------------------------------------------
  Обслуживание прерываний CAN шины при отправке и получении данных
-------------------------------------------------------------------------------------------------------------*/
static void CAN_isr(void *ptr)
{
  volatile CAN_Type *CAN;
  uint32_t           reg;

  CAN = (CAN_Type *)ptr;

  reg = CAN->IFLAG1; // Получить флаги прерываний (всего в работе 16-ть младших флагов).
                     // Если не разрешен FIFO то каждый флаг отмечает прерывание от соответствующего буфера.
  // Снимаються флаги записью 1
  // Каждый флаг может отмечать, как прерывание по приему так и по передаче.
  if (reg & 0x0FF)
  {
    // Установка флага от 0-го до 7-го
    tx_event_flags_set(&can_flags_grp, reg & 0x0FF, TX_OR); // Флаги сообщают о приеме пакета
  }

  if (reg & FLAG_TX_DONE)
  {
    tx_event_flags_set(&can_flags_grp, FLAG_TX_DONE, TX_OR); // Установка 8-го флага сообщающего об окончании отправки пакета
  }
  CAN->IFLAG1 = reg;                              // Сбрасываем флаги, но только те о которых мы сообщили
}

/*-------------------------------------------------------------------------------------------------------------
  Обслуживание прерываний CAN шины в результате ошибок м других событий
-------------------------------------------------------------------------------------------------------------*/
void CAN_err_isr(void *ptr)
{
  volatile CAN_Type  *CAN;
  uint32_t           reg;

  CAN = (CAN_Type *)ptr;

  // Получить флаги ошибок и сбросить их
  reg = CAN->ESR1 & (
       BIT(17)   // TWRNINT. 1 The Tx error counter transitioned from less than 96 to greater than or equal to 96.
       + BIT(16) // RWRNINT. 1 The Rx error counter transitioned from less than 96 to greater than or equal to 96.
       + BIT(2)  // BOFFINT. ‘Bus Off’ Interrupt
       + BIT(1)  // ERRINT . Error Interrupt
       + BIT(0)  // WAKINT . Wake-Up Interrupt
       );
  if (reg)
  {
    CAN->ESR1 = reg; // Сбрасываем флаги
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void CAN0_ORed_Message_buffer_IRQHandler(void) // OR'ed Message buffer
{ CAN_isr(CAN0);     }
void CAN0_ORed_IRQHandler(void) // Bus Off OR Transmit Warning OR Receive Warning
{ CAN_err_isr(CAN0); }
void CAN0_Error_IRQHandler(void) // Error
{ CAN_err_isr(CAN0); }
void CAN0_Wake_Up_IRQHandler(void) // Wake Up
{ CAN_err_isr(CAN0); }
void CAN0_Reserved1_IRQHandler(void) // Reserved
{ CAN_err_isr(CAN0); }
void CAN0_Reserved2_IRQHandler(void) // Reserved (MB extension 32-47)
{ CAN_err_isr(CAN0); }
void CAN0_Reserved3_IRQHandler(void) // Reserved (MB extension 48-63)
{ CAN_err_isr(CAN0); }

void CAN1_ORed_Message_buffer_IRQHandler(void) // OR'ed Message buffer
{ CAN_isr(CAN1);     }
void CAN1_ORed_IRQHandler(void) // Bus Off OR Transmit Warning OR Receive Warning
{ CAN_err_isr(CAN1); }
void CAN1_Error_IRQHandler(void) // Error
{ CAN_err_isr(CAN1); }
void CAN1_Wake_Up_IRQHandler(void) // Wake Up
{ CAN_err_isr(CAN1); }
void CAN1_Reserved1_IRQHandler(void) // Reserved
{ CAN_err_isr(CAN1); }
void CAN1_Reserved2_IRQHandler(void) // Reserved (MB extension 32-47)
{ CAN_err_isr(CAN1); }
void CAN1_Reserved3_IRQHandler(void) // Reserved (MB extension 48-63)
{ CAN_err_isr(CAN1); }


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
int Solve_can_timings(uint32_t bus_clock, uint32_t bitrate, T_can_ctrl *ctrl)
{
  uint32_t TQ_x_Prescaler = bus_clock / bitrate;
  uint32_t TQ;
  uint32_t lowest_diff;
  uint32_t diff;
  uint32_t best_TQ;
  uint32_t actual_freq;


  ctrl->pseg1 = 0;
  ctrl->pseg2 = 0;
  ctrl->propseg = 0;
  ctrl->rjw = 0;
  ctrl->presdiv = 0;


  // If BSP_CANPE_CLOCK is defined, then we will calculate the CAN bit timing parameters
  // using the method outlined in AN1798, section 4.1. A maximum time for PROP_SEG will be used,
  // the remaining TQ will be split equally between PSEG1 and PSEG2, provided PSEG2 >=2. RJW is
  // set to the minimum of 4 or PSEG1.


  if (TQ_x_Prescaler < (CAN_MIN_TQ - 1))
  {
    // We will be off by more than 12.5%
    return RES_ERROR;
  }

  // First, find the best TQ and pre-scaler to use for the desired frequency. If any exact matches
  // is found, we use the match that gives us the lowest pre-scaler and highest TQ, otherwise we pick
  // the TQ and prescaler that generates the closest frequency to the desired frequency.

  lowest_diff = bitrate;
  best_TQ = 0;
  for (TQ = CAN_MAX_TQ; TQ >= CAN_MIN_TQ; TQ--)
  {
    ctrl->presdiv = TQ_x_Prescaler / TQ;
    if (ctrl->presdiv <= 256)
    {
      if (TQ_x_Prescaler == TQ * ctrl->presdiv)
      {
        best_TQ = TQ;
        break;
      }
      actual_freq =(bus_clock / ctrl->presdiv) / TQ;

      if (actual_freq > bitrate)
      {
        diff = actual_freq - bitrate;
      }
      else
      {
        diff = bitrate - actual_freq;
      }

      if (diff < lowest_diff)
      {
        lowest_diff = diff;
        best_TQ = TQ;
      }
    }
  }
  if ((best_TQ >= CAN_MIN_TQ) && (ctrl->presdiv <= 256))
  {
    ctrl->pseg2 =(best_TQ - CAN_MAX_PROPSEG) / 2;
    if (ctrl->pseg2 < CAN_MIN_PSEG2) ctrl->pseg2 = CAN_MIN_PSEG2;

    if (best_TQ == CAN_MIN_TQ)
    {
      ctrl->pseg1 = 1;
    }
    else
    {
      ctrl->pseg1 = ctrl->pseg2;
    }

    ctrl->propseg = best_TQ - ctrl->pseg1 - ctrl->pseg2 - 1;

    if (ctrl->pseg1 < CAN_MAX_RJW)
    {
      ctrl->rjw = ctrl->pseg1;
    }
    else
    {
      ctrl->rjw = CAN_MAX_RJW;
    }

    ctrl->propseg -= 1;
    ctrl->rjw -= 1;
    ctrl->pseg1 -= 1;
    ctrl->pseg2 -= 1;
    ctrl->presdiv -= 1;
  }
  else
  {
    return RES_ERROR;
  }

  return RES_OK;

}

/*-------------------------------------------------------------------------------------------------------------
  Создание объектов синхронизации задачи управления лифтом
-------------------------------------------------------------------------------------------------------------*/
static uint32_t Create_can_task_sync_obj(void)
{
  // Создаем группу флагов
  tx_event_flags_create(&can_flags_grp, "CAN");

  // Создаем очередь сообщений для отправки пакетов передатчику
  tx_queue_create(&can_queue, "CAN", sizeof(T_can_tx_message) / 4, can_tx_queue_buf, sizeof(can_tx_queue_buf));

  can_tx_ready = 1;
  return RES_OK;
}



/*-------------------------------------------------------------------------------------------------------------
  Обслуживание прерываний CAN шины

  ptr - указатель на область регистров контроллера CAN:  CAN0_BASE_PTR или CAN1_BASE_PTR
-------------------------------------------------------------------------------------------------------------*/
static int CAN_init(CAN_Type *ptr, uint32_t bitrate)
{

  T_can_ctrl           ctrl;
  uint32_t             i;

  g_ptr_CAN = ptr;

  if (ptr == CAN0)
  {
    PCC->CLKCFG[PCC_CAN0_INDEX] = BIT(30);     // Разрешаем тактирование CAN0
    g_ptr_CAN->CTRL1 |= BIT(13); // Тактирование от источника тактирования периферии

    // Инициализация всех прерываний
    NVIC_SetPriority(CAN0_ORed_Message_buffer_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_ORed_Message_buffer_IRQn);
    NVIC_SetPriority(CAN0_ORed_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_ORed_IRQn);
    NVIC_SetPriority(CAN0_Error_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_Error_IRQn);
    NVIC_SetPriority(CAN0_Wake_Up_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_Wake_Up_IRQn);
    NVIC_SetPriority(CAN0_Reserved1_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_Reserved1_IRQn);
    NVIC_SetPriority(CAN0_Reserved2_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_Reserved2_IRQn);
    NVIC_SetPriority(CAN0_Reserved3_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN0_Reserved3_IRQn);
  }
  else if (ptr == CAN1)
  {
    PCC->CLKCFG[PCC_CAN1_INDEX]= BIT(30);  // Разрешаем тактирование CAN0
    g_ptr_CAN->CTRL1 |= BIT(13); // Тактирование от источника тактирования периферии

    // Инициализация всех прерываний
    NVIC_SetPriority(CAN1_ORed_Message_buffer_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_ORed_Message_buffer_IRQn);
    NVIC_SetPriority(CAN1_ORed_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_ORed_IRQn);
    NVIC_SetPriority(CAN1_Error_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_Error_IRQn);
    NVIC_SetPriority(CAN1_Wake_Up_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_Wake_Up_IRQn);
    NVIC_SetPriority(CAN1_Reserved1_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_Reserved1_IRQn);
    NVIC_SetPriority(CAN1_Reserved2_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_Reserved2_IRQn);
    NVIC_SetPriority(CAN1_Reserved3_IRQn, CAN_PRIO);
    NVIC_EnableIRQ(CAN1_Reserved3_IRQn);

  }
  else return RES_ERROR;

  // Выполняем программный сброс и переходим во Freeze mode
  if (g_ptr_CAN->MCR & BIT(20)) // Проверим выключен ли модуль, и сбросим этот режим если надо
  {
    g_ptr_CAN->MCR &=(~BIT(31));       // Включаем модуль
    while (g_ptr_CAN->MCR & BIT(20))    // Ждем пока флаг не сбросится
    {
    }
  }

  g_ptr_CAN->MCR = BIT(25);             // Выполняем SOFTRESET
  while (g_ptr_CAN->MCR & BIT(25))      // Ожидаем пока SOFTRESET закончится
  {
  }

  g_ptr_CAN->MCR |= 0
                   + BIT(30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
                   + BIT(28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
  ;
  while ((g_ptr_CAN->MCR & BIT(24)) == 0)   // Ожидаем установки FRZACK
  {
  }

  Solve_can_timings(CAN_UNIT_CLOCK, bitrate,&ctrl);

  // ПРИМЕР настройка вручную скорости CAN на 500000 бит/сек :
  // ----------------------------------------------------------------------------------------------------------------------------------
  // Частота тактирования CAN периферии здесь равна 60 МГц
  // Бит состоит из 18-и квантов. Значит один квант должен длится = 1/(500000*18) = 0.1111 мкс
  // Делитель presdiv должен при этом равняться 6.6666
  // Выбираем значение presdiv = 6 , тогда получаем скорость 555555 бит/сек
  // ctrl.presdiv = 6 - 1   Делитель частоты = 6. Получаем квант Tq = 6/60000000 = 0.1 мкс
  // ctrl.rjw = 4 - 1       Стандартная величина = 3 (на длину бита не влияет)
  // ctrl.propseg = 7 - 1   Длина PROP_SEG.   3-х битное поле (фаза перед взятием отсчета)
  // ctrl.pseg1 = 5 - 1     Длина PHASE_SEG1. 3-х битное поле (фаза перед взятием отсчета).
  // ctrl.pseg2 = 5 - 1     Длина PHASE_SEG2. 3-х битное поле (фаза после взятия отсчета).
  // Длина бита в результате получается = SYNC_SEG(всегда равен 1) + PROP_SEG + PHASE_SEG1 + PHASE_SEG2 = 18Tq = 18*0.1 = 1.8 мкс
  // ----------------------------------------------------------------------------------------------------------------------------------

  g_ptr_CAN->CTRL1 = 0
                    + LSHIFT(ctrl.presdiv, 24) // PRESDIV | Prescaler Division Factor
                    + LSHIFT(ctrl.rjw, 22)     // RJW     | Resync Jump Width
                    + LSHIFT(ctrl.pseg1, 19)   // PSEG1   | Phase Segment 1
                    + LSHIFT(ctrl.pseg2, 16)   // PSEG2   | Phase Segment 2
                    + LSHIFT(1, 15)            // BOFFMSK | Bus Off Mask. 1 Bus Off interrupt enabled
                    + LSHIFT(1, 14)            // ERRMSK  | Error Mask.   1 Error interrupt enabled
                    + LSHIFT(1, 13)            // CLKSRC  | CAN Engine Clock Source. 1 The CAN engine clock source is the peripheral clock
                    + LSHIFT(0, 12)            // LPB     | Loop Back Mode
                    + LSHIFT(1, 11)            // TWRNMSK | Tx Warning Interrupt Mask. 1 Tx Warning Interrupt enabled
                    + LSHIFT(1, 10)            // RWRNMSK | Rx Warning Interrupt Mask. 1 Rx Warning Interrupt enabled
                    + LSHIFT(1, 7)             // SMP     | CAN Bit Sampling. 0 - Just one sample is used to determine the bit value, 1 - Three samples are used to determine the value of the received bit
                    + LSHIFT(0, 6)             // BOFFREC | Bus Off Recovery. 0 - Automatic recovering from Bus Off state enabled, 1 - Automatic recovering from Bus Off state disabled
                    + LSHIFT(0, 5)             // TSYN    | Timer Sync. 1 Timer Sync feature enabled
                    + LSHIFT(0, 4)             // LBUF    | Lowest Buffer Transmitted First. 1 Lowest number buffer is transmitted first.
                    + LSHIFT(0, 3)             // LOM     | Listen-Only Mode. 1 FlexCAN module operates in Listen-Only Mode.
                    + LSHIFT(ctrl.propseg, 0)  // PROPSEG | Propagation Segment.
  ;

  g_ptr_CAN->CTRL2 = 0
                    + LSHIFT(1, 28)    // WRMFRZ | Write-Access to Memory in Freeze mode. 1 Enable unrestricted write access to FlexCAN memory.
                    + LSHIFT(0, 24)    // RFFN   | Number of Rx FIFO Filters
                    + LSHIFT(0x22, 19) // TASD   | Tx Arbitration Start Delay.
                    + LSHIFT(0, 18)    // MRP    | Mailboxes Reception Priority. 0 Matching starts from Rx FIFO and continues on Mailboxes.
                    + LSHIFT(0, 17)    // RRS    | Remote Request Storing. 0 Remote Response Frame is generated.
                    + LSHIFT(0, 16)    // EACEN  | Entire Frame Arbitration Field Comparison Enable for Rx Mailboxes.0 Rx Mailbox filter’s IDE bit is always compared and RTR is never compared despite mask bits.
  ;

  for (i = 0; i < 16; i++)
  {
    g_ptr_CAN->RXIMR[i] = 0;    // Сбросим все маски
    g_ptr_CAN->MB[i].CS = 0;    // Все буфферы сообщений не активны
    g_ptr_CAN->MB[i].ID = 0;    //
    g_ptr_CAN->MB[i].WORD0 = 0; //
    g_ptr_CAN->MB[i].WORD1 = 0; //
  }
  //CAN->RXIMR[CAN_RX_MB1] = 0xFF000000;

  // Выходим из режима Freeze
  g_ptr_CAN->MCR = 0
                  + LSHIFT(0, 31) // MDIS    | Module Disable. 1 Disable the FlexCAN module.
                  + LSHIFT(0, 30) // FRZ     | Freeze Enable. 1 Enabled to enter Freeze Mode
                  + LSHIFT(0, 29) // RFEN    | Rx FIFO Enable. 1 Rx FIFO enabled
                  + LSHIFT(0, 28) // HALT    | Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
                  + LSHIFT(0, 26) // WAKMSK  | Wake Up Interrupt Mask. 1 Wake Up Interrupt is enabled.
                  + LSHIFT(0, 25) // SOFTRST | Soft Reset. 1 Resets the registers affected by soft reset.
                  + LSHIFT(0, 23) // SUPV    | Supervisor Mode. 1 FlexCAN is in Supervisor Mode.
                  + LSHIFT(0, 22) // SLFWAK  | Self Wake Up. 1 FlexCAN Self Wake Up feature is enabled.
                  + LSHIFT(1, 21) // WRNEN   | Warning Interrupt Enable. 1 TWRNINT and RWRNINT bits are set when the respective error counter transitions from less than 96 to greater than or equal to 96.
                  + LSHIFT(1, 17) // SRXDIS  | Self Reception Disable. 1 Self reception disabled
                  + LSHIFT(1, 16) // IRMQ    | Individual Rx Masking and Queue Enable. 1 Individual Rx masking and queue feature are enabled.
                  + LSHIFT(0, 13) // LPRIOEN | Local Priority Enable. 1 Local Priority enabled
                  + LSHIFT(0, 12) // AEN     | Abort Enable. 0 Abort disabled.  !!!Не выставлять 1, поскольку процедуры инициализации MB расчитаны на режим 0
                  + LSHIFT(0, 8) // IDAM     | ID Acceptance Mode. 00 Format A: One full ID (standard and extended) per ID Filter Table element.
                  + LSHIFT(16, 0) // MAXMB   | Number of the Last Message Buffer.
  ;

  return RES_OK;
}



/*-------------------------------------------------------------------------------------------------------------
  Установка буффера на прием и инициация приема
  n    - номер буфера
  id   - идентификатор CAN пакета
  mask - фильтр-маска принимаемого идентификатора
  ext  - если расширенный формат то 1, иначе 0
  rtr  - если 1 то будет отправлен пакет вида Remote Request Frame
-------------------------------------------------------------------------------------------------------------*/
void CAN_set_rx_mbox(volatile CAN_Type *ptr_CAN, uint8_t n, uint32_t id, uint32_t mask, uint8_t ext, uint8_t rtr)
{
  // Переходим в Freeze режим поскольку маску фильтра можно изменить только в этом режиме
  ptr_CAN->MCR |= 0
                 + BIT(30) // FRZ    . Freeze Enable. 1 Enabled to enter Freeze Mode
                 + BIT(28) // HALT   . Halt FlexCAN. 1 Enters Freeze Mode if the FRZ bit is asserted.
  ;
  while ((ptr_CAN->MCR & BIT(24)) == 0)   // Ожидаем установки FRZACK
  {
  }

  // Сбросим флаг прерывания
  ptr_CAN->IFLAG1 |=(1 << n);

  // Инициализируем mailbox на передачу
  ptr_CAN->MB[n].CS = 0
                     + LSHIFT(0x0, 24) // CODE. 0b0000:INACTIVE- MB is not active.
                     + LSHIFT(1, 22) // SRR.  Substitute Remote Request. (должен быть 1 в расширенном формате)
                     + LSHIFT(ext, 21) // IDE.  ID Extended Bit.           1 = расширенный формат
                     + LSHIFT(rtr, 20) // RTR.  Remote Transmission Request. 1 - это mailbox с Remote Request Frame
                     + LSHIFT(0, 16) // DLC.  Длина данных
  ;
  ptr_CAN->MB[n].ID = id;
  ptr_CAN->MB[n].WORD0 = 0;
  ptr_CAN->MB[n].WORD1 = 0;
  ptr_CAN->RXIMR[n] = mask; // Установим маску


  ptr_CAN->IMASK1 |=(1 << n); // Разрешаем прерывания после приема

  // Обозначаем готовность к приему
  ptr_CAN->MB[n].CS |= BIT(26); // 0b0100: EMPTY - MB is active and empty.

  ptr_CAN->MCR &= ~(BIT(30)+ BIT(28)); // Выходим из режиме Freeze
  while (ptr_CAN->MCR & BIT(24))        // Ожидаем сброса FRZACK
  {
  }

}

/*-------------------------------------------------------------------------------------------------------------
  Установка буффера на отправку и инициация отправки
  n    - номер буфера
  id   - идентификатор CAN пакета
  data - буффер с данными длинной 8 байт
  len  - длина данных
  ext  - если расширенный формат то 1, иначе 0
  rtr  - если 1 то будет отправлен пакет вида Remote transmission request
-------------------------------------------------------------------------------------------------------------*/
static uint32_t CAN_set_tx_mbox(volatile CAN_Type *ptr_CAN, uint8_t n, uint32_t id, uint8_t *data, uint8_t len, uint8_t ext, uint8_t rtr)
{
  // Сбросим флаг прерывания
  ptr_CAN->IFLAG1 |=(1 << n);
  // Инициализируем mailbox на передачу
  ptr_CAN->MB[n].CS = 0
                     + LSHIFT(0x8, 24) // CODE. 0b1000:MB is not active
                     + LSHIFT(1, 22) // SRR.  Substitute Remote Request. (должен быть 1 в расширенном формате)
                     + LSHIFT(ext, 21) // IDE.  ID Extended Bit.           1 = расширенный формат
                     + LSHIFT(rtr, 20) // RTR.  Remote Transmission Request. 1 - это mailbox с Remote Request Frame
                     + LSHIFT(len, 16) // DLC.  Длина данных
  ;
  ptr_CAN->MB[n].ID = id;
  ptr_CAN->MB[n].WORD0 =(data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  ptr_CAN->MB[n].WORD1 =(data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

  // Разрешаем прерывания после передачи
  ptr_CAN->IMASK1 |=(1 << n);
  // Запускаем передачу
  ptr_CAN->MB[n].CS |= BIT(26); // 0b1100: DATA - MB is a Tx Data Frame (MB RTR must be 0)

  return RES_OK;
}


/*-------------------------------------------------------------------------------------------------------------
  Выборка данных из mailbox-а с принятым пакетом
  n    - номер буфера
-------------------------------------------------------------------------------------------------------------*/
static void CAN_read_rx_mbox(volatile CAN_Type *ptr_CAN, uint8_t n, T_can_rx_message *rx)
{

  uint32_t          cs;
  uint32_t          w;
  volatile uint32_t tmp;

  cs = ptr_CAN->MB[n].CS;
  rx->len =(cs >> 16) & 0x0F;
  rx->ext =(cs >> 21) & 1;
  rx->rtr =(cs >> 20) & 1;
  rx->code =(cs >> 24) & 0x0F;
  rx->canid = ptr_CAN->MB[n].ID;
  w = ptr_CAN->MB[n].WORD0;
  rx->data[0] =(w >> 24) & 0xFF;
  rx->data[1] =(w >> 16) & 0xFF;
  rx->data[2] =(w >> 8) & 0xFF;
  rx->data[3] =(w >> 0) & 0xFF;
  w = ptr_CAN->MB[n].WORD1;
  rx->data[4] =(w >> 24) & 0xFF;
  rx->data[5] =(w >> 16) & 0xFF;
  rx->data[6] =(w >> 8) & 0xFF;
  rx->data[7] =(w >> 0) & 0xFF;
  tmp = ptr_CAN->TIMER; // Разблокировка Mialbox

}

/*-----------------------------------------------------------------------------------------------------


  \param canid
  \param data
  \param len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t CAN_post_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
  UINT res;
  T_can_tx_message queue_msg;

  queue_msg.canid = canid;
  queue_msg.rtr = CAN_NO_RTR;
  memset(queue_msg.data, 0, 8);
  memcpy(queue_msg.data, data, len);
  queue_msg.len = len;
  res = tx_queue_send(&can_queue,&queue_msg, MS_TO_TICKS(timeout_ms));
  if (res == TX_SUCCESS)
  {
    tx_event_flags_set(&can_flags_grp, FLAG_PACKET_QUEUED, TX_OR); // Флаги сообщают о приеме пакета
    return RES_OK;
  }
  return RES_ERROR;
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t CAN_post_rtr_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
  UINT res;
  T_can_tx_message queue_msg;

  queue_msg.canid = canid;
  queue_msg.rtr = CAN_RTR;
  memset(queue_msg.data, 0, 8);
  memcpy(queue_msg.data, data, len);
  queue_msg.len = len;
  res = tx_queue_send(&can_queue,&queue_msg, MS_TO_TICKS(timeout_ms));
  if (res == TX_SUCCESS)
  {
    tx_event_flags_set(&can_flags_grp, FLAG_PACKET_QUEUED, TX_OR); // Флаги сообщают о приеме пакета
    return RES_OK;
  }
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------


  \param parameter
-----------------------------------------------------------------------------------------------------*/
static void Thread_can(ULONG initial_input)
{
  T_can_tx_message tx_msg;
  T_can_rx_message rx_msg;
  ULONG  actual_flags;

  CAN_init(CAN0, CAN0_BITRATE);
  // Инициализация всех майлбоксов предназначенных для приема сообщений
  for (uint32_t n = 0; n < RX_MBOX_CNT; n++)
  {
    CAN_set_rx_mbox(g_ptr_CAN, rx_mboxes[n].mb_num, rx_mboxes[n].canid, rx_mboxes[n].canid_mask, 1, 0);
  }
  Create_can_task_sync_obj();

  while (1)
  {
    actual_flags = 0;
    // Ожидание флагов
    if (tx_event_flags_get(&can_flags_grp, FLAG_PACKET_QUEUED |  FLAG_TX_DONE | 0xFF, TX_OR_CLEAR ,&actual_flags, MS_TO_TICKS(10)) == TX_SUCCESS)
    {
      if (actual_flags & FLAG_PACKET_QUEUED)
      {
        if (can_tx_ready)
        {
          if (tx_queue_receive(&can_queue,&tx_msg, 0) == TX_SUCCESS)
          {
            can_tx_ready = 0;
            CAN_set_tx_mbox(g_ptr_CAN, CAN_TX_MB1, tx_msg.canid, tx_msg.data, tx_msg.len, CAN_EXTENDED_FORMAT, tx_msg.rtr);
          }
        }
      }

      if (actual_flags & FLAG_TX_DONE)
      {
        can_tx_ready = 1;
        if (tx_queue_receive(&can_queue,&tx_msg, 0) == TX_SUCCESS)
        {
          can_tx_ready = 0;
          CAN_set_tx_mbox(g_ptr_CAN, CAN_TX_MB1, tx_msg.canid, tx_msg.data, tx_msg.len, CAN_EXTENDED_FORMAT, tx_msg.rtr);
        }
      }

      // Обслуживание принятых пакетов
      if (actual_flags & 0xFF)
      {
        for (uint32_t n = 0; n < RX_MBOX_CNT; n++)
        {
          if (actual_flags & (1 << rx_mboxes[n].mb_num))
          {
            Set_output_can_active_blink(OUTP_LEDCAN0); // Информируем светодиодом об активности приема

            CAN_read_rx_mbox(g_ptr_CAN, rx_mboxes[n].mb_num,&rx_msg);

            if (rx_msg.canid == REQUEST_STATE_TO_ALL)
            {
               // Получить состояние системы и отправить в ответ
              Pack_sytem_state_to_can_msg(tx_msg.data);
              CAN_post_packet_to_send(BCKPC_STATE_PACKET, tx_msg.data, 8, 0);
            }
            else if (rx_msg.canid == BCKPC_CMD_PACKET)
            {
              // Выполнить команду
              Execute_can_command(rx_msg.data);
            }
          }
        }
      }
    } // if (tx_event_flags_get
  } // while(1)
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_CAN_create(void)
{
  tx_thread_create(&can_thread, "CAN", Thread_can,
    0,
    (void *)thread_can_stack, // stack_start
    THREAD_CAN_STACK_SIZE,   // stack_size
    THREAD_CAN_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
    THREAD_CAN_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}


