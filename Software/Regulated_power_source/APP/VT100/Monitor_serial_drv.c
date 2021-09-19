#include "App.h"

#define  UART0_DRIVER   0
#define  UART1_DRIVER   1
#define  UART2_DRIVER   2

static int Mnsdrv_init(void **pcbl, void *pdrv);
static int Mnsdrv_send_buf(const void *buf, unsigned int len);
static int Mnsdrv_wait_ch(unsigned char *b, int timeout);
static int Mnsdrv_printf(const char  *fmt_ptr, ...);
static int Mnsdrv_deinit(void **pcbl);


T_monitor_driver uart0_driver =
{
  MN_DRIVER_MARK,
  UART0_DRIVER,
  Mnsdrv_init,
  Mnsdrv_send_buf,
  Mnsdrv_wait_ch,
  Mnsdrv_printf,
  Mnsdrv_deinit,
  0,
};

T_monitor_driver uart1_driver =
{
  MN_DRIVER_MARK,
  UART1_DRIVER,
  Mnsdrv_init,
  Mnsdrv_send_buf,
  Mnsdrv_wait_ch,
  Mnsdrv_printf,
  Mnsdrv_deinit,
  0,
};

T_monitor_driver uart2_driver =
{
  MN_DRIVER_MARK,
  UART2_DRIVER,
  Mnsdrv_init,
  Mnsdrv_send_buf,
  Mnsdrv_wait_ch,
  Mnsdrv_printf,
  Mnsdrv_deinit,
  0,
};

#define SER_DRIVERS_NUM  3
static const T_monitor_driver  *ser_drivers[SER_DRIVERS_NUM] = {&uart0_driver,&uart1_driver,&uart2_driver};


#define INPUT_BUF_SZ    128  // Должно быть степенью 2
#define OUTPUT_BUF_SZ   128  // Должно быть степенью 2
#define OUT_STR_MAX_SZ  256

typedef struct
{
    uint8_t                 port_num;
    uint8_t                 input_buf[INPUT_BUF_SZ];
    volatile uint32_t       input_buf_head;
    volatile uint32_t       input_buf_tail;
    volatile uint32_t       inbuf_data_cnt;

    uint8_t                 output_buf[OUTPUT_BUF_SZ];
    volatile uint32_t       output_buf_head;
    volatile uint32_t       output_buf_tail;
    volatile uint32_t       outbuf_data_cnt;

    char                    output_str[OUT_STR_MAX_SZ];
    TX_EVENT_FLAGS_GROUP    uart_flags_grp;

} T_ser_drv_cbl;


#define  TRANCEIVER_BUF_READY   BIT(0)
#define  RECEVED_DATA           BIT(1)
/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерывания возникающее в случае готовности к отправке новых данных

  \param portn
