#ifndef BACKPMAN10_PINS_H
  #define BACKPMAN10_PINS_H

typedef struct
{
    GPIO_Type      *gpio;
    PORT_Type      *port;
    unsigned char  pin_num;
    unsigned char  irqc; //  Interrupt Configuration
                         //  0000 Interrupt/DMA Request disabled.
    //  0001 DMA Request on rising edge.
    //  0010 DMA Request on falling edge.
    //  0011 DMA Request on either edge.
    //  0100 Reserved.
    //  1000 Interrupt when logic zero.
    //  1001 Interrupt on rising edge.
    //  1010 Interrupt on falling edge.
    //  1011 Interrupt on either edge.
    //  1100 Interrupt when logic one.
    unsigned char  lock; //  if 1 Pin Control Register bits [15:0] are locked and cannot be updated until the next System Reset.
    unsigned char  mux;  //  Pin Mux Control
                         //  000 Pin Disabled (Analog).
    //  001 Alternative 1 (GPIO).
    //  010 Alternative 2 (chip specific).
    //  011 Alternative 3 (chip specific).
    //  100 Alternative 4 (chip specific).
    //  101 Alternative 5 (chip specific).
    //  110 Alternative 6 (chip specific).
    //  111 Alternative 7 (chip specific / JTAG / NMI).
    unsigned char  DSE; // 0 Low drive strength is configured on the corresponding pin, if pin is configured as a digital output.
                        // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.
    unsigned char  ODE;  // 0 Open Drain output is disabled on the corresponding pin.
                         // 1 Open Drain output is enabled on the corresponding pin, provided pin is configured as a digital output.
    unsigned char  PFE;  // 0 Passive Input Filter is disabled on the corresponding pin.
                         // 1 Passive Input Filter is enabled on the corresponding pin.
    unsigned char  PUPD; // 00 Internal pull-up or pull-down resistor is not enabled on the corresponding pin.
                         // 10 Internal pull-down resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
    // 11 Internal pull-up resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
    unsigned char  dir;  // 0 Pin is configured as general purpose input, if configured for the GPIO function
                         // 1 Pin is configured for general purpose output, if configured for the GPIO function
    unsigned char  init; // Init state

} T_IO_pins_configuration;

  #define   ANAL          0  // Pin Disabled (Analog).
  #define   ALT0          0  // Pin Disabled (Analog).
  #define   GPIO          1  // Alternative 1 (GPIO).
  #define   ALT1          1  // Alternative 1 (GPIO).
  #define   ALT2          2  //
  #define   ALT3          3  //
  #define   ALT4          4  //
  #define   ALT5          5  //
  #define   ALT6          6  //
  #define   ALT7          7  //

  #define   DSE_LO        0 // 0 Low drive strength is configured on the corresponding pin, if pin is configured as a digital output.
  #define   DSE_HI        1 // 1 High drive strength is configured on the corresponding pin, if pin is configured as a digital output.

  #define   OD_DIS        0 // 0 Open Drain output is disabled on the corresponding pin.
  #define   OD__EN        1 // 1 Open Drain output is enabled on the corresponding pin, provided pin is configured as a digital output.

  #define   PFE_DIS       0 // 0 Passive Input Filter is disabled on the corresponding pin.
  #define   PFE__EN       1 // 1 Passive Input Filter is enabled on the corresponding pin.

  #define   FAST_SLEW     0 // 0 Fast slew rate is configured on the corresponding pin, if pin is configured as a digital output.
  #define   SLOW_SLEW     1 // 1 Slow slew rate is configured on the corresponding pin, if pin is configured as a digital output.


  #define   PUPD_DIS      0 // 00 Internal pull-up or pull-down resistor is not enabled on the corresponding pin.
  #define   PULL__DN      2 // 10 Internal pull-down resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.
  #define   PULL__UP      3 // 11 Internal pull-up resistor is enabled on the corresponding pin, if the corresponding Port Pull Enable Register bit is set.

  #define   GP_INP        0 // 0 Pin is configured as general purpose input, if configured for the GPIO function
  #define   GP_OUT        1 // 1 Pin is configured for general purpose output, if configured for the GPIO function

  #define   IRQ_DIS       0  // Interrupt Status Flag (ISF) is disabled.
  #define   IRQ_IDR       1  // ISF flag and DMA request on rising edge.
  #define   IRQ_IDF       2  // ISF flag and DMA request on falling edge.
  #define   IRQ_IDE       3  // ISF flag and DMA request on either edge.
  #define   IRQ_IL0       8  // ISF flag and Interrupt when logic 0.
  #define   IRQ_IRE       9  // ISF flag and Interrupt on rising-edge.
  #define   IRQ_IFE       10 // ISF flag and Interrupt on falling-edge.
  #define   IRQ_IEE       11 // ISF flag and Interrupt on either edge.
  #define   IRQ_IL1       12 // ISF flag and Interrupt when logic 1.

