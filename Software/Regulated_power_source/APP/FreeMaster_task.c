#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"
#include "freemaster_serial_lpuart.h"


uint8_t myhandler(uint8_t /*nAppcmd*/, uint8_t * /*pData*/, uint16_t /*nDataLen*/);


FMSTR_TSA_TABLE_LIST_BEGIN()

FMSTR_TSA_TABLE(wvar_vars)
FMSTR_TSA_TABLE(app_vars)

FMSTR_TSA_TABLE_LIST_END()

TX_THREAD   freemaster_thread;
#pragma data_alignment=8
uint8_t     thread_freemaster_stack[THREAD_FREEMASTER_STACK_SIZE];
static void Thread_freemaster(ULONG initial_input);


#define     FREEMATER_FLAG_RX       BIT(0)
#define     FREEMATER_FLAG_TX       BIT(1)
static      TX_EVENT_FLAGS_GROUP    freemaster_flags;


#define FM_PIPE_RX_BUF_SIZE 64
#define FM_PIPE_TX_BUF_SIZE 1024
#define FM_PIPE_MAX_STR_LEN 512

#define FM_PIPE_PORT_NUM    0
#define FM_PIPE_CALLBACK    0
#define FM_PIPE_TYPE        FMSTR_PIPE_TYPE_ANSI_TERMINAL
FMSTR_ADDR                  pipeRxBuff;
FMSTR_PIPE_SIZE             pipeRxSize;
FMSTR_ADDR                  pipeTxBuff;
FMSTR_PIPE_SIZE             pipeTxSize;

FMSTR_HPIPE                 fm_pipe = NULL;
T_app_log_record            *p_log_rec;
char                        *log_str;
uint8_t                     f_unsent_record;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t FreeMaster_task_create(void)
{
  UINT res;
  res = tx_thread_create(&freemaster_thread, "FreeMaster", Thread_freemaster,
       0,
       (void *)thread_freemaster_stack, // stack_start
       THREAD_FREEMASTER_STACK_SIZE,   // stack_size
       THREAD_FREEMASTER_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
       THREAD_FREEMASTER_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
       TX_NO_TIME_SLICE,
       TX_AUTO_START);
  if (res != TX_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
static void FreeMaster_UART_rx_isr(uint32_t intf_num)
{
  FMSTR_SerialIsr();
  tx_event_flags_set(&freemaster_flags, FREEMATER_FLAG_RX, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param intf_num
-----------------------------------------------------------------------------------------------------*/
static void FreeMaster_UART_tx_isr(uint32_t intf_num)
{
  FMSTR_SerialIsr();
  //tx_event_flags_set(&freemaster_flags, FREEMATER_FLAG_TX, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------
  Обработка пользовательских комманд поступающих движку FreeMaster

 \param app_command

 \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t Freemaster_Command_Manager(uint16_t app_command)
{
  uint8_t  res   = 0;
  uint32_t len;
  uint8_t  *dbuf;

  // Получаем указатель на буфер с данными команды
  dbuf = FMSTR_GetAppCmdData(&len);

  switch (app_command)
  {
  case FMCMD_SET_DCDC_DAC:
    if (len >= 2)
    {
      uint16_t val;
      memcpy(&val, dbuf, 2);
      Set_DCDC_DAC_code(val);
    }
    break;
  case FMCMD_ENABLE_DCDC:
    Set_EN_DCDC(1);
    break;
  case FMCMD_DISABLE_DCDC:
    Set_EN_DCDC(0);
    break;
  case FMCMD_CHECK_LOG_PIPE:
    FMSTR_PipePuts(fm_pipe, "Log pipe checked.\r\n");
    break;
  case FMCMD_SAVE_WVARS:
    request_save_to_NV_mem = 1;
    break;
  case FMCMD_RESET_DEVICE:
    Reset_system();
    break;
  default:
    res = 0;
    break;
  }


  return res;
}


/*-----------------------------------------------------------------------------------------------------


  \param str
  \param max_str_len
  \param p_log_rec
-----------------------------------------------------------------------------------------------------*/
void Format_log_string(char *str, uint32_t max_str_len, T_app_log_record  *p_log_rec)
{
  if (p_log_rec->line_num != 0)
  {
    snprintf(str, max_str_len, "%06d.%06d %s (%s %d)\n\r",
      p_log_rec->time.sec, p_log_rec->time.usec,
      p_log_rec->msg,
      p_log_rec->func_name,
      p_log_rec->line_num);
  }
  else
  {
    snprintf(str, max_str_len, "%06d.%06d %s\n\r",
      p_log_rec->time.sec, p_log_rec->time.usec,
      p_log_rec->msg);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Freemaster_send_log_to_pipe(void)
{
  if (f_unsent_record != 0)
  {
    if (FMSTR_PipePuts(fm_pipe, log_str)!= FMSTR_TRUE) return;
    f_unsent_record = 0;
  }

  while (AppLog_get_tail_record(p_log_rec) != 0)
  {
    Format_log_string(log_str, FM_PIPE_MAX_STR_LEN, p_log_rec);
    if (FMSTR_PipePuts(fm_pipe, log_str)!= FMSTR_TRUE)
    {
      f_unsent_record = 1;
      return;
    }
  }

}

/*-------------------------------------------------------------------------------------------------------------
  Цикл движка FreeMaster
-------------------------------------------------------------------------------------------------------------*/
static void Thread_freemaster(ULONG initial_input)
{

  uint16_t app_command;
  uint8_t  res = 0;
  ULONG    actual_flags;

  tx_event_flags_create(&freemaster_flags, "FreeMaster");
  BCKUPC10_UART_Init(UART0, FREEMASTER_UART_SPEED, UART_NO_PARITY, UART_STOP_1BIT, UART_HW_FLOW_CONTROL_MODE,FreeMaster_UART_rx_isr,FreeMaster_UART_tx_isr);
  BCKUPC10_UART_Set_TX_state(UART0, 1);
  BCKUPC10_UART_Set_RX_state(UART0, 1);
  BCKUPC10_UART_Set_RX_Int(UART0, 1);

  FMSTR_SerialSetBaseAddress(LPUART0);

  if (!FMSTR_Init())
  {
    return;
  }


  pipeRxSize = FM_PIPE_RX_BUF_SIZE;
  pipeRxBuff = App_malloc(pipeRxSize);
  pipeTxSize = FM_PIPE_TX_BUF_SIZE;
  pipeTxBuff = App_malloc(pipeTxSize);
  p_log_rec  = App_malloc(sizeof(T_app_log_record));
  log_str    = App_malloc(FM_PIPE_MAX_STR_LEN);
  if ((pipeRxBuff != NULL) && (pipeTxBuff != NULL) && (p_log_rec != NULL) && (log_str != NULL))
  {
    fm_pipe = FMSTR_PipeOpen(FM_PIPE_PORT_NUM, FM_PIPE_CALLBACK,  pipeRxBuff, pipeRxSize, pipeTxBuff, pipeTxSize, FM_PIPE_TYPE, "SysLog");
  }

  while (1)
  {
    app_command = FMSTR_GetAppCmd();

    if (app_command != FMSTR_APPCMDRESULT_NOCMD)
    {
      res = Freemaster_Command_Manager(app_command);
      FMSTR_AppCmdAck(res);
    }
    if (tx_event_flags_get(&freemaster_flags, 0xFFFFFFFF, TX_OR_CLEAR ,&actual_flags, MS_TO_TICKS(10)) == TX_SUCCESS)
    {
      FMSTR_Poll();
      if (fm_pipe != NULL)
      {
        Freemaster_send_log_to_pipe();
      }
    }
  }
}


