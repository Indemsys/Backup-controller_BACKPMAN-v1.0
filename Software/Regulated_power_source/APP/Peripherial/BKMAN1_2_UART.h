#ifndef BACKPMAN10_UART_H
  #define BACKPMAN10_UART_H

  #define UART0   0
  #define UART1   1
  #define UART2   2

  #define DEFAULT_TX_ISR   0
  #define DEFAULT_RX_ISR   0


  #define UART_STOP_1BIT   0
  #define UART_STOP_2BIT   1


  #define UART_NO_PARITY   0
  #define UART_EVEN_PARITY 1
  #define UART_ODD_PARITY  2


  #define UART_DISABLE_HW_FLOW_CTRL  0
  #define UART_ENABLE_HW_FLOW_CTRL   1

typedef void (*T_UART_isr)(uint32_t intf_num);

typedef void (*T_uart_tx_callback)(uint8_t portn);
typedef void (*T_uart_rx_callback)(uint8_t portn, uint8_t ch);
typedef void (*T_uart_tx_compl_callback)(uint8_t portn);


typedef struct
{

    T_uart_tx_callback          uart_tx_callback;
    T_uart_rx_callback          uart_rx_callback;
    T_uart_tx_compl_callback    uart_tx_compl_callback;

} T_UART_cbl;



LPUART_Type*      BCKUPC10_get_UART(uint32_t intf_num);
void              BCKUPC10_UART_Init(uint32_t intf_num, uint32_t  baud, uint8_t  parity, uint8_t  stops, uint8_t en_hw_flow_ctrl, T_UART_isr rx_isr_ptr, T_UART_isr tx_isr_ptr);
void              BCKUPC10_UART_Set_TX_state(uint32_t intf_num, uint8_t flag);
void              BCKUPC10_UART_Set_RX_state(uint32_t intf_num, uint8_t flag);
void              BCKUPC10_UART_Set_RX_Int(uint32_t intf_num, uint8_t flag);
void              BCKUPC10_UART_Set_TX_Int(uint32_t intf_num, uint8_t flag);
void              BCKUPC10_UART_Set_TXC_Int(uint32_t intf_num, uint8_t flag);
void              BCKUPC10_UART_clear_rx_fifo(uint32_t intf_num);
uint8_t           BCKUPC10_UART_Get_TXEMPT_flag(uint32_t intf_num);
void              BCKUPC10_UART_enable_tx_DMA(uint32_t intf_num);
void              BCKUPC10_UART_Set_txrx_callback(uint32_t intf_num, T_uart_rx_callback rx_callback, T_uart_tx_callback tx_callback);
void              BCKUPC10_UART_Set_tc_callback(uint32_t intf_num, T_uart_tx_compl_callback tc_callback);
void              BCKUPC10_UART_send_byte(uint32_t intf_num, uint8_t b);
void              BCKUPC10_UART_DeInit(uint32_t intf_num);


#endif // BCKUPC10_UART_H



