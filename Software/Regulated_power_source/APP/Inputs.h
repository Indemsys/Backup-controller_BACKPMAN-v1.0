#ifndef BCKUPC10_INPUTS_CONTROLLER_H
  #define BCKUPC10_INPUTS_CONTROLLER_H

// Структура для обработчика подавления дребезга
typedef struct
{
    int8_t    val;
    int8_t    curr;
    int8_t    prev;
    uint32_t  cnt;
    uint8_t   init;

} T_bncf;

  #define GEN_SW     0
  #define ESC_SW     1

// Структура алгоритма определения состояния сигналов
typedef struct
{
    uint8_t            itype;    // Тип  сигнала. 0 - простой контакт бистабильный, 1 - контакт в цепи безопасности с 3-я состояниями
    uint16_t           *p_smpl1; // Указатель на результат сэмплирования напряжения на контакте с более высоким напряжением
    uint16_t           *p_smpl2; // Указатель на результат сэмплирования напряжения на контакте с более низким напряжением
    uint16_t           lbound;   // Граница между логическим 0 и 1 на входе
    uint32_t           l0_time;  // Время устоявшегося состояния для фиксации низкого уровня сигнала
    uint32_t           l1_time;  // Время устоявшегося состояния для фиксации высокого уровня сигнала
    uint32_t           lu_time;  // Время устоявшегося состояния для фиксации неопределенного уровня сигнала
    int8_t             *val;     // Указатель на переменную для сохранения вычисленного состояния входа
    int8_t             *val_prev; // Указатель на переменную для сохранения предыдущего состояния входа
    int8_t             *flag;    // Указатель на флаг переменной. Флаг не равный нулю указывает на произошедшее изменение состояния переменной
    T_bncf             pbncf;    // Структура для алгоритма фильтрации дребезга
} T_input_cbl;


  #define EXT_ANIN10V_BOUND     ((uint16_t)(10.0f*EXT_AIN_SCALE/AIN_SCALE)) //
  #define EXT_ANIN5V_BOUND      ((uint16_t)(5.0f*EXT_AIN_SCALE/AIN_SCALE)) //
  #define EXT_ANIN3V_BOUND      ((uint16_t)(3.0f*EXT_AIN_SCALE/AIN_SCALE)) //
  #define EXT_ANIN1V_BOUND      ((uint16_t)(1.0f*EXT_AIN_SCALE/AIN_SCALE)) //
  #define EXT_ANIN08V_BOUND     ((uint16_t)(0.8f*EXT_AIN_SCALE/AIN_SCALE)) //
  #define ANIN1V_BOUND          ((uint16_t)(1.0f/AIN_SCALE)) //


  #define   ESC_BNC_L  (2000/MAIN_CYCLE_PERIOD_US)     // Количество тактов для отработки антидребезга датчика в цепи безопасности при переходе в активное состояние (разорван)
  #define   ESC_BNC_H  (50000/MAIN_CYCLE_PERIOD_US)    // Количество тактов для отработки антидребезга датчика в цепи безопасности при переходе в пассивное состояние (замкнут)
  #define   ESC_BNC_U  (50000/MAIN_CYCLE_PERIOD_US)    // Количество тактов для отработки антидребезга датчика в цепи безопасности при переходе в неопределенное состояние


  #define   GEN_BNC_L  (10000/MAIN_CYCLE_PERIOD_US)    // Количество тактов для отработки антидребезга обычного датчика при переходе в состояние 0
  #define   GEN_BNC_H  (10000/MAIN_CYCLE_PERIOD_US)    // Количество тактов для отработки антидребезга обычного датчика при переходе в состояние 1



typedef struct
{
    int8_t MAINS_PATH_STATE;    //
    int8_t INV_PATH_STATE;      //

} T_BCKUPC10_inputs;

typedef struct
{
    int8_t f_MAINS_PATH_STATE;
    int8_t f_INV_PATH_STATE;

} T_BCKUPC10_inputs_flags;

void      BCKUPC10_Inputs_handling(void);

uint32_t  Inputs_do_processing(T_input_cbl *scbl);
void      Manual_Encoder_handling(void);

uint8_t   Is_man_enc_sw_pressed(void);
uint8_t   Is_man_enc_sw_released(void);
uint8_t   Is_man_enc_a_pressed(void);
uint8_t   Is_man_enc_a_released(void);
uint8_t   Is_man_enc_b_pressed(void);
uint8_t   Is_man_enc_b_released(void);


#endif // INPUTS_PROCESSING_H



