#ifndef __BCKUPC10_MONITOR_H
  #define __BCKUPC10_MONITOR_H


extern const T_VT100_Menu MENU_BCKUPC10;

float          Value_ACCUM_SUM_I         (int32_t v);
float          Value_ACCUM_CHARGE        (int32_t v);
float          Value_PwrSrcPower         (int32_t v);
float          Value_AccumChargePower    (int32_t v);
float          Value_AccumDisChargePower (int32_t v);
float          Value_chaging_pwr_loss    (int32_t v);
float          Value_charging_efficiency (int32_t v);


#endif // __PLATF_MONITOR_H