extern int32_t  st_oledp;
extern int32_t  st_oledv;
extern int32_t  st_oled_cs;
extern int32_t  st_oled_res;
extern int32_t  st_oled_dc;
extern int32_t  st_led_gr;
extern int32_t  st_led_rd;
extern int32_t  st_out1;
extern int32_t  st_out2;
extern int32_t  st_invsw_on;
extern int32_t  st_pids_shdn;
extern int32_t  st_aids_shdn;
extern int32_t  st_aids_fbc;
extern int32_t  st_en_DCDC;
extern int32_t  st_dcdc_mode;
extern int32_t  st_sw_inv_path;
extern int32_t  st_sw_mains_path;

//  Переменные sig_xxx  предназначены для управления выходами из FreeMaster
extern int32_t  sig_oledp;
extern int32_t  sig_oledv;
extern int32_t  sig_oled_cs;
extern int32_t  sig_oled_res;
extern int32_t  sig_oled_dc;
extern int32_t  sig_led_gr;
extern int32_t  sig_led_rd;
extern int32_t  sig_out1;
extern int32_t  sig_out2;
extern int32_t  sig_invsw_on;
extern int32_t  sig_pids_shdn;
extern int32_t  sig_aids_shdn;
extern int32_t  sig_aids_fbc;
extern int32_t  sig_en_DCDC;
extern int32_t  sig_dcdc_mode;
extern int32_t  sig_sw_inv_path;
extern int32_t  sig_sw_mains_path;





int      BCKUPC10_init_pins(void);

void     Init_outputs_states(void);
void     Synchronize_output_signals(void);

void     Set_OLEDP(int32_t val);
void     Set_OLEDV(int32_t val);
void     Set_OLED_CS(int32_t val);
void     Set_OLED_RES(int32_t val);
void     Set_OLED_DC(int32_t val);
void     Set_LED_GR(int32_t val);
void     Set_LED_RD(int32_t val);
void     Set_OUT1(int32_t val);
void     Set_OUT2(int32_t val);
void     Set_INVSW_ON(int32_t val);
void     Set_PIDS_SHDN(int32_t val);
void     Set_AIDS_SHDN(int32_t val);
void     Set_AIDS_FBC(int32_t val);
void     Set_EN_DCDC(int32_t val);
void     Set_DCDC_MODE(int32_t val);
void     Set_SW_INV_PATH(int32_t val);
void     Set_SW_MAINS_PATH(int32_t val);

int32_t Get_OLEDP(void);
int32_t Get_OLEDV(void);
int32_t Get_OLED_CS(void);
int32_t Get_OLED_RES(void);
int32_t Get_OLED_DC(void);
int32_t Get_LED_GR(void);
int32_t Get_LED_RD(void);
int32_t Get_OUT1(void);
int32_t Get_OUT2(void);
int32_t Get_INVSW_ON(void);
int32_t Get_PIDS_SHDN(void);
int32_t Get_AIDS_SHDN(void);
int32_t Get_AIDS_FBC(void);
int32_t Get_EN_DCDC(void);
int32_t Get_DCDC_MODE(void);
int32_t Get_SW_INV_PATH(void);
int32_t Get_SW_MAINS_PATH(void);
int32_t Get_ENC_SW(void);
int32_t Get_ENC_A(void);
int32_t Get_ENC_B(void);


int32_t Get_PIDS_FAULT(void);
int32_t Get_AIDS_FAULT(void);
int32_t Get_DCDC_PGOOD(void);
int32_t Get_INVSW_FAULT(void);

void Set_pin_OLEDP(int32_t val);
void Set_pin_OLEDCLK(int32_t val);
void Set_pin_OLEDDIN(int32_t val);
void Set_pin_OLEDV(int32_t val);
void Set_pin_OLED_CS(int32_t val);
void Set_pin_OLED_RES(int32_t val);
void Set_pin_OLED_DC(int32_t val);
void Set_pin_LED_GR(int32_t val);
void Set_pin_LED_RD(int32_t val);
void Set_pin_OUT1(int32_t val);
void Set_pin_OUT2(int32_t val);
void Set_pin_INVSW_ON(int32_t val);
void Set_pin_PIDS_SHDN(int32_t val);
void Set_pin_AIDS_SHDN(int32_t val);
void Set_pin_AIDS_FBC(int32_t val);
void Set_pin_EN_DCDC(int32_t val);
void Set_pin_DCDC_MODE(int32_t val);
void Set_pin_SW_INV_PATH(int32_t val);
void Set_pin_SW_MAINS_PATH(int32_t val);


#endif // BCKUPC10_PINS_H



