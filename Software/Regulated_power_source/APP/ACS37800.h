#ifndef ACS37800_H
  #define ACS37800_H

#define U15_SPI_ID  0
#define U17_SPI_ID  1

#define ACS37800_RD_CMD  (0x80)
#define ACS37800_WR_CMD  (0x00)

void     ACS37800_read_register(uint8_t reg_addr, uint32_t *p_reg_val);
void     ACS37800_read(void);
uint32_t ACS37800_test(void);


float    Value_VRMS_U15(int32_t v);
float    Value_IRMS_U15(int32_t v);
float    Value_VRMS_U17(int32_t v);
float    Value_IRMS_U17(int32_t v);



#endif // ACS37800_H



