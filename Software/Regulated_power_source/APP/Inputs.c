// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.05
// 09:27:50
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern void FMSTR_Recorder(void);

T_BCKUPC10_inputs        inp;
T_BCKUPC10_inputs        inp_prev;
T_BCKUPC10_inputs_flags  inp_flags;

static T_input_cbl BCKUPC10_sigs_cbls[] =
{
  { GEN_SW  ,&g_ADC_results.smpl_MAINS_PATH_STATE   ,&g_ADC_results.smpl_MAINS_PATH_STATE  ,  EXT_ANIN10V_BOUND,  ESC_BNC_L  ,ESC_BNC_H  ,ESC_BNC_U ,&inp.MAINS_PATH_STATE  ,&inp_prev.MAINS_PATH_STATE ,&inp_flags.f_MAINS_PATH_STATE },
  { GEN_SW  ,&g_ADC_results.smpl_INV_PATH_STATE     ,&g_ADC_results.smpl_INV_PATH_STATE    ,  EXT_ANIN10V_BOUND,  ESC_BNC_L  ,ESC_BNC_H  ,ESC_BNC_U ,&inp.INV_PATH_STATE    ,&inp_prev.INV_PATH_STATE   ,&inp_flags.f_INV_PATH_STATE   },
};


#define BCKUPC10_SIGS_NUM (sizeof(BCKUPC10_sigs_cbls)/sizeof(BCKUPC10_sigs_cbls[0]))

/*------------------------------------------------------------------------------
  Процедура определения состояния сигнала

  Возвращает 1 если значение сигнала изменилось, иначе 0
 ------------------------------------------------------------------------------*/
uint32_t Inputs_do_processing(T_input_cbl *scbl)
{
  uint8_t upper_sig, lower_sig;

  if (scbl->itype == GEN_SW)
  {
    // Обрабатываем сигнал с бистабильного датчика
    if (*scbl->p_smpl1 > scbl->lbound) scbl->pbncf.curr = 1;
    else scbl->pbncf.curr = 0;
  }
  else if (scbl->itype == ESC_SW)
  {
    // Обрабатываем сигнал с датчика обладающего неопределенным состоянием (контакты в цепи безопасности)
    if (*scbl->p_smpl1 > scbl->lbound) upper_sig = 1;
    else upper_sig = 0;

    if (*scbl->p_smpl2 > scbl->lbound) lower_sig = 1;
    else lower_sig = 0;

    if ((upper_sig) && (lower_sig))
    {
      scbl->pbncf.curr = 0; // 0 - замкнутое состояние
    }
    else if (((upper_sig) && (!lower_sig)) || ((!upper_sig) && (lower_sig)))
    {
      scbl->pbncf.curr = 1; // 1 - разомкнутое состояние
    }
    else
    {
      scbl->pbncf.curr = 0; // В данной схеме не может быть неопределенных состояний
    }

  }
  else return 0;


  if (scbl->pbncf.init == 0)
  {
    scbl->pbncf.init = 1;
    scbl->pbncf.prev = scbl->pbncf.curr;
    *scbl->val  = scbl->pbncf.curr;
    *scbl->val_prev =*scbl->val;
    return 0;
  }

  if (scbl->pbncf.prev != scbl->pbncf.curr)
  {
    scbl->pbncf.cnt = 0;
    scbl->pbncf.prev = scbl->pbncf.curr;
  }

  if ((scbl->pbncf.curr == 0) && (scbl->pbncf.cnt == scbl->l0_time))
  {
    *scbl->val_prev =*scbl->val;
    *scbl->val = scbl->pbncf.curr;
    *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else if ((scbl->pbncf.curr == 1) && (scbl->pbncf.cnt == scbl->l1_time))
  {
    *scbl->val_prev =*scbl->val;
    *scbl->val = scbl->pbncf.curr;
    *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else if ((scbl->pbncf.curr == -1) && (scbl->pbncf.cnt == scbl->lu_time))
  {
    *scbl->val_prev =*scbl->val;
    *scbl->val = scbl->pbncf.curr;
    *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else
  {
    if (scbl->pbncf.cnt < UINT32_MAX)
    {
      scbl->pbncf.cnt++;
    }
  }
  return 0;
}

/*------------------------------------------------------------------------------
  Обработка и идентификация состояний входных сигналов
  Вызывается с периодом MAIN_CYCLE_PERIOD_US мкс (1024 мкс)

 ------------------------------------------------------------------------------*/
void BCKUPC10_Inputs_handling(void)
{
  uint32_t i;
  for (i=0; i < BCKUPC10_SIGS_NUM; i++)
  {
    Inputs_do_processing(&BCKUPC10_sigs_cbls[i]);
  }
}


typedef struct
{
  uint8_t  prev_state;
  uint16_t state_cnt;
  uint8_t  state;
  uint8_t  state_changed;

} T_man_enc_cbl;

T_man_enc_cbl  enc_sw_cbl;
T_man_enc_cbl  enc_a_cbl;
T_man_enc_cbl  enc_b_cbl;

/*-----------------------------------------------------------------------------------------------------


  \param pcbl
  \param current_state
-----------------------------------------------------------------------------------------------------*/
static void Man_enc_sig_handling(T_man_enc_cbl *pcbl, uint8_t current_state, uint16_t bounce_timeout)
{
  TX_INTERRUPT_SAVE_AREA

  TX_DISABLE;
  if (current_state == pcbl->prev_state)
  {
    if (pcbl->state_cnt != 0xFFFF) pcbl->state_cnt++;
    if (pcbl->state_cnt == bounce_timeout)
    {
      pcbl->state = current_state;
      pcbl->state_changed = 1;
    }
  }
  else
  {
    pcbl->prev_state = current_state;
    pcbl->state_cnt = 0;
  }
  TX_RESTORE;
}

/*-----------------------------------------------------------------------------------------------------


  \param st

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t Is_man_enc_sig_changed_to(T_man_enc_cbl *pcbl, uint8_t st)
{
  TX_INTERRUPT_SAVE_AREA
  uint8_t res = 0;
  TX_DISABLE;
  if (pcbl->state_changed != 0)
  {
    if (pcbl->state == st)
    {
      res = 1;
      pcbl->state_changed = 0;
    }
  }
  TX_RESTORE;
  return res;
}

uint8_t Is_man_enc_sw_pressed(void)  { return Is_man_enc_sig_changed_to(&enc_sw_cbl, 0); }
uint8_t Is_man_enc_sw_released(void) { return Is_man_enc_sig_changed_to(&enc_sw_cbl, 1); }
uint8_t Is_man_enc_a_pressed(void)   { return Is_man_enc_sig_changed_to(&enc_a_cbl,  1); }
uint8_t Is_man_enc_a_released(void)  { return Is_man_enc_sig_changed_to(&enc_a_cbl,  0); }
uint8_t Is_man_enc_b_pressed(void)   { return Is_man_enc_sig_changed_to(&enc_b_cbl,  1); }
uint8_t Is_man_enc_b_released(void)  { return Is_man_enc_sig_changed_to(&enc_b_cbl,  0); }

/*-----------------------------------------------------------------------------------------------------
  Обработка и идентификация состояний сигналов ручного энкодера

  \param void
-----------------------------------------------------------------------------------------------------*/
void Manual_Encoder_handling(void)
{
  Man_enc_sig_handling(&enc_sw_cbl, Get_ENC_SW(), 50);
  Man_enc_sig_handling(&enc_a_cbl, Get_ENC_A(), 4);
  Man_enc_sig_handling(&enc_b_cbl, Get_ENC_B(), 4);
}

