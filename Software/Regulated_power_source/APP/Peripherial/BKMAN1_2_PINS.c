#include "App.h"


int32_t  st_oledp;
int32_t  st_oledv;
int32_t  st_oled_cs;
int32_t  st_oled_res;
int32_t  st_oled_dc;
int32_t  st_led_gr;
int32_t  st_led_rd;
int32_t  st_out1;
int32_t  st_out2;
int32_t  st_invsw_on;
int32_t  st_pids_shdn;
int32_t  st_aids_shdn;
int32_t  st_aids_fbc;
int32_t  st_en_DCDC;
int32_t  st_dcdc_mode;
int32_t  st_sw_inv_path;
int32_t  st_sw_mains_path;

//   Переменные sig_xxx  предназначены для управления выходами из FreeMaster
int32_t  sig_oledp;
int32_t  sig_oledv;
int32_t  sig_oled_cs;
int32_t  sig_oled_res;
int32_t  sig_oled_dc;
int32_t  sig_led_gr;
int32_t  sig_led_rd;
int32_t  sig_out1;
int32_t  sig_out2;
int32_t  sig_invsw_on;
int32_t  sig_pids_shdn;
int32_t  sig_aids_shdn;
int32_t  sig_aids_fbc;
int32_t  sig_en_DCDC;
int32_t  sig_dcdc_mode;
int32_t  sig_sw_inv_path;
int32_t  sig_sw_mains_path;


