#ifndef __FREEMASTER_VARS
  #define __FREEMASTER_VARS
#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"

FMSTR_TSA_TABLE_BEGIN(wvar_vars)
FMSTR_TSA_RW_VAR( wvar.vcom_mode                       ,FMSTR_TSA_UINT8     ) // Select VCOM mode (0 - VT100 terminal, 1 - FreeMaster)  | def.val.= 0
FMSTR_TSA_TABLE_END();


#endif