-----------------------------------------------------------------------------------------------------*/
static void uart_tx_callback(uint8_t portn)
{
  uint8_t b;
  T_ser_drv_cbl *p;
  p = ser_drivers[portn]->pdrvcbl;
  uint32_t tail = p->output_buf_tail;
  uint32_t cnt;

  cnt = p->outbuf_data_cnt;

  if (cnt == 0)
  {
    BCKUPC10_UART_Set_TX_Int(portn, 0); // Запретим прерывания если нечего передавать
  }
  else
  {
    b =  p->output_buf[tail];
    BCKUPC10_UART_send_byte(portn,b);
    cnt--;
    tail++;
    tail &=(OUTPUT_BUF_SZ - 1);
    if (cnt == 0)
    {
      BCKUPC10_UART_Set_TX_Int(portn, 0); // Запретим прерывания если нечего передавать
    }
    p->output_buf_tail = tail;
    p->outbuf_data_cnt = cnt;
  }
  tx_event_flags_set(&p->uart_flags_grp, TRANCEIVER_BUF_READY, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------
  Обслуживание прерывания при поступлении новых данных

  \param portn
  \param ch
-----------------------------------------------------------------------------------------------------*/
static void uart_rx_callback(uint8_t portn, uint8_t ch)
{
  T_ser_drv_cbl *p;
  p = ser_drivers[portn]->pdrvcbl;
  uint32_t head = p->input_buf_head;
  uint32_t cnt = p->inbuf_data_cnt;

  // Получили байт, сохраняем в кольцевом буфере
  if (cnt != INPUT_BUF_SZ)
  {
    p->input_buf[head] = ch;
    head++;
    head &=(INPUT_BUF_SZ - 1);
    cnt++;
    p->input_buf_head = head;
    p->inbuf_data_cnt = cnt;
  }
  // Послать сообщение о приеме байта
  tx_event_flags_set(&p->uart_flags_grp, RECEVED_DATA, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


  \param pcbl  - укзатель на структуру  T_ser_drv_cbl.    Эта структура с внутренними переменными драйвера
  \param pdrv  - указатель на структуру T_monitor_driver  Это структура с идентификаторовм драйвера, с указателями на функции драйвера и указателями на внутрению структуру

  \return int
-----------------------------------------------------------------------------------------------------*/
static int Mnsdrv_init(void **pcbl, void *pdrv)
{
  T_monitor_driver *driver;
  uint32_t          uart_num = 0;
  uint32_t          baud_rate = 0;

  driver = (T_monitor_driver *)pdrv;

  switch (driver->driver_type)
  {
  case UART0_DRIVER:
    uart_num = UART0;
    baud_rate = UART0_BAUD_RATE;
    break;
  case UART1_DRIVER:
    uart_num = UART1;
    baud_rate = UART1_BAUD_RATE;
    break;
  case UART2_DRIVER:
    uart_num = UART2;
    baud_rate = UART2_BAUD_RATE;
    break;
  }


  BCKUPC10_UART_Set_txrx_callback(uart_num, uart_rx_callback, uart_tx_callback);
  BCKUPC10_UART_Init(uart_num, baud_rate, UART_NO_PARITY, UART_STOP_1BIT, UART_HW_FLOW_CONTROL_MODE,DEFAULT_RX_ISR,DEFAULT_TX_ISR);
  BCKUPC10_UART_Set_TX_state(uart_num, 1);
  BCKUPC10_UART_Set_RX_state(uart_num, 1);
  BCKUPC10_UART_Set_RX_Int(uart_num, 1);



  // Если драйвер еще не был инициализирован, то выделить память для управлющей структуры и ждать сигнала из интерфеса
  if (*pcbl == 0)
  {
    T_ser_drv_cbl *p;

    // Выделяем память для управляющей структуры драйвера
    p = (T_ser_drv_cbl *)App_malloc(sizeof(T_ser_drv_cbl));

    tx_event_flags_create(&p->uart_flags_grp, "VT100");
    tx_event_flags_set(&p->uart_flags_grp, TRANCEIVER_BUF_READY, TX_OR);
    p->port_num = uart_num;

    *pcbl = p; //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
  }
  return RES_OK;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static int Mnsdrv_deinit(void **pcbl)
{
  App_free(*pcbl);
  *pcbl = 0;
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_monitor_driver*
-----------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnsdrv_get_uart0_driver(void)
{
  return &uart0_driver;
}
T_monitor_driver* Mnsdrv_get_uart1_driver(void)
{
  return &uart1_driver;
}
T_monitor_driver* Mnsdrv_get_uart2_driver(void)
{
  return &uart2_driver;
}



/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnsdrv_send_buf(const void *buf, unsigned int len)
{
  ULONG        actual_flags;
  T_ser_drv_cbl *p;
  GET_MCBL;
  p = (T_ser_drv_cbl *)(mcbl->pdrv->pdrvcbl);
  uint32_t head;
  uint32_t cnt;
  uint32_t indx = 0;

  if (len == 0) return RES_OK;

  do
  {
    tx_event_flags_get(&p->uart_flags_grp, TRANCEIVER_BUF_READY, TX_AND_CLEAR,&actual_flags,  TX_WAIT_FOREVER); // Ждать бесконечно флага свободного буфера для передачи

    BCKUPC10_UART_Set_TX_Int(p->port_num, 0);

    cnt = p->outbuf_data_cnt;
    if (cnt != OUTPUT_BUF_SZ)
    {

      head = p->output_buf_head;
      do
      {
        p->output_buf[head] =((uint8_t *)buf)[indx];
        indx++;
        len--;
        head++;
        head &=(OUTPUT_BUF_SZ - 1);
        p->output_buf_head = head;
        cnt++;
        if (cnt == OUTPUT_BUF_SZ) break;
      } while (len > 0);

      p->outbuf_data_cnt = cnt;
    }

    BCKUPC10_UART_Set_TX_Int(p->port_num, 1);

  } while (len > 0);

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnsdrv_wait_ch(unsigned char *b, int timeout)
{
  ULONG        actual_flags;
  T_ser_drv_cbl   *p;
  GET_MCBL;
  p = (T_ser_drv_cbl *)(mcbl->pdrv->pdrvcbl);
  uint32_t tail;
  uint32_t cnt;

  cnt = p->inbuf_data_cnt;
  if (cnt == 0)
  {
    // Ждать  флага появления данных
    if (tx_event_flags_get(&p->uart_flags_grp, RECEVED_DATA, TX_AND_CLEAR,&actual_flags,  MS_TO_TICKS(timeout)) != TX_SUCCESS) return RES_ERROR;
    BCKUPC10_UART_Set_RX_Int(p->port_num, 0); //
    cnt = p->inbuf_data_cnt;
  }

  if (cnt > 0)
  {
    BCKUPC10_UART_Set_RX_Int(p->port_num, 0); //
    tail = p->input_buf_tail;
    cnt = p->inbuf_data_cnt;
    *b = p->input_buf[tail];
    tail++;
    tail &=(INPUT_BUF_SZ - 1);
    p->input_buf_tail = tail;
    cnt--;
    p->inbuf_data_cnt = cnt;
    if (cnt == 0) tx_event_flags_get(&p->uart_flags_grp, RECEVED_DATA, TX_AND_CLEAR,&actual_flags, 0); // Очищаем флаг если был принят последний байт
    BCKUPC10_UART_Set_RX_Int(p->port_num, 1);
    return RES_OK;
  }

  BCKUPC10_UART_Set_RX_Int(p->port_num, 1);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnsdrv_printf(const char  *fmt_ptr, ...)
{
  int32_t       n = 0;
  va_list       ap;
  T_ser_drv_cbl *p;
  GET_MCBL;

  p = (T_ser_drv_cbl *)(mcbl->pdrv->pdrvcbl);

  va_start(ap, fmt_ptr);
  n = vsnprintf(p->output_str, OUT_STR_MAX_SZ, (char *)fmt_ptr, ap);
  va_end(ap);
  Mnsdrv_send_buf(p->output_str, n);
  return n;
}

