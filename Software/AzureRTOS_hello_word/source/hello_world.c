#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "tx_API.h"

#define THREAD_MAIN_STACK_SIZE             1024
#define THREAD_1_STACK_SIZE                1024
#define THREAD_2_STACK_SIZE                1024
#define THREAD_MAIN_PRIORITY               3
#define THREAD_1_PRIORITY                  1
#define THREAD_2_PRIORITY                  2

TX_THREAD               main_thread;
TX_THREAD               thread1;
TX_THREAD               thread2;
#pragma data_alignment=8
uint8_t                 thread_main_stack[THREAD_MAIN_STACK_SIZE];
#pragma data_alignment=8
uint8_t                 thread_1_stack[THREAD_1_STACK_SIZE];
#pragma data_alignment=8
uint8_t                 thread_2_stack[THREAD_2_STACK_SIZE];


static void             Thread_main(ULONG initial_input);
static void             Thread_1(ULONG initial_input);
static void             Thread_2(ULONG initial_input);
/*-----------------------------------------------------------------------------------------------------
  Модифицированная функция из SDK

  \param void

  \return int
-----------------------------------------------------------------------------------------------------*/
int main(void)
{
  /* Init board hardware. */
  BOARD_InitPins();
  BOARD_InitBootClocks();
  BOARD_InitDebugConsole();
  tx_kernel_enter();
}

/*-----------------------------------------------------------------------------------------------------
  Вспомогательная функция для создания первичной задачи

  \param first_unused_memory
-----------------------------------------------------------------------------------------------------*/
void    tx_application_define(void *first_unused_memory)
{
  tx_thread_create(&main_thread, "Main", Thread_main,
    0,
    (void *)thread_main_stack, // stack_start
    THREAD_MAIN_STACK_SIZE,    // stack_size

    THREAD_MAIN_PRIORITY,     // priority.
                              // Numerical priority of thread.
                              // Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.

    THREAD_MAIN_PRIORITY,     // preempt_threshold.
                              // Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption.
                              // Only priorities higher than this level are allowed to preempt this thread.
                              // This value must be less than or equal to the specified priority.
                              // A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
}

/*-----------------------------------------------------------------------------------------------------
  Первичная задача

  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_main(ULONG initial_input)
{
  char ch;
  PRINTF("hello world.\r\n");

  tx_thread_create(&thread1, "Thread1", Thread_1, 0, (void *)thread_1_stack, THREAD_1_STACK_SIZE, THREAD_1_PRIORITY, THREAD_1_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
  tx_thread_create(&thread2, "Thread2", Thread_2, 0, (void *)thread_2_stack, THREAD_2_STACK_SIZE, THREAD_2_PRIORITY, THREAD_2_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);


  while (1)
  {
    ch = GETCHAR();
    PUTCHAR(ch);
    if (ch == 0x1B)  // ESC
    {
      tx_thread_terminate(&main_thread);
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
  Две задачи для проверки в отладчике ленивого сохранения контекста FPU

-----------------------------------------------------------------------------------------------------*/
volatile float fv;
static void Thread_1(ULONG initial_input)
{
  for (;;)
  {
    fv = fv + 0.1f;
    _tx_thread_sleep(1);
  }
}

static void Thread_2(ULONG initial_input)
{
  for (;;)
  {
    _tx_thread_sleep(1);
  }
}

