#ifndef __DCDC_DAC80501_H
  #define __DCDC_DAC80501_H

#define MIN_DCDC_VOLTAGE       (1.66f)
#define MAX_DCDC_VOLTAGE       (31.91f)
#define DCDC_MIN_VOLTAGE_CODE  (0xFFFF)


uint32_t   DCDC_DAC_initializing(void);
uint32_t   DCDC_DAC_refresh(void);

uint32_t   DCDC_DAC_read_status(void);
uint16_t   DCDC_DAC_Volt_to_code(float v);
float      DCDC_DAC_Code_to_Volt(uint16_t code);

int32_t    Get_DCDC_DAC_code(void);
void       Set_DCDC_DAC_code(int32_t val);
float      Value_Charger_Code(int32_t v);

#endif // BASE_MAX5478_DIGPOT_H