// Настройки линий ввода/вывода платы LNDC v1.01 на базе микроконтроллера MKE18F512VLL16
const T_IO_pins_configuration BCKPC01_pins_conf[] =
{
  //gpio   port   num    irqc       lock mux    DSE      ODE     PFE      PUPD      dir     init
  { GPIOA, PORTA,   0,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   1 }, //OLED_CLK           79 PTA0 Def=ADC0_SE0/ACMP0_IN0 ALT0=ADC0_SE0/ACMP0_IN0 ALT1=PTA0 ALT2=FTM2_CH1 ATL3=LPI2C0_SCLS ALT4=FXIO_D2 ALT5=FTM2_QD_PHA ALT6=LPUART0_CTS ALT7=TRGMUX_OUT3
  { GPIOA, PORTA,   1,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   1 }, //OLED_DIN           78 PTA1 Def=ADC0_SE1/ACMP0_IN1 ALT0=ADC0_SE1/ACMP0_IN1 ALT1=PTA1 ALT2=FTM1_CH1 ATL3=LPI2C0_SDAS ALT4=FXIO_D3 ALT5=FTM1_QD_PHA ALT6=LPUART0_RTS ALT7=TRGMUX_OUT0
  { GPIOA, PORTA,   2,   IRQ_DIS,   0,   ALT6,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART0_RX           73 PTA2 Def=ADC1_SE0 ALT0=ADC1_SE0 ALT1=PTA2 ALT2=FTM3_CH0 ATL3=LPI2C0_SDA ALT4=EWM_OUT_b ALT5= ALT6=LPUART0_RX ALT7=
  { GPIOA, PORTA,   3,   IRQ_DIS,   0,   ALT6,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART0_TX           72 PTA3 Def=ADC1_SE1 ALT0=ADC1_SE1 ALT1=PTA3 ALT2=FTM3_CH1 ATL3=LPI2C0_SCL ALT4=EWM_IN ALT5= ALT6=LPUART0_TX ALT7=
  //GPIOA, PORTA,   4,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SWD_DIO            98 PTA4 Def=JTAG_TMS/SWD_DIO ALT0= ALT1=PTA4 ALT2= ATL3= ALT4=ACMP0_OUT ALT5=EWM_OUT_b ALT6= ALT7=JTAG_TMS/SWD_DIO
  //GPIOA, PORTA,   5,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //Reset              97 PTA5 Def=RESET_b ALT0= ALT1=PTA5 ALT2= ATL3=TCLK1 ALT4= ALT5= ALT6=JTAG_TRST_b ALT7=RESET_b
  { GPIOA, PORTA,   6,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  58 PTA6 Def=ADC0_SE2/ACMP1_IN0 ALT0=ADC0_SE2/ACMP1_IN0 ALT1=PTA6 ALT2=FTM0_FLT1 ATL3=LPSPI1_PCS1 ALT4= ALT5= ALT6=LPUART1_CTS ALT7=
  { GPIOA, PORTA,   7,   IRQ_DIS,   0,   ALT6,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART1_RTS          57 PTA7 Def=ADC0_SE3/ACMP1_IN1 ALT0=ADC0_SE3/ACMP1_IN1 ALT1=PTA7 ALT2=FTM0_FLT2 ATL3= ALT4=RTC_CLKIN ALT5= ALT6=LPUART1_RTS ALT7=
  { GPIOA, PORTA,   8,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  100 PTA8 Def=DISABLED ALT0= ALT1=PTA8 ALT2= ATL3= ALT4=FXIO_D6 ALT5=FTM3_FLT3 ALT6= ALT7=
  { GPIOA, PORTA,   9,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  99 PTA9 Def=DISABLED ALT0= ALT1=PTA9 ALT2= ATL3= ALT4=FXIO_D7 ALT5=FTM3_FLT2 ALT6=FTM1_FLT3 ALT7=
  //GPIOA, PORTA,  10,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SWO                92 PTA10 Def=JTAG_TDO/noetm_Trace_SWO ALT0= ALT1=PTA10 ALT2=FTM1_CH4 ATL3=LPUART0_TX ALT4=FXIO_D0 ALT5= ALT6= ALT7=JTAG_TDO/noetm_Trace_SWO
  { GPIOA, PORTA,  11,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  91 PTA11 Def=DISABLED ALT0= ALT1=PTA11 ALT2=FTM1_CH5 ATL3=LPUART0_RX ALT4=FXIO_D1 ALT5= ALT6= ALT7=
  { GPIOA, PORTA,  12,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  90 PTA12 Def=ADC2_SE5 ALT0=ADC2_SE5 ALT1=PTA12 ALT2=FTM1_CH6 ATL3=CAN1_RX ALT4=LPI2C1_SDAS ALT5= ALT6= ALT7=
  { GPIOA, PORTA,  13,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  89 PTA13 Def=ADC2_SE4 ALT0=ADC2_SE4 ALT1=PTA13 ALT2=FTM1_CH7 ATL3=CAN1_TX ALT4=LPI2C1_SCLS ALT5= ALT6= ALT7=
  { GPIOA, PORTA,  14,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  88 PTA14 Def=DISABLED ALT0= ALT1=PTA14 ALT2=FTM0_FLT0 ATL3=FTM3_FLT1 ALT4=EWM_IN ALT5= ALT6=FTM1_FLT0 ALT7=BUSOUT
  { GPIOA, PORTA,  15,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //OLED_RES           83 PTA15 Def=ADC1_SE12 ALT0=ADC1_SE12 ALT1=PTA15 ALT2=FTM1_CH2 ATL3=LPSPI0_PCS3 ALT4= ALT5= ALT6= ALT7=
  { GPIOA, PORTA,  16,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //CANCTRL            82 PTA16 Def=ADC1_SE13 ALT0=ADC1_SE13 ALT1=PTA16 ALT2=FTM1_CH3 ATL3=LPSPI1_PCS2 ALT4= ALT5= ALT6= ALT7=
  { GPIOA, PORTA,  17,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  62 PTA17 Def=DISABLED ALT0= ALT1=PTA17 ALT2=FTM0_CH6 ATL3=FTM3_FLT0 ALT4=EWM_OUT_b ALT5= ALT6= ALT7=

  { GPIOB, PORTB,   0,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //ACCUM_ID_I         54 PTB0  Def=ADC0_SE4 ALT0=ADC0_SE4 ALT1=PTB0 ALT2=LPUART0_RX ATL3=LPSPI0_PCS0 ALT4=LPTMR0_ALT3 ALT5=PWT_IN3 ALT6= ALT7=
  { GPIOB, PORTB,   1,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //ACCUM_I            53 PTB1  Def=ADC0_SE5 ALT0=ADC0_SE5 ALT1=PTB1 ALT2=LPUART0_TX ATL3=LPSPI0_SOUT ALT4=TCLK0 ALT5= ALT6= ALT7=
  { GPIOB, PORTB,   2,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI0_SCK           48 PTB2  Def=ADC0_SE6 ALT0=ADC0_SE6 ALT1=PTB2 ALT2=FTM1_CH0 ATL3=LPSPI0_SCK ALT4=FTM1_QD_PHB ALT5= ALT6=TRGMUX_IN3 ALT7=
  { GPIOB, PORTB,   3,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI0_MISO          47 PTB3  Def=ADC0_SE7 ALT0=ADC0_SE7 ALT1=PTB3 ALT2=FTM1_CH1 ATL3=LPSPI0_SIN ALT4=FTM1_QD_PHA ALT5= ALT6=TRGMUX_IN2 ALT7=
  { GPIOB, PORTB,   4,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI0_MOSI          28 PTB4  Def=ACMP1_IN2 ALT0=ACMP1_IN2 ALT1=PTB4 ALT2=FTM0_CH4 ATL3=LPSPI0_SOUT ALT4= ALT5= ALT6=TRGMUX_IN1 ALT7=
  { GPIOB, PORTB,   5,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI0_CS            27 PTB5  Def=DISABLED ALT0= ALT1=PTB5 ALT2=FTM0_CH5 ATL3=LPSPI0_PCS1 ALT4= ALT5= ALT6=TRGMUX_IN0 ALT7=ACMP1_OUT
  //GPIOB, PORTB,   6,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //XTAL               16 PTB6  Def=XTAL ALT0=XTAL ALT1=PTB6 ALT2=LPI2C0_SDA ATL3= ALT4= ALT5= ALT6= ALT7=
  //GPIOB, PORTB,   7,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //EXTAL              15 PTB7  Def=EXTAL ALT0=EXTAL ALT1=PTB7 ALT2=LPI2C0_SCL ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,   8,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   1 }, //OLED_CS            77 PTB8  Def=ADC2_SE11 ALT0=ADC2_SE11 ALT1=PTB8 ALT2=FTM3_CH0 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,   9,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  76 PTB9  Def=ADC2_SE10 ALT0=ADC2_SE10 ALT1=PTB9 ALT2=FTM3_CH1 ATL3=LPI2C0_SCLS ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  10,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //ACCUM_V            75 PTB10 Def=ADC2_SE9 ALT0=ADC2_SE9 ALT1=PTB10 ALT2=FTM3_CH2 ATL3=LPI2C0_SDAS ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  11,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  74 PTB11 Def=ADC2_SE8 ALT0=ADC2_SE8 ALT1=PTB11 ALT2=FTM3_CH3 ATL3=LPI2C0_HREQ ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  12,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //PSRC_V             68 PTB12 Def=ADC1_SE7 ALT0=ADC1_SE7 ALT1=PTB12 ALT2=FTM0_CH0 ATL3=FTM3_FLT2 ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  13,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //OLEDP              67 PTB13 Def=ADC1_SE8 ALT0=ADC1_SE8 ALT1=PTB13 ALT2=FTM0_CH1 ATL3=FTM3_FLT1 ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  14,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI1_SCK           66 PTB14 Def=ADC1_SE9 ALT0=ADC1_SE9 ALT1=PTB14 ALT2=FTM0_CH2 ATL3=LPSPI1_SCK ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  15,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI1_MISO          65 PTB15 Def=ADC1_SE14 ALT0=ADC1_SE14 ALT1=PTB15 ALT2=FTM0_CH3 ATL3=LPSPI1_SIN ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  16,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI1_MOSI          64 PTB16 Def=ADC1_SE15 ALT0=ADC1_SE15 ALT1=PTB16 ALT2=FTM0_CH4 ATL3=LPSPI1_SOUT ALT4= ALT5= ALT6= ALT7=
  { GPIOB, PORTB,  17,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SPI1_CS            63 PTB17 Def=ADC2_SE3 ALT0=ADC2_SE3 ALT1=PTB17 ALT2=FTM0_CH5 ATL3=LPSPI1_PCS3 ALT4= ALT5= ALT6= ALT7=

  { GPIOC, PORTC,   0,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //PSRC_I             40 PTC0  Def=ADC0_SE8/ACMP1_IN4 ALT0=ADC0_SE8/ACMP1_IN4 ALT1=PTC0 ALT2=FTM0_CH0 ATL3= ALT4= ALT5= ALT6=FTM1_CH6 ALT7=
  { GPIOC, PORTC,   1,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //INVERT_I           39 PTC1  Def=ADC0_SE9/ACMP1_IN3 ALT0=ADC0_SE9/ACMP1_IN3 ALT1=PTC1 ALT2=FTM0_CH1 ATL3= ALT4= ALT5= ALT6=FTM1_CH7 ALT7=
  { GPIOC, PORTC,   2,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //CAN0_RX            30 PTC2  Def=ADC0_SE10/ACMP0_IN5/XTAL32 ALT0=ADC0_SE10/ACMP0_IN5/XTAL32 ALT1=PTC2 ALT2=FTM0_CH2 ATL3=CAN0_RX ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,   3,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //CAN0_TX            29 PTC3  Def=ADC0_SE11/ACMP0_IN4/EXTAL32 ALT0=ADC0_SE11/ACMP0_IN4/EXTAL32 ALT1=PTC3 ALT2=FTM0_CH3 ATL3=CAN0_TX ALT4= ALT5= ALT6= ALT7=
  //GPIOC, PORTC,   4,   IRQ_DIS,   0,   ALT0,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //SWD_CLK            96 PTC4  Def=JTAG_TCLK/SWD_CLK ALT0=ACMP0_IN2 ALT1=PTC4 ALT2=FTM1_CH0 ATL3=RTC_CLKOUT ALT4= ALT5=EWM_IN ALT6=FTM1_QD_PHB ALT7=JTAG_TCLK/SWD_CLK
  { GPIOC, PORTC,   5,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //LEDCAN0            95 PTC5  Def=JTAG_TDI ALT0= ALT1=PTC5 ALT2=FTM2_CH0 ATL3=RTC_CLKOUT ALT4=LPI2C1_HREQ ALT5= ALT6=FTM2_QD_PHB ALT7=JTAG_TDI
  { GPIOC, PORTC,   6,   IRQ_DIS,   0,   ALT2,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART1_RX           81 PTC6  Def=ADC1_SE4 ALT0=ADC1_SE4 ALT1=PTC6 ALT2=LPUART1_RX ATL3=CAN1_RX ALT4=FTM3_CH2 ALT5= ALT6= ALT7=
  { GPIOC, PORTC,   7,   IRQ_DIS,   0,   ALT2,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART1_TX           80 PTC7  Def=ADC1_SE5 ALT0=ADC1_SE5 ALT1=PTC7 ALT2=LPUART1_TX ATL3=CAN1_TX ALT4=FTM3_CH3 ALT5= ALT6= ALT7=
  { GPIOC, PORTC,   8,   IRQ_DIS,   0,   ALT6,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART0_CTS          56 PTC8  Def=ADC2_SE14 ALT0=ADC2_SE14 ALT1=PTC8 ALT2=LPUART1_RX ATL3=FTM1_FLT0 ALT4= ALT5= ALT6=LPUART0_CTS ALT7=
  { GPIOC, PORTC,   9,   IRQ_DIS,   0,   ALT6,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //UART0_RTS          55 PTC9  Def=ADC2_SE15 ALT0=ADC2_SE15 ALT1=PTC9 ALT2=LPUART1_TX ATL3=FTM1_FLT1 ALT4= ALT5= ALT6=LPUART0_RTS ALT7=
  { GPIOC, PORTC,  10,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  52 PTC10 Def=DISABLED ALT0= ALT1=PTC10 ALT2=FTM3_CH4 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  11,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //OUT1               51 PTC11 Def=DISABLED ALT0= ALT1=PTC11 ALT2=FTM3_CH5 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  12,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //OUT2               50 PTC12 Def=DISABLED ALT0= ALT1=PTC12 ALT2=FTM3_CH6 ATL3=FTM2_CH6 ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  13,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  49 PTC13 Def=DISABLED ALT0= ALT1=PTC13 ALT2=FTM3_CH7 ATL3=FTM2_CH7 ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  14,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //INVSW_FAULT        46 PTC14 Def=ADC0_SE12/ACMP2_IN5 ALT0=ADC0_SE12/ACMP2_IN5 ALT1=PTC14 ALT2=FTM1_CH2 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  15,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //AIDS_FAULT         45 PTC15 Def=ADC0_SE13/ACMP2_IN4 ALT0=ADC0_SE13/ACMP2_IN4 ALT1=PTC15 ALT2=FTM1_CH3 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  16,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  44 PTC16 Def=ADC0_SE14 ALT0=ADC0_SE14 ALT1=PTC16 ALT2=FTM1_FLT2 ATL3= ALT4=LPI2C1_SDAS ALT5= ALT6= ALT7=
  { GPIOC, PORTC,  17,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //INVERT_V           43 PTC17 Def=ADC0_SE15 ALT0=ADC0_SE15 ALT1=PTC17 ALT2=FTM1_FLT3 ATL3= ALT4=LPI2C1_SCLS ALT5= ALT6= ALT7=

  { GPIOD, PORTD,   0,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  4  PTD0  Def=ADC2_SE0 ALT0=ADC2_SE0 ALT1=PTD0 ALT2=FTM0_CH2 ATL3=LPSPI1_SCK ALT4=FTM2_CH0 ALT5= ALT6=FXIO_D0 ALT7=TRGMUX_OUT1
  { GPIOD, PORTD,   1,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //VREF_1.65V         3  PTD1  Def=ADC2_SE1 ALT0=ADC2_SE1 ALT1=PTD1 ALT2=FTM0_CH3 ATL3=LPSPI1_SIN ALT4=FTM2_CH1 ALT5= ALT6=FXIO_D1 ALT7=TRGMUX_OUT2
  { GPIOD, PORTD,   2,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //PWR24_V            71 PTD2  Def=ADC1_SE2 ALT0=ADC1_SE2 ALT1=PTD2 ALT2=FTM3_CH4 ATL3=LPSPI1_SOUT ALT4=FXIO_D4 ALT5= ALT6=TRGMUX_IN5 ALT7=
  { GPIOD, PORTD,   3,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //MAINS_PATH_STATE   70 PTD3  Def=NMI_b ALT0=ADC1_SE3 ALT1=PTD3 ALT2=FTM3_CH5 ATL3=LPSPI1_PCS0 ALT4=FXIO_D5 ALT5= ALT6=TRGMUX_IN4 ALT7=NMI_b
  { GPIOD, PORTD,   4,   IRQ_DIS,   0,   ANAL,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //INV_PATH_STATE     69 PTD4  Def=ADC1_SE6/ACMP1_IN6 ALT0=ADC1_SE6/ACMP1_IN6 ALT1=PTD4 ALT2=FTM0_FLT3 ATL3=FTM3_FLT3 ALT4= ALT5= ALT6= ALT7=
  { GPIOD, PORTD,   5,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //SW_INV_PATH        33 PTD5  Def=DISABLED ALT0= ALT1=PTD5 ALT2=FTM2_CH3 ATL3=LPTMR0_ALT2 ALT4=FTM2_FLT1 ALT5=PWT_IN2 ALT6=TRGMUX_IN7 ALT7=
  { GPIOD, PORTD,   6,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //SW_MAINS_PATH      32 PTD6  Def=DISABLED ALT0= ALT1=PTD6 ALT2=LPUART2_RX ATL3= ALT4=FTM2_FLT2 ALT5= ALT6= ALT7=
  { GPIOD, PORTD,   7,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //OLEDV              31 PTD7  Def=DISABLED ALT0= ALT1=PTD7 ALT2=LPUART2_TX ATL3= ALT4=FTM2_FLT3 ALT5= ALT6= ALT7=
  { GPIOD, PORTD,   8,   IRQ_DIS,   0,   ALT2,  DSE_HI,  OD__EN, PFE__EN, PULL__UP, GP_INP,   0 }, //I2C1_SDA           42 PTD8  Def=DISABLED ALT0= ALT1=PTD8 ALT2=LPI2C1_SDA ATL3= ALT4=FTM2_FLT2 ALT5= ALT6=FTM1_CH4 ALT7=
  { GPIOD, PORTD,   9,   IRQ_DIS,   0,   ALT2,  DSE_HI,  OD__EN, PFE__EN, PULL__UP, GP_INP,   0 }, //I2C1_SCL           41 PTD9  Def=ACMP1_IN5 ALT0=ACMP1_IN5 ALT1=PTD9 ALT2=LPI2C1_SCL ATL3= ALT4=FTM2_FLT3 ALT5= ALT6=FTM1_CH5 ALT7=
  { GPIOD, PORTD,  10,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //ENC_B              36 PTD10 Def=DISABLED ALT0= ALT1=PTD10 ALT2=FTM2_CH0 ATL3=FTM2_QD_PHB ALT4= ALT5= ALT6= ALT7=
  { GPIOD, PORTD,  11,   IRQ_DIS,   0,   ALT3,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //ENC_A              35 PTD11 Def=DISABLED ALT0= ALT1=PTD11 ALT2=FTM2_CH1 ATL3=FTM2_QD_PHA ALT4= ALT5= ALT6=LPUART2_CTS ALT7=
  { GPIOD, PORTD,  12,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //ENC_SW             34 PTD12 Def=DISABLED ALT0= ALT1=PTD12 ALT2=FTM2_CH2 ATL3=LPI2C1_HREQ ALT4= ALT5= ALT6=LPUART2_RTS ALT7=
  { GPIOD, PORTD,  13,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  25 PTD13 Def=DISABLED ALT0= ALT1=PTD13 ALT2=FTM2_CH4 ATL3= ALT4= ALT5= ALT6= ALT7=RTC_CLKOUT
  { GPIOD, PORTD,  14,   IRQ_DIS,   0,   ALT7,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  24 PTD14 Def=DISABLED ALT0= ALT1=PTD14 ALT2=FTM2_CH5 ATL3= ALT4= ALT5= ALT6= ALT7=CLKOUT
  { GPIOD, PORTD,  15,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //LED_RD             22 PTD15 Def=ACMP2_IN1 ALT0=ACMP2_IN1 ALT1=PTD15 ALT2=FTM0_CH0 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOD, PORTD,  16,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //LED_GR             21 PTD16 Def=ACMP2_IN0 ALT0=ACMP2_IN0 ALT1=PTD16 ALT2=FTM0_CH1 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOD, PORTD,  17,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //DCDC_PGOOD         20 PTD17 Def=DISABLED ALT0= ALT1=PTD17 ALT2=FTM0_FLT2 ATL3=LPUART2_RX ALT4= ALT5= ALT6= ALT7=

  { GPIOE, PORTE,   0,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  94 PTE0  Def=ADC2_SE7 ALT0=ADC2_SE7 ALT1=PTE0 ALT2=LPSPI0_SCK ATL3=TCLK1 ALT4=LPI2C1_SDA ALT5= ALT6=FTM1_FLT2 ALT7=
  { GPIOE, PORTE,   1,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  93 PTE1  Def=ADC2_SE6 ALT0=ADC2_SE6 ALT1=PTE1 ALT2=LPSPI0_SIN ATL3=LPI2C0_HREQ ALT4=LPI2C1_SCL ALT5= ALT6=FTM1_FLT1 ALT7=
  { GPIOE, PORTE,   2,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  85 PTE2  Def=ADC1_SE10 ALT0=ADC1_SE10 ALT1=PTE2 ALT2=LPSPI0_SOUT ATL3=LPTMR0_ALT3 ALT4=FTM3_CH6 ALT5=PWT_IN3 ALT6=LPUART1_CTS ALT7=
  { GPIOE, PORTE,   3,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //INVSW_ON           18 PTE3  Def=DISABLED ALT0= ALT1=PTE3 ALT2=FTM0_FLT0 ATL3=LPUART2_RTS ALT4=FTM2_FLT0 ALT5= ALT6=TRGMUX_IN6 ALT7=ACMP2_OUT
  { GPIOE, PORTE,   4,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //DCDC_MODE          9  PTE4  Def=DISABLED ALT0= ALT1=PTE4 ALT2=BUSOUT ATL3=FTM2_QD_PHB ALT4=FTM2_CH2 ALT5=CAN0_RX ALT6=FXIO_D6 ALT7=EWM_OUT_b
  { GPIOE, PORTE,   5,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  8  PTE5  Def=DISABLED ALT0= ALT1=PTE5 ALT2=TCLK2 ATL3=FTM2_QD_PHA ALT4=FTM2_CH3 ALT5=CAN0_TX ALT6=FXIO_D7 ALT7=EWM_IN
  { GPIOE, PORTE,   6,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //OLED_DC            84 PTE6  Def=ADC1_SE11/ACMP0_IN6 ALT0=ADC1_SE11/ACMP0_IN6 ALT1=PTE6 ALT2=LPSPI0_PCS2 ATL3= ALT4=FTM3_CH7 ALT5= ALT6=LPUART1_RTS ALT7=
  { GPIOE, PORTE,   7,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  59 PTE7  Def=ADC2_SE2/ACMP2_IN6 ALT0=ADC2_SE2/ACMP2_IN6 ALT1=PTE7 ALT2=FTM0_CH7 ATL3=FTM3_FLT0 ALT4= ALT5= ALT6= ALT7=
  { GPIOE, PORTE,   8,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  26 PTE8  Def=ACMP0_IN3 ALT0=ACMP0_IN3 ALT1=PTE8 ALT2=FTM0_CH6 ATL3= ALT4= ALT5= ALT6= ALT7=
  { GPIOE, PORTE,   9,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE__EN, PUPD_DIS, GP_INP,   0 }, //PIDS_FAULT         23 PTE9  Def=ACMP2_IN2/DAC0_OUT ALT0=ACMP2_IN2/DAC0_OUT ALT1=PTE9 ALT2=FTM0_CH7 ATL3=LPUART2_CTS ALT4= ALT5= ALT6= ALT7=
  { GPIOE, PORTE,  10,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  6  PTE10 Def=ADC2_SE12 ALT0=ADC2_SE12 ALT1=PTE10 ALT2=CLKOUT ATL3= ALT4=FTM2_CH4 ALT5= ALT6=FXIO_D4 ALT7=TRGMUX_OUT4
  { GPIOE, PORTE,  11,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //-                  5  PTE11 Def=ADC2_SE13 ALT0=ADC2_SE13 ALT1=PTE11 ALT2=PWT_IN1 ATL3=LPTMR0_ALT1 ALT4=FTM2_CH5 ALT5= ALT6=FXIO_D5 ALT7=TRGMUX_OUT5
  { GPIOE, PORTE,  12,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //AIDS_FBC           19 PTE12 Def=DISABLED ALT0= ALT1=PTE12 ALT2=FTM0_FLT3 ATL3=LPUART2_TX ALT4= ALT5= ALT6= ALT7=
  { GPIOE, PORTE,  13,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //AIDS_SHDN          7  PTE13 Def=DISABLED ALT0= ALT1=PTE13 ALT2= ATL3= ALT4=FTM2_FLT0 ALT5= ALT6= ALT7=
  { GPIOE, PORTE,  14,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_INP,   0 }, //                   17 PTE14 Def=ACMP2_IN3 ALT0=ACMP2_IN3 ALT1=PTE14 ALT2=FTM0_FLT1 ATL3= ALT4=FTM2_FLT1 ALT5= ALT6= ALT7=
  { GPIOE, PORTE,  15,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //PIDS_SHDN          2  PTE15 Def=DISABLED ALT0= ALT1=PTE15 ALT2= ATL3= ALT4=FTM2_CH6 ALT5= ALT6=FXIO_D2 ALT7=TRGMUX_OUT6
  { GPIOE, PORTE,  16,   IRQ_DIS,   0,   GPIO,  DSE_HI,  OD_DIS, PFE_DIS, PUPD_DIS, GP_OUT,   0 }, //EN_CHARGER         1  PTE16 Def=DISABLED ALT0= ALT1=PTE16 ALT2= ATL3= ALT4=FTM2_CH7 ALT5= ALT6=FXIO_D3 ALT7=TRGMUX_OUT7
};

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static void Config_pin(const T_IO_pins_configuration pinc)
{
  pinc.port->PCR[pinc.pin_num] = LSHIFT(pinc.irqc, 16) |
                                LSHIFT(pinc.lock, 15) |
                                LSHIFT(pinc.mux, 8) |
                                LSHIFT(pinc.DSE, 6) |
                                LSHIFT(pinc.ODE, 5) |
                                LSHIFT(pinc.PFE, 4) |
                                LSHIFT(pinc.PUPD, 0);

  if (pinc.init == 0) pinc.gpio->PCOR = LSHIFT(1, pinc.pin_num);
  else pinc.gpio->PSOR = LSHIFT(1, pinc.pin_num);
  pinc.gpio->PDDR =(pinc.gpio->PDDR & ~LSHIFT(1, pinc.pin_num)) | LSHIFT(pinc.dir, pinc.pin_num);
}

/*------------------------------------------------------------------------------



 \return int
 ------------------------------------------------------------------------------*/
int BCKUPC10_init_pins(void)
{
  int i;

  // Включаем тактирование на всех портах

  // Включаем тактирование на всех портах
  PCC->CLKCFG[PCC_PORTA_INDEX] = BIT(30);
  PCC->CLKCFG[PCC_PORTB_INDEX] = BIT(30);
  PCC->CLKCFG[PCC_PORTC_INDEX] = BIT(30);
  PCC->CLKCFG[PCC_PORTD_INDEX] = BIT(30);
  PCC->CLKCFG[PCC_PORTE_INDEX] = BIT(30);


  for (i = 0; i < (sizeof(BCKPC01_pins_conf) / sizeof(BCKPC01_pins_conf[0])); i++)
  {
    Config_pin(BCKPC01_pins_conf[i]);
  }

  // 31 такт на всех цифровых фильтрах портов
  PORTA->DFWR = 31;
  PORTB->DFWR = 31;
  PORTC->DFWR = 31;
  PORTD->DFWR = 31;
  PORTE->DFWR = 31;

  // Тактирование от системогй шины 60 МГц
  PORTA->DFCR = 0;
  PORTB->DFCR = 0;
  PORTC->DFCR = 0;
  PORTD->DFCR = 0;
  PORTE->DFCR = 0;

  // Цифовой фильтр включаем на  пинах
  PORTA->DFER = 0;
  PORTB->DFER = 0;
  PORTC->DFER = BIT(14)+BIT(15);
  PORTD->DFER = BIT(8)+BIT(9)+ BIT(17);
  PORTE->DFER = BIT(9);


  return 0;
}


/*-----------------------------------------------------------------------------------------------------
  Переменные sig_xxx  предназначены для управления выходами из FreeMaster

  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_outputs_states(void)
{
  sig_oledp         = st_oledp         = Get_OLEDP();
  sig_oledv         = st_oledv         = Get_OLEDV();
  sig_oled_cs       = st_oled_cs       = Get_OLED_CS();
  sig_oled_res      = st_oled_res      = Get_OLED_RES();
  sig_oled_dc       = st_oled_dc       = Get_OLED_DC();
  sig_led_gr        = st_led_gr        = Get_LED_GR();
  sig_led_rd        = st_led_rd        = Get_LED_RD();
  sig_out1          = st_out1          = Get_OUT1();
  sig_out2          = st_out2          = Get_OUT2();
  sig_invsw_on      = st_invsw_on      = Get_INVSW_ON();
  sig_pids_shdn     = st_pids_shdn     = Get_PIDS_SHDN();
  sig_aids_shdn     = st_aids_shdn     = Get_AIDS_SHDN();
  sig_aids_fbc      = st_aids_fbc      = Get_AIDS_FBC();
  sig_en_DCDC       = st_en_DCDC       = Get_EN_DCDC();
  sig_dcdc_mode     = st_dcdc_mode     = Get_DCDC_MODE();
  sig_sw_inv_path   = st_sw_inv_path   = Get_SW_INV_PATH();
  sig_sw_mains_path = st_sw_mains_path = Get_SW_MAINS_PATH();
}

/*-----------------------------------------------------------------------------------------------------
  Синхронизация состояния пинов с их переменными состояния
  Переменные состояния st_xxx введены для того чтобы иметь возможность управлять пинами из внешних программ типа FreeMaster через переменные в памяти

  \param void
-----------------------------------------------------------------------------------------------------*/
void Synchronize_output_signals(void)
{
  if (sig_oledp         != st_oledp)  Set_OLEDP(sig_oledp);
  if (sig_oledv         != st_oledv)  Set_OLEDV(sig_oledv);
  if (sig_oled_cs       != st_oled_cs)  Set_OLED_CS(sig_oled_cs);
  if (sig_oled_res      != st_oled_res)  Set_OLED_RES(sig_oled_res);
  if (sig_oled_dc       != st_oled_dc)  Set_OLED_DC(sig_oled_dc);
  if (sig_led_gr        != st_led_gr)  Set_LED_GR(sig_led_gr);
  if (sig_led_rd        != st_led_rd)  Set_LED_RD(sig_led_rd);
  if (sig_out1          != st_out1)  Set_OUT1(sig_out1);
  if (sig_out2          != st_out2)  Set_OUT2(sig_out2);
  if (sig_invsw_on      != st_invsw_on)  Set_INVSW_ON(sig_invsw_on);
  if (sig_pids_shdn     != st_pids_shdn)  Set_PIDS_SHDN(sig_pids_shdn);
  if (sig_aids_shdn     != st_aids_shdn)  Set_AIDS_SHDN(sig_aids_shdn);
  if (sig_aids_fbc      != st_aids_fbc)  Set_AIDS_FBC(sig_aids_fbc);
  if (sig_en_DCDC       != st_en_DCDC)  Set_EN_DCDC(sig_en_DCDC);
  if (sig_dcdc_mode     != st_dcdc_mode)  Set_DCDC_MODE(sig_dcdc_mode);
  if (sig_sw_inv_path   != st_sw_inv_path)  Set_SW_INV_PATH(sig_sw_inv_path);
  if (sig_sw_mains_path != st_sw_mains_path)  Set_SW_MAINS_PATH(sig_sw_mains_path);
}

void Set_OLEDP(int32_t val) { if (val != st_oledp)
  {Set_pin_OLEDP(val);
    sig_oledp         = st_oledp        = val; }
}
void Set_OLEDV(int32_t val) { if (val != st_oledv)
  {Set_pin_OLEDV(val);
    sig_oledv         = st_oledv        = val; }
}
void Set_OLED_CS(int32_t val) { if (val != st_oled_cs)
  {Set_pin_OLED_CS(val);
    sig_oled_cs       = st_oled_cs      = val; }
}
void Set_OLED_RES(int32_t val) { if (val != st_oled_res)
  {Set_pin_OLED_RES(val);
    sig_oled_res      = st_oled_res     = val; }
}
void Set_OLED_DC(int32_t val) { if (val != st_oled_dc)
  {Set_pin_OLED_DC(val);
    sig_oled_dc       = st_oled_dc      = val; }
}
void Set_LED_GR(int32_t val) { if (val != st_led_gr)
  {Set_pin_LED_GR(val);
    sig_led_gr        = st_led_gr       = val; }
}
void Set_LED_RD(int32_t val) { if (val != st_led_rd)
  {Set_pin_LED_RD(val);
    sig_led_rd        = st_led_rd       = val; }
}
void Set_OUT1(int32_t val) { if (val != st_out1)
  {Set_pin_OUT1(val);
    sig_out1          = st_out1         = val; }
}
void Set_OUT2(int32_t val) { if (val != st_out2)
  {Set_pin_OUT2(val);
    sig_out2          = st_out2         = val; }
}
void Set_INVSW_ON(int32_t val) { if (val != st_invsw_on)
  {Set_pin_INVSW_ON(val);
    sig_invsw_on      = st_invsw_on     = val; }
}
void Set_PIDS_SHDN(int32_t val) { if (val != st_pids_shdn)
  {Set_pin_PIDS_SHDN(val);
    sig_pids_shdn     = st_pids_shdn    = val; }
}
void Set_AIDS_SHDN(int32_t val) { if (val != st_aids_shdn)
  {Set_pin_AIDS_SHDN(val);
    sig_aids_shdn     = st_aids_shdn    = val; }
}
void Set_AIDS_FBC(int32_t val) { if (val != st_aids_fbc)
  {Set_pin_AIDS_FBC(val);
    sig_aids_fbc      = st_aids_fbc     = val; }
}
void Set_EN_DCDC(int32_t val) { if (val != st_en_DCDC)
  {Set_pin_EN_DCDC(val);
    sig_en_DCDC       = st_en_DCDC      = val; }
}
void Set_DCDC_MODE(int32_t val) { if (val != st_dcdc_mode)
  {Set_pin_DCDC_MODE(val);
    sig_dcdc_mode     = st_dcdc_mode    = val; }
}
void Set_SW_INV_PATH(int32_t val) { if (val != st_sw_inv_path)
  {Set_pin_SW_INV_PATH(val);
    sig_sw_inv_path   = st_sw_inv_path  = val; }
}
void Set_SW_MAINS_PATH(int32_t val) { if (val != st_sw_mains_path)
  {Set_pin_SW_MAINS_PATH(val);
    sig_sw_mains_path = st_sw_mains_path= val; }
}


void Set_pin_OLEDP(int32_t val) { if (val != 0) GPIOB->PSOR  = BIT(13);
  else GPIOB->PCOR  = BIT(13); }
void Set_pin_OLEDCLK(int32_t val) { if (val != 0) GPIOA->PSOR  = BIT(0);
  else GPIOA->PCOR  = BIT(0); }
void Set_pin_OLEDDIN(int32_t val) { if (val != 0) GPIOA->PSOR  = BIT(1);
  else GPIOA->PCOR  = BIT(1); }
void Set_pin_OLEDV(int32_t val) { if (val != 0) GPIOD->PSOR  = BIT(7);
  else GPIOD->PCOR  = BIT(7); }
void Set_pin_OLED_CS(int32_t val) { if (val != 0) GPIOB->PSOR  = BIT(8);
  else GPIOB->PCOR  = BIT(8); }
void Set_pin_OLED_RES(int32_t val) { if (val != 0) GPIOA->PSOR  = BIT(15);
  else GPIOA->PCOR  = BIT(15); }
void Set_pin_OLED_DC(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(6);
  else GPIOE->PCOR  = BIT(6); }
void Set_pin_LED_GR(int32_t val) { if (val != 0) GPIOD->PSOR  = BIT(16);
  else GPIOD->PCOR  = BIT(16); }
void Set_pin_LED_RD(int32_t val) { if (val != 0) GPIOD->PSOR  = BIT(15);
  else GPIOD->PCOR  = BIT(15); }
void Set_pin_OUT1(int32_t val) { if (val != 0) GPIOC->PSOR  = BIT(11);
  else GPIOC->PCOR  = BIT(11); }
void Set_pin_OUT2(int32_t val) { if (val != 0) GPIOC->PSOR  = BIT(12);
  else GPIOC->PCOR  = BIT(12); }
void Set_pin_INVSW_ON(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(3);
  else GPIOE->PCOR  = BIT(3); }
void Set_pin_PIDS_SHDN(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(15);
  else GPIOE->PCOR  = BIT(15); }
void Set_pin_AIDS_SHDN(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(13);
  else GPIOE->PCOR  = BIT(13); }
void Set_pin_AIDS_FBC(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(12);
  else GPIOE->PCOR  = BIT(12); }
void Set_pin_EN_DCDC(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(16);
  else GPIOE->PCOR  = BIT(16); }
void Set_pin_DCDC_MODE(int32_t val) { if (val != 0) GPIOE->PSOR  = BIT(4);
  else GPIOE->PCOR  = BIT(4); }
void Set_pin_SW_INV_PATH(int32_t val) { if (val != 0) GPIOD->PSOR  = BIT(5);
  else GPIOD->PCOR  = BIT(5); }
void Set_pin_SW_MAINS_PATH(int32_t val) { if (val != 0) GPIOD->PSOR  = BIT(6);
  else GPIOD->PCOR  = BIT(6); }


int32_t Get_OLEDP(void)           { return (GPIOB->PDOR & BIT(13))>> 13;}
int32_t Get_OLEDV(void)           { return (GPIOD->PDOR & BIT(7 ))>> 7; }
int32_t Get_OLED_CS(void)         { return (GPIOB->PDOR & BIT(8 ))>> 8; }
int32_t Get_OLED_RES(void)        { return (GPIOA->PDOR & BIT(15))>> 15;}
int32_t Get_OLED_DC(void)         { return (GPIOE->PDOR & BIT(6 ))>> 6; }
int32_t Get_LED_GR(void)          { return (GPIOD->PDOR & BIT(16))>> 16;}
int32_t Get_LED_RD(void)          { return (GPIOD->PDOR & BIT(15))>> 15;}
int32_t Get_OUT1(void)            { return (GPIOC->PDOR & BIT(11))>> 11;}
int32_t Get_OUT2(void)            { return (GPIOC->PDOR & BIT(12))>> 12;}
int32_t Get_INVSW_ON(void)        { return (GPIOE->PDOR & BIT(3 ))>> 3; }
int32_t Get_PIDS_SHDN(void)       { return (GPIOE->PDOR & BIT(15))>> 15;}
int32_t Get_AIDS_SHDN(void) // 0 - выключение идеального диода от аккумулятора к системе, 1 - включение идеального диода от аккумулятора к системе
                                  { return (GPIOE->PDOR & BIT(13))>> 13;}
// От момента подачи сигнала до срабатываняи ключей проходит не менее 1.5 мс
int32_t Get_AIDS_FBC(void)        { return (GPIOE->PDOR & BIT(12))>> 12;}
int32_t Get_EN_DCDC(void)         { return (GPIOE->PDOR & BIT(16))>> 16;}
int32_t Get_DCDC_MODE(void) // 0 - forced continuous mode, 1 - pulse-skipping mode.
                                  { return (GPIOE->PDOR & BIT(4 ))>> 4; }
// В состоянии 0 меньше нагрев микросхемы DCDC преобразователя на 10 градусов (увеличение потребления на 1 Вт),
// но больше нагрев индуктивности на 1 градус. Измерено в режиме когда на выходе было 31В и 1А
// Однако в режиме 0 преобразователь пропускает обратный ток и может сжечь входную схему
int32_t Get_SW_INV_PATH(void)     { return (GPIOD->PDOR & BIT(5 ))>> 5; }
int32_t Get_SW_MAINS_PATH(void)   { return (GPIOD->PDOR & BIT(6 ))>> 6; }
int32_t Get_ENC_SW(void)          { return (GPIOD->PDIR & BIT(12))>> 12;}
int32_t Get_ENC_A(void)           { return (GPIOD->PDIR & BIT(11))>> 11;}
int32_t Get_ENC_B(void)           { return (GPIOD->PDIR & BIT(10))>> 10;}




int32_t Get_PIDS_FAULT(void) // Если напряжение на системной шине присутствует, то 0 здесь появится только при перенапряжении. Если ключ выключен то ноль не появляется
                                  { return (GPIOE->PDIR & BIT(9 ))>> 9; }
int32_t Get_AIDS_FAULT(void) // Если напряжение на системной шине присутствует, то 0 здесь появится только при перенапряжении. Если ключ выключен то ноль не появляется
                                  { return (GPIOC->PDIR & BIT(15))>> 15;}
int32_t Get_DCDC_PGOOD(void)      { return (GPIOD->PDIR & BIT(17))>> 17;}
int32_t Get_INVSW_FAULT(void) // Здесь будет ноль если ключ выключен или ошибка ключа.
                                  { return (GPIOC->PDIR & BIT(14))>> 14;}

