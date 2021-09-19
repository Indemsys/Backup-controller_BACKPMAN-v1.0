#ifndef __BCKUPC10_DMA
#define __BCKUPC10_DMA


// Определения для семейства микроконтроллеров KE1xF

#define DMA_CHANNELS_COUNT  16
#define MAX_DMA_SPI_BUFF    2048

// Номера источников запросов для мультиплесора MUX запросов DMA
// 16-ть регистров DMAMUX->CHCFG управляют мультиплексированием запросов с 63 периферийных сигналов на 16 входов контролера DMA

#define  DMUX_SRC__DISABLED        0   // Channel disabled1
#define  DMUX_SRC_RESERVED1        1   //  —
#define  DMUX_SRC_LPUART0_RX       2   // Receive
#define  DMUX_SRC_LPUART0_TX       3   // Transmit
#define  DMUX_SRC_LPUART1_RX       4   // Receive
#define  DMUX_SRC_LPUART1_TX       5   // Transmit
#define  DMUX_SRC_LPUART2_RX       6   // Receive
#define  DMUX_SRC_LPUART2_TX       7   // Transmit
#define  DMUX_SRC_RESERVED2        8   //  —
#define  DMUX_SRC_RESERVED3        9   //  —
#define  DMUX_SRC_FLEXIO_SH0       10  // Shifter0
#define  DMUX_SRC_FLEXIO_SH1       11  // Shifter1
#define  DMUX_SRC_FLEXIO_SH2       12  // Shifter2
#define  DMUX_SRC_FLEXIO_SH3       13  // Shifter3
#define  DMUX_SRC_LPSPI0_RX        14  // Receive
#define  DMUX_SRC_LPSPI0_TX        15  // Transmi
#define  DMUX_SRC_LPSPI1_RX        16  // Receive
#define  DMUX_SRC_LPSPI1_TX        17  // Transmi
#define  DMUX_SRC_LPI2C0_RX        18  // Receive
#define  DMUX_SRC_LPI2C0_TX        19  // Transmit
#define  DMUX_SRC_FTM0_CH0         20  // Channel 0
#define  DMUX_SRC_FTM0_CH1         21  // Channel 1
#define  DMUX_SRC_FTM0_CH2         22  // Channel 2
#define  DMUX_SRC_FTM0_CH3         23  // Channel 3
#define  DMUX_SRC_FTM0_CH4         24  // Channel 4
#define  DMUX_SRC_FTM0_CH5         25  // Channel 5
#define  DMUX_SRC_FTM0_CH6         26  // Channel 6
#define  DMUX_SRC_FTM0_CH7         27  // Channel 7
#define  DMUX_SRC_FTM1_CH0         28  // Channel 0
#define  DMUX_SRC_FTM1_CH1         29  // Channel 1
#define  DMUX_SRC_FTM2_CH0         30  // Channel 0
#define  DMUX_SRC_FTM2_CH1         31  // Channel 1
#define  DMUX_SRC_LPI2C1_FTM3_CH0  32  // LPI2C1 Receiveor FTM3 Channel 0
#define  DMUX_SRC_LPI2C1_FTM3_CH1  33  // LPI2C1 Transmitor FTM3 Channel 1
#define  DMUX_SRC_FTM3_CH2         34  // Channel 2
#define  DMUX_SRC_FTM3_CH3         35  // Channel 3
#define  DMUX_SRC_FTM3_CH4         36  // Channel 4
#define  DMUX_SRC_FTM3_CH5         37  // Channel 5
#define  DMUX_SRC_FTM3_CH6         38  // Channel 6
#define  DMUX_SRC_FTM3_CH7         39  // Channel 7
#define  DMUX_SRC_ADC0             40  // ADC0 COCO
#define  DMUX_SRC_ADC1             41  // ADC1 COCO
#define  DMUX_SRC_ADC2             42  // ADC2 COCO
#define  DMUX_SRC_CMP0             43  // — Yes
#define  DMUX_SRC_CMP1             44  // — Yes
#define  DMUX_SRC_CMP2             45  // — Yes
#define  DMUX_SRC_PDB0             46  // —
#define  DMUX_SRC_PDB1             47  // —
#define  DMUX_SRC_PDB2             48  // —
#define  DMUX_SRC_PORTA            49  // control module Port A
#define  DMUX_SRC_PORTB            50  // control module Port B
#define  DMUX_SRC_PORTC            51  // control module Port C
#define  DMUX_SRC_PORTD            52  // control module Port D
#define  DMUX_SRC_PORTE            53  // control module Port E
#define  DMUX_SRC_FLEXCAN02        54  // FlexCAN0
#define  DMUX_SRC_FLEXCAN12        55  // FlexCAN1
#define  DMUX_SRC_DAC0             56  // DAC0 Yes
#define  DMUX_SRC_FTM1             57  // OR of ch2 - ch7
#define  DMUX_SRC_FTM2             58  // OR of ch2 - ch7
#define  DMUX_SRC_LPTMR0           59  // — Yes
#define  DMUX_SRC_DMAMUX1          60  // Always enabled
#define  DMUX_SRC_DMAMUX2          61  // Always enabled
#define  DMUX_SRC_DMAMUX3          62  // Always enabled
#define  DMUX_SRC_DMAMUX4          63  // Always enabled


