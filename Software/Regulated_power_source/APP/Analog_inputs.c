// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.12.20
// 10:52:31
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

int32_t Get_MAINS_PATH_STATE      (void)         { return (int32_t)g_ADC_results.smpl_MAINS_PATH_STATE;    }
int32_t Get_INV_PATH_STATE        (void)         { return (int32_t)g_ADC_results.smpl_INV_PATH_STATE  ;    }


float   Value_ACCUM_ID_I          (int32_t v)   { return sys.accum_id_i;                    }  // Знаковая величина. Ток аккумулятора через идеальный диод в систему (с плюсом) и из системы (с минусом)
float   Value_ACCUM_I             (int32_t v)   { return sys.accum_i;                       }  // Знаковая величина. Ток от аккумулятора в систему (с плюсом) и из системв (с минусом). Не считая тока в инвертер
float   Value_PSRC_I              (int32_t v)   { return sys.psrc_i;                        }  // Знаковая величина. Ток источника питания в систему (с плюсом) и из системы (с минусом)
float   Value_INVERT_I            (int32_t v)   { return sys.invert_i;                      }  // Знаковое величина. Ток в инвертер от аккумулятора (с плюсом) и обратно (с минусом)
float   Value_INVERT_V            (int32_t v)   { return sys.invert_v;                      }  // Напряжение на входе 24V инвертера
float   Value_SYS_V               (int32_t v)   { return sys.sys_v;                         }  // Напряжение выходящее в систему
float   Value_MAINS_PATH_STATE    (int32_t v)   { return (float)v * MAINS_PATH_STATE_SCALE; }  // Напряжение на реле подключающем инвертер
float   Value_INV_PATH_STATE      (int32_t v)   { return (float)v * INV_PATH_STATE_SCALE;   }  // Напряжение на реле подключающем сеть
float   Value_PSRC_V              (int32_t v)   { return sys.psrc_v;                        }  // Напряжение на источнике питания
float   Value_ACCUM_V             (int32_t v)   { return sys.accum_v;                       }  // Напряжение на уккумуляторе

float   Value_fp_ACCUM_ID_I       (float v)   { return v * ACCUM_ID_I_SCALE;       }  // Знаковая величина. Ток аккумулятора через идеальный диод в систему (с плюсом) и из системы (с минусом)
float   Value_fp_ACCUM_I          (float v)   { return v * ACCUM_I_SCALE;          }  // Знаковая величина. Ток от аккумулятора в систему (с плюсом) и из системв (с минусом). Не считая тока в инвертер
float   Value_fp_PSRC_I           (float v)   { return v * PSRC_I_SCALE;           }  // Знаковая величина. Ток источника питания в систему (с плюсом) и из системы (с минусом)
float   Value_fp_INVERT_I         (float v)   { return v * INVERT_I_SCALE;         }  // Знаковое величина. Ток в инвертер от аккумулятора (с плюсом) и обратно (с минусом)
float   Value_fp_INVERT_V         (float v)   { return v * INVERT_V_SCALE;         }  // Напряжение на входе 24V инвертера
float   Value_fp_SYS_V            (float v)   { return v * SYS_V_SCALE;            }  // Напряжение выходящее в систему
float   Value_fp_MAINS_PATH_STATE (float v)   { return v * MAINS_PATH_STATE_SCALE; }  // Напряжение на реле подключающем инвертер
float   Value_fp_INV_PATH_STATE   (float v)   { return v * INV_PATH_STATE_SCALE;   }  // Напряжение на реле подключающем сеть
float   Value_fp_PSRC_V           (float v)   { return v * PSRC_V_SCALE;           }  // Напряжение на источнике питания
float   Value_fp_ACCUM_V          (float v)   { return v * ACCUM_V_SCALE;          }  // Напряжение на уккумуляторе

