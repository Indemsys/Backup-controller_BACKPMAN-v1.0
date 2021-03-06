#ifndef __PARAMS_H
  #define __PARAMS_H

#define  APP_PROFILE        BKMAN1_2
#define  MAIN_PARAMS_ROOT   BKMAN1_2_main
#define  PARAMS_ROOT        BKMAN1_2_0

#define  DWVAR_SIZE        3
#define  PARMNU_ITEM_NUM   2

#define  SELECTORS_NUM     2



  #define VAL_LOCAL_EDITED 0x01  //
  #define VAL_READONLY     0x02  // Можно только читать
  #define VAL_PROTECT      0x04  // Защишено паролем
  #define VAL_UNVISIBLE    0x08  // Не выводится на дисплей
  #define VAL_NOINIT       0x10  // Не инициализируется


enum vartypes
{
    tint8u  = 1,
    tint16u  = 2,
    tint32u  = 3,
    tfloat  = 4,
    tarrofdouble  = 5,
    tstring  = 6,
    tarrofbyte  = 7,
    tint32s  = 8,
};


enum enm_parmnlev
{
    BKMAN1_2_0,
    BKMAN1_2_main,
    BKMAN1_2_General,
};


typedef struct 
{
  enum enm_parmnlev prevlev;
  enum enm_parmnlev currlev;
  const char* name;
  const char* shrtname;
  const char  visible;
}
T_parmenu;


typedef struct
{
  const uint8_t*     var_name;        // Имя параметра
  const uint8_t*     var_description; // Строковое описание
  const uint8_t*     var_alias;       // Короткая аббревиатура
  void*              val;          // Указатель на значение переменной в RAM
  enum  vartypes     vartype;      // Идентификатор типа переменной
  float              defval;       // Значение по умолчанию
  float              minval;       // Минимальное возможное значение
  float              maxval;       // Максимальное возможное значение  
  uint8_t            attr;         // Аттрибуты переменной
  unsigned int       parmnlev;     // Подгруппа к которой принадлежит параметр
  const  void*       pdefval;      // Указатель на данные для инициализации
  const  char*       format;       // Строка форматирования при выводе на дисплей
  void               (*func)(void);// Указатель на функцию выполняемую после редактирования
  uint16_t           varlen;       // Длинна переменной
  uint32_t           menu_pos;     // Позиция в меню
  uint32_t           selector_id;  // Идентификатор селектора
} T_work_params;


typedef struct
{
  uint32_t           val;
  const uint8_t*     caption;
  int32_t            img_indx;
} T_selector_items;


typedef struct
{
  const uint8_t*           name;
  uint32_t                 items_cnt;
  const T_selector_items*  items_list;
} T_selectors_list;

extern const T_selectors_list selectors_list[];

typedef struct
{
  uint8_t        name[64];                      // Product  name | def.val.= BACKPMAN10
  uint8_t        vcom_mode;                     // Select VCOM mode (0 - VT100 terminal, 1 - FreeMaster)  | def.val.= 0
  uint8_t        ver[64];                       // Firmware version | def.val.= 1.0
} WVAR_TYPE;


#endif



// Selector description:  Выбор между Yes и No
#define BINARY_NO                                 0
#define BINARY_YES                                1