#define DMA_1BYTE_MINOR_TRANSFER    1
#define DMA_2BYTE_MINOR_TRANSFER    2
#define DMA_4BYTE_MINOR_TRANSFER    4


typedef struct {
  uint32_t SADDR;
  uint16_t SOFF;
  uint16_t ATTR;
  union {
    uint32_t NBYTES_MLNO;
    uint32_t NBYTES_MLOFFNO;
    uint32_t NBYTES_MLOFFYES;
  };
  uint32_t SLAST;
  uint32_t DADDR;
  uint16_t DOFF;
  union {
    uint16_t CITER_ELINKNO;
    uint16_t CITER_ELINKYES;
  };
  uint32_t DLAST_SGA;
  uint16_t CSR;
  union {
    uint16_t BITER_ELINKNO;
    uint16_t BITER_ELINKYES;
  };
} T_DMA_TCD;


typedef struct
{
  uint8_t             dma_channel;    // номер канала DMA пересылающий данные из FIFO SPI в буффер с данными
  uint8_t             *databuf;       // указатель на исходный буфер с данными
  uint32_t            datasz;         // размер исходного буфера данных
  uint32_t            periph_port;    // адрес регистра порта периферии
  DMAMUX_Type         *dma_mux;
  uint32_t            dmux_src;       // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA. Например DMUX0_SRC_SPI0_RX
  uint8_t             minor_tranf_sz; // Количество пересылаемых байт в минорном цикле

} T_DMA_config;



typedef struct
{
  FTM_Type            *FTM;        // Указатель на таймер
  uint8_t             ftm_ch;     // Номер канала таймера
  uint8_t             dma_ch;     // номер канала DMA пересылающий данные из буффер с данными канал таймера
  uint32_t            saddr;      // указатель на массив с данными
  uint32_t            arrsz;      // размер массива данных
  uint32_t            daddr;      // адрес назаначения - регистр канала таймера
  DMAMUX_Type         *dma_mux;     // Указатель на структуру мультиплексора
  uint32_t            dmux_src;   // номер входа периферии для выбранного мультиплексора DMAMUX для передачи на DMA.

} T_DMA_WS2812B_config;



typedef struct
{
  uint8_t       dma_tx_channel;   // Номер дескриптора DMA на который указывает mem_ch
  uint8_t       dma_rx_channel;   // Номер дескриптора DMA предлназначенного для приема
  uint8_t       minor_tranf_sz;   // Количество пересылаемых байт в минорном цикле

} T_DMA_cbl;


void Init_DMA(void);


void Config_DMA_TX(T_DMA_config *cfg, T_DMA_cbl *pDS_cbl);
void Config_DMA_RX(T_DMA_config *cfg, T_DMA_cbl *pDS_cbl);

void Start_DMA_TX_w_dummyRX(T_DMA_cbl *pDS_cbl, void *buf, uint32_t sz);
void Start_DMA_RX_w_dummyTX(T_DMA_cbl *pDS_cbl, void *buf, uint32_t sz);
void Start_DMA_RXTX(T_DMA_cbl *pDS_cbl, void *outbuf, void *inbuf, uint32_t sz);

#endif
