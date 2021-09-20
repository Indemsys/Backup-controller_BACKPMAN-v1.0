#include "App.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"

const T_parmenu parmenu[2]=
{
  {BKMAN1_2_0,BKMAN1_2_main,"Parameters and settings","PARAMETERS", -1},//  Основная категория
  {BKMAN1_2_main,BKMAN1_2_General,"General settings","GENERAL SETTINGS", -1},//  
};


const T_work_params dwvar[3]=
{
  {
    "name",
    "Product  name",
    "-",
    (void*)&wvar.name,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN1_2_General,
    "BACKPMAN10",
    "%s",
    0,
    sizeof(wvar.name),
    1,
    0,
  },
  {
    "ver",
    "Firmware version",
    "-",
    (void*)&wvar.ver,
    tstring,
    0,
    0,
    0,
    0,
    BKMAN1_2_General,
    "1.0",
    "%s",
    0,
    sizeof(wvar.ver),
    2,
    0,
  },
  {
    "vcom_mode",
    "Select VCOM mode (0 - VT100 terminal, 1 - FreeMaster) ",
    "-",
    (void*)&wvar.vcom_mode,
    tint8u,
    0,
    0,
    1,
    0,
    BKMAN1_2_General,
    "",
    "%d",
    0,
    sizeof(wvar.vcom_mode),
    3,
    1,
  },
};
 
 
// Selector description:  Выбор между Yes и No
static const T_selector_items selector_1[2] = 
{
  {0, "No", 0},
  {1, "Yes", 1},
};
 
const T_selectors_list selectors_list[2] = 
{
  {"string"            , 0    , 0             },
  {"binary"            , 2    , selector_1    },
};
 
