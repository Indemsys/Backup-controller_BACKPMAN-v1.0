#ifndef __BCKUPC20_CAN_H
  #define __BCKUPC20_CAN_H

  #define CAN_DEVICE  0

  #define CAN_SPEED   555555ul //

  #define CAN_MIN_TQ  8
  #define CAN_MAX_TQ  25

  #define CAN_MAX_PROPSEG 8
  #define CAN_MAX_RJW 4
  #define CAN_MIN_PSEG2 2


// Майлбоксы на чтение
  #define CAN_RX_MB1     0
  #define CAN_RX_MB2     1
  #define CAN_RX_MB3     2
  #define CAN_RX_MB4     3
  #define CAN_RX_MB5     4
  #define CAN_RX_MB6     5
  #define CAN_RX_MB7     6
  #define CAN_RX_MB8     7
// Всего майлбоксов на чтение
  #define RX_MBOX_CNT    8

// Майлбокс на передачу
  #define CAN_TX_MB1     8

// Количество сообщений в логе приемника
  #define CAN_RX_LOG_SZ  128

  #define CAT_TX_QUEUE_SZ  32

  #define CAN_RX_WIAT_MS   10

#define CAN_EXTENDED_FORMAT 1
#define CAN_BASE_FORMAT     0
#define CAN_RTR             1
#define CAN_NO_RTR          0

typedef struct
{
  uint32_t pseg1;
  uint32_t pseg2;
  uint32_t propseg;
  uint32_t rjw;
  uint32_t presdiv;
}
T_can_ctrl;


typedef struct
{
  uint32_t canid;     //Полученый идентификатор CAN пакета
  uint8_t  data[8]; //Буффер с данными длинной 8 байт
  uint8_t  len;     //Длина данных
  uint8_t  ext;     //Формат идентификатора(расширенный = 1, стандартный = 0)
  uint8_t  rtr;     //Признак  Remote Request Frame
  uint8_t  code;
}
T_can_rx_message;

typedef struct
{
  unsigned char  mb_num;
  uint32_t canid;
  uint32_t canid_mask;

} T_can_rx_config;


typedef struct
{
  uint32_t canid;
  uint8_t  data[8];
  uint8_t  len;
  uint8_t  rtr;  // Флаг rtr в can пакете
  uint8_t  rsv1; // Байлы для приведения размера типа кратному 32-битному слову
  uint8_t  rsv2;
}
T_can_tx_message;

typedef struct
{
  uint32_t tx_err_cnt;
}
T_can_statistic;


typedef void (*T_can_processing)(void);

uint32_t CAN_post_packet_to_send(uint32_t canid, uint8_t* data, uint8_t len, uint32_t timeout_ms);
uint32_t CAN_post_rtr_packet_to_send(uint32_t canid, uint8_t* data, uint8_t len, uint32_t timeout_ms);
void     Thread_CAN_create(void);
#endif
