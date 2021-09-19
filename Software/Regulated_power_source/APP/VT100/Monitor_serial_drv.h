#ifndef __MONITOR_SERIAL_DRV
  #define __MONITOR_SERIAL_DRV

extern T_monitor_driver uart0_driver;
extern T_monitor_driver uart1_driver;
extern T_monitor_driver uart2_driver;


#define UART0_BAUD_RATE   115200  
#define UART1_BAUD_RATE   115200  
#define UART2_BAUD_RATE   115200  


#define UART_HW_FLOW_CONTROL_MODE     UART_DISABLE_HW_FLOW_CTRL

T_monitor_driver *Mnsdrv_get_uart0_driver(void);
T_monitor_driver *Mnsdrv_get_uart1_driver(void);
T_monitor_driver *Mnsdrv_get_uart2_driver(void);


#endif
