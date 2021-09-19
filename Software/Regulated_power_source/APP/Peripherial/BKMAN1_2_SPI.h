#ifndef BCKUPC10_SPI_H
  #define BCKUPC10_SPI_H

#define SPI_INTFS_NUM  2

#define SPI_SPEED_30000  1
#define SPI_SPEED_15000  2
#define SPI_SPEED_7500   4
#define SPI_SPEED_3750   8
#define SPI_SPEED_1875   16
#define SPI_SPEED_938    32
#define SPI_SPEED_469    64
#define SPI_SPEED_234    128

#define SPI_FLAG_FRAME_COMPLETE  BIT(0)

typedef void (*T_SPI_isr)(uint32_t intf_num);


LPSPI_Type* Get_SPI_type(uint32_t intf_num);
void        SPI_Init(uint32_t intf_num);
uint32_t    SPI_wait_frame_complete(uint32_t intf_num);

#endif // BCKUPC10_SPI_H



