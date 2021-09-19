// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-07
// 10:43:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
#include   "App.h"


static void Do_Signal_dignostics(uint8_t keycode);


const T_VT100_Menu_item MENU_BASE_ITEMS[] =
{
  { '1', Do_Signal_dignostics, 0 },
  { 'R', 0, 0 },
  { 'M', 0,(void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_BCKUPC10       =
{
  "BACKPMAN 1.0 rev1.2 diagnostic menu",
  "\033[5C <1> - Signals diagnostic\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_BASE_ITEMS,
};



typedef   float   (*T_conv_func)(int32_t);
typedef   int32_t (*T_get_func)(void);
typedef   void    (*T_set_func)(int32_t);
typedef   void    (*T_get_str_func)(int32_t n, uint8_t *str);

#define   VTYPE__BOOL  1
#define   VTYPE_INT32  2
#define   VTYPE_FLOAT  3
#define   VTYPE_STRNG  4

typedef struct
{
    uint8_t           row;
    uint8_t           col;
    char              cmd_sym;  // Символ редактирования если редактирование доступно
    uint8_t           vtype;    // Представлять символ как бинарный
    const char *const fmt;
    const char *const name;
    T_get_func        getv;     // Функция чтения значения
    T_set_func        setv;     // Функция запис значения
    void            *cfunc;     // Функция конвертирования значения или нетипизированная функция
    const char *const cfmt;

} T_sigs_map;



const T_sigs_map pcb_sigs_map[]=
{ //row  col cmd_sym     vtype        fmt     name                  getv                 setv                cfunc                                  cfmt
  { 3 ,  1 , 0         , VTYPE_FLOAT ,""       , "System voltage            "  , 0                    , 0                 , (void *)Value_SYS_V               ,"%7.2f V"   },
  { 4 ,  1 , 0         , VTYPE_FLOAT ,""       , "Power source voltage      "  , 0                    , 0                 , (void *)Value_PSRC_V              ,"%7.2f V"   },
  { 5 ,  1 , 0         , VTYPE_FLOAT ,""       , "Power source current      "  , 0                    , 0                 , (void *)Value_PSRC_I              ,"%7.3f A"   },

  { 7 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accumulator voltage       "  , 0                    , 0                 , (void *)Value_ACCUM_V             ,"%7.2f V"   },
  { 8 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accum.curr. to/from system"  , 0                    , 0                 , (void *)Value_ACCUM_I             ,"%7.3f A"   },
  { 9 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accum.curr. to system     "  , 0                    , 0                 , (void *)Value_ACCUM_ID_I          ,"%7.3f A"   },
  {10 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accumulator total current "  , 0                    , 0                 , (void *)Value_ACCUM_SUM_I         ,"%7.3f A"   },

  {12 ,  1 , 0         , VTYPE_FLOAT ,""       , "Inverter input voltage    "  , 0                    , 0                 , (void *)Value_INVERT_V            ,"%7.2f V"   },
  {13 ,  1 , 0         , VTYPE_FLOAT ,""       , "Inverter input current    "  , 0                    , 0                 , (void *)Value_INVERT_I            ,"%7.3f A"   },

  {15 ,  1 , 0         , VTYPE_INT32 ,"%05d"   , "Inverter relay coil       "  , Get_MAINS_PATH_STATE , 0                 , (void *)Value_MAINS_PATH_STATE    ,"(%7.2f V)" },
  {16 ,  1 , 0         , VTYPE_INT32 ,"%05d"   , "Mains relay coil          "  , Get_INV_PATH_STATE   , 0                 , (void *)Value_INV_PATH_STATE      ,"(%7.2f V)" },
  {17 ,  1 , 0         , VTYPE__BOOL ,"%-4d"   , "Power source switch fault "  , Get_PIDS_FAULT       , 0                 , 0                                 ," "         },
  {18 ,  1 , 0         , VTYPE__BOOL ,"%-4d"   , "Accum. ideal diode fault  "  , Get_AIDS_FAULT       , 0                 , 0                                 ," "         },
  {19 ,  1 , 0         , VTYPE__BOOL ,"%-4d"   , "DCDC PGOOD signal         "  , Get_DCDC_PGOOD       , 0                 , 0                                 ," "         },
  {20 ,  1 , 0         , VTYPE__BOOL ,"%-4d"   , "Inverter power sw. fault  "  , Get_INVSW_FAULT      , 0                 , 0                                 ," "         },

  {22 ,  1 ,'k'        , VTYPE_INT32 ,"%05d"   , "Charger DAC code          "  , Get_DCDC_DAC_code     , Set_DCDC_DAC_code , (void *)Value_Charger_Code        ,"(%7.2f V)" },

  {24 ,  1 , 0         , VTYPE_FLOAT ,""       , "Power source power        "  , 0                    , 0                 , (void *)Value_PwrSrcPower         ,"%7.1f W"   },
  {25 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accumulator charge power  "  , 0                    , 0                 , (void *)Value_AccumChargePower    ,"%7.1f W"   },
  {26 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accumulator dischar.power "  , 0                    , 0                 , (void *)Value_AccumDisChargePower ,"%7.1f W"   },
  {27 ,  1 , 0         , VTYPE_FLOAT ,""       , "Charging power loss       "  , 0                    , 0                 , (void *)Value_chaging_pwr_loss    ,"%7.1f W"   },
  {28 ,  1 , 0         , VTYPE_FLOAT ,""       , "Charging efficiency       "  , 0                    , 0                 , (void *)Value_charging_efficiency ,"%7.1f %%"  },

  {30 ,  1 , 0         , VTYPE_FLOAT ,""       , "V RMS U15                 "  , 0                    , 0                 , (void *)Value_VRMS_U15            ,"%7.1f V"   },
  {31 ,  1 , 0         , VTYPE_FLOAT ,""       , "I RMS U15                 "  , 0                    , 0                 , (void *)Value_IRMS_U15            ,"%7.1f A"   },
  {32 ,  1 , 0         , VTYPE_FLOAT ,""       , "V RMS U17                 "  , 0                    , 0                 , (void *)Value_VRMS_U17            ,"%7.1f V"   },
  {33 ,  1 , 0         , VTYPE_FLOAT ,""       , "I RMS U17                 "  , 0                    , 0                 , (void *)Value_IRMS_U17            ,"%7.1f A"   },

  {35 ,  1 , 0         , VTYPE_FLOAT ,""       , "Accumulator charge        "  , 0                    , 0                 , (void *)Value_ACCUM_CHARGE        ,"%7.2f Ah"  },

  { 3 , 54 ,'p'        , VTYPE__BOOL ,"%-4d"   , "OLEDP               "        , Get_OLEDP            , Set_OLEDP         , 0                                 ," "         },
  { 4 , 54 ,'q'        , VTYPE__BOOL ,"%-4d"   , "OLEDV               "        , Get_OLEDV            , Set_OLEDV         , 0                                 ," "         },
  { 5 , 54 ,'w'        , VTYPE__BOOL ,"%-4d"   , "OLED_CS             "        , Get_OLED_CS          , Set_OLED_CS       , 0                                 ," "         },
  { 6 , 54 ,'e'        , VTYPE__BOOL ,"%-4d"   , "OLED_RES            "        , Get_OLED_RES         , Set_OLED_RES      , 0                                 ," "         },
  { 7 , 54 ,'r'        , VTYPE__BOOL ,"%-4d"   , "OLED_DC             "        , Get_OLED_DC          , Set_OLED_DC       , 0                                 ," "         },
  { 8 , 54 ,'t'        , VTYPE__BOOL ,"%-4d"   , "LED_GR              "        , Get_LED_GR           , Set_LED_GR        , 0                                 ," "         },
  { 9 , 54 ,'y'        , VTYPE__BOOL ,"%-4d"   , "LED_RD              "        , Get_LED_RD           , Set_LED_RD        , 0                                 ," "         },
  {10 , 54 ,'u'        , VTYPE__BOOL ,"%-4d"   , "OUT1                "        , Get_OUT1             , Set_OUT1          , 0                                 ," "         },
  {11 , 54 ,'i'        , VTYPE__BOOL ,"%-4d"   , "OUT2                "        , Get_OUT2             , Set_OUT2          , 0                                 ," "         },
  {12 , 54 ,'o'        , VTYPE__BOOL ,"%-4d"   , "INVSW_ON            "        , Get_INVSW_ON         , Set_INVSW_ON      , 0                                 ," "         },
  {13 , 54 ,'a'        , VTYPE__BOOL ,"%-4d"   , "PIDS_SHDN           "        , Get_PIDS_SHDN        , Set_PIDS_SHDN     , 0                                 ," "         },
  {14 , 54 ,'s'        , VTYPE__BOOL ,"%-4d"   , "AIDS_SHDN           "        , Get_AIDS_SHDN        , Set_AIDS_SHDN     , 0                                 ," "         },
  {15 , 54 ,'d'        , VTYPE__BOOL ,"%-4d"   , "AIDS_FBC            "        , Get_AIDS_FBC         , Set_AIDS_FBC      , 0                                 ," "         },
  {16 , 54 ,'f'        , VTYPE__BOOL ,"%-4d"   , "EN_CHARGER          "        , Get_EN_DCDC          , Set_EN_DCDC       , 0                                 ," "         },
  {17 , 54 ,'g'        , VTYPE__BOOL ,"%-4d"   , "DCDC_MODE           "        , Get_DCDC_MODE        , Set_DCDC_MODE     , 0                                 ," "         },
  {18 , 54 ,'h'        , VTYPE__BOOL ,"%-4d"   , "SW_INV_PATH         "        , Get_SW_INV_PATH      , Set_SW_INV_PATH   , 0                                 ," "         },
  {19 , 54 ,'j'        , VTYPE__BOOL ,"%-4d"   , "SW_MAINS_PATH       "        , Get_SW_MAINS_PATH    , Set_SW_MAINS_PATH , 0                                 ," "         },
};

#define EDIT_LINE_RAW  37

#define SIGS_MAP_SZ  (sizeof(pcb_sigs_map)/sizeof(pcb_sigs_map[0]))

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float   Value_ACCUM_SUM_I(int32_t v)
{
  return Get_float_val(&sys.accum_sum_i);
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float   Value_ACCUM_CHARGE(int32_t v)
{
  return Get_float_val(&sys.accum_charge);
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Value_PwrSrcPower(int32_t v)
{
  return Get_float_val(&sys.pwr_src_power);
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Value_AccumChargePower(int32_t v)
{
  return Get_float_val(&sys.accum_charge_power);
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Value_AccumDisChargePower(int32_t v)
{
  return Get_float_val(&sys.accum_discharge_power);
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Value_chaging_pwr_loss(int32_t v)
{
  return Get_float_val(&sys.chaging_pwr_loss);
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Value_charging_efficiency(int32_t v)
{
  return Get_float_val(&sys.charging_efficiency);
}

/*-----------------------------------------------------------------------------------------------------


  \param n
-----------------------------------------------------------------------------------------------------*/
static void _Print_signal_value(const T_sigs_map *arr, uint32_t n)
{
  int32_t v = 0;
  GET_MCBL;

  if (arr[n].getv != 0)
  {

    v = arr[n].getv();

    if (arr[n].vtype == VTYPE__BOOL)
    {
      if (v == 0) MPRINTF("0");
      else MPRINTF("1");
    }
    else if (arr[n].vtype == VTYPE_INT32)
    {
      float f = 0.0f;
      if (arr[n].cfunc != 0) f =((T_conv_func)arr[n].cfunc)(v);
      MPRINTF(arr[n].fmt,v);
      MPRINTF(arr[n].cfmt,f);
    }
  }
  else
  {
    if (arr[n].vtype == VTYPE_FLOAT)
    {
      float f = 0.0f;
      if (arr[n].cfunc != 0) f =((T_conv_func)arr[n].cfunc)(v);
      MPRINTF(arr[n].cfmt,f);
    }
    else if (arr[n].vtype == VTYPE_STRNG)
    {

    }

  }
}
/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
static void Do_Signal_dignostics(uint8_t keycode)
{
  uint32_t               i;
  uint32_t               row;
  uint8_t                b;
  int32_t                refr_tcks = 200;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(VT100_CLR_LINE"Signals diagnostic. ESC-exit\n\r");

  do
  {
    row = 2;
    // Вывод сигналов
    for (i=0; i < SIGS_MAP_SZ; i++)
    {
      MPRINTF(VT100_CURSOR_SET,pcb_sigs_map[i].row,pcb_sigs_map[i].col);

      if (pcb_sigs_map[i].setv != 0)
      {
        MPRINTF("[%c]", pcb_sigs_map[i].cmd_sym);
      }
      else
      {
        MPRINTF("   ");
      }
      MPRINTF(" %s = ", pcb_sigs_map[i].name);
      _Print_signal_value(pcb_sigs_map, i);
      row++;

    }

    if (WAIT_CHAR(&b, refr_tcks) == RES_OK)
    {
      switch (b)
      {
      case VT100_ESC:
        return;
      default:
        // Ищем символ в таблице и если находим то выполняем редактирование или переключение
        for (i=0; i < SIGS_MAP_SZ; i++)
        {
          if (b == pcb_sigs_map[i].cmd_sym)
          {
            if (pcb_sigs_map[i].vtype == VTYPE__BOOL)
            {
              if (pcb_sigs_map[i].getv() != 0) pcb_sigs_map[i].setv(0);
              else pcb_sigs_map[i].setv(1);
            }
            else if (pcb_sigs_map[i].vtype == VTYPE_INT32)
            {
              int32_t v = pcb_sigs_map[i].getv();
              Edit_integer_val(EDIT_LINE_RAW,&v, -INT32_MAX, INT32_MAX);
              pcb_sigs_map[i].setv(v);
            }
          }
        }
        break;
      }
    }

  }while (1);
}


