/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2019 NXP
 *
 * License: NXP LA_OPT_NXP_Software_License
 *
 * NXP Confidential. This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms.  If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.  This code may only be used in a microprocessor,
 * microcontroller, sensor or digital signal processor ("NXP Product")
 * supplied directly or indirectly from NXP.  See the full NXP Software
 * License Agreement in license/LA_OPT_NXP_Software_License.pdf
 *
 * FreeMASTER Communication Driver - LPUART low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Numeric identifier to help pre-processor to identify whether our driver is used or not. */
#define FMSTR_SERIAL_MCUX_LPUART_ID 1

#if (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_MCUX_LPUART_ID)

  #include "freemaster_serial_lpuart.h"

  #if (!(FMSTR_DISABLE))

    #include "fsl_common.h"
//#include "fsl_lpuart.h"

    #include "freemaster_protocol.h"
    #include "freemaster_serial.h"

/******************************************************************************
* Adapter configuration
******************************************************************************/

/***********************************
*  local variables
***********************************/

/* Serial base address */
    #ifdef FMSTR_SERIAL_BASE
static LPUART_Type *fmstr_serialBaseAddr = FMSTR_SERIAL_BASE;
    #else
static LPUART_Type *fmstr_serialBaseAddr = NULL;
    #endif

/***********************************
*  local function prototypes
***********************************/

/* Interface function - Initialization of serial UART driver adapter */
static FMSTR_BOOL _FMSTR_SerialLpuartInit(void);
static void _FMSTR_SerialLpuartEnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableReceive(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableReceiveInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_SerialLpuartIsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitterActive(void);
static void _FMSTR_SerialLpuartPutChar(FMSTR_BCHR  ch);
static FMSTR_BCHR _FMSTR_SerialLpuartGetChar(void);
static void _FMSTR_SerialLpuartFlush(void);

/***********************************
*  global variables
***********************************/
/* Interface of this serial UART driver */
const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_MCUX_LPUART =
{
  .Init                           = _FMSTR_SerialLpuartInit,
  .EnableTransmit                 = _FMSTR_SerialLpuartEnableTransmit,
  .EnableReceive                  = _FMSTR_SerialLpuartEnableReceive,
  .EnableTransmitInterrupt        = _FMSTR_SerialLpuartEnableTransmitInterrupt,
  .EnableTransmitCompleteInterrupt= _FMSTR_SerialLpuartEnableTransmitCompleteInterrupt,
  .EnableReceiveInterrupt         = _FMSTR_SerialLpuartEnableReceiveInterrupt,
  .IsTransmitRegEmpty             = _FMSTR_SerialLpuartIsTransmitRegEmpty,
  .IsReceiveRegFull               = _FMSTR_SerialLpuartIsReceiveRegFull,
  .IsTransmitterActive            = _FMSTR_SerialLpuartIsTransmitterActive,
  .PutChar                        = _FMSTR_SerialLpuartPutChar,
  .GetChar                        = _FMSTR_SerialLpuartGetChar,
  .Flush                          = _FMSTR_SerialLpuartFlush,

};

/*-----------------------------------------------------------------------------------------------------


  \param base
  \param enable
-----------------------------------------------------------------------------------------------------*/
static inline void LPUART_EnableTx(LPUART_Type *base, bool enable)
{
  if (enable)
  {
    base->CTRL |= LPUART_CTRL_TE_MASK;
  }
  else
  {
    base->CTRL &= ~LPUART_CTRL_TE_MASK;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param base
  \param enable
-----------------------------------------------------------------------------------------------------*/
static inline void LPUART_EnableRx(LPUART_Type *base, bool enable)
{
  if (enable)
  {
    base->CTRL |= LPUART_CTRL_RE_MASK;
  }
  else
  {
    base->CTRL &= ~LPUART_CTRL_RE_MASK;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param base
  \param mask
-----------------------------------------------------------------------------------------------------*/
void LPUART_EnableInterrupts(LPUART_Type *base, uint32_t mask)
{
  base->BAUD |=((mask << 8U) & (LPUART_BAUD_LBKDIE_MASK | LPUART_BAUD_RXEDGIE_MASK));
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
  base->FIFO =(base->FIFO & ~(LPUART_FIFO_TXOF_MASK | LPUART_FIFO_RXUF_MASK)) | ((mask << 8U) & (LPUART_FIFO_TXOFE_MASK | LPUART_FIFO_RXUFE_MASK));
#endif
  mask &= 0xFFFFFF00U;
  base->CTRL |= mask;
}

/*-----------------------------------------------------------------------------------------------------


  \param base
  \param mask
-----------------------------------------------------------------------------------------------------*/
void LPUART_DisableInterrupts(LPUART_Type *base, uint32_t mask)
{
  base->BAUD &= ~((mask << 8U) & (LPUART_BAUD_LBKDIE_MASK | LPUART_BAUD_RXEDGIE_MASK));
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
  base->FIFO =(base->FIFO & ~(LPUART_FIFO_TXOF_MASK | LPUART_FIFO_RXUF_MASK)) &  ~((mask << 8U) & (LPUART_FIFO_TXOFE_MASK | LPUART_FIFO_RXUFE_MASK));
#endif
  mask &= 0xFFFFFF00U;
  base->CTRL &= ~mask;
}

/*-----------------------------------------------------------------------------------------------------
  \param base

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t LPUART_GetStatusFlags(LPUART_Type *base)
{
  uint32_t temp;
  temp = base->STAT;
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
  temp |=(base->FIFO & (LPUART_FIFO_TXEMPT_MASK | LPUART_FIFO_RXEMPT_MASK | LPUART_FIFO_TXOF_MASK | LPUART_FIFO_RXUF_MASK))>>16U;
#endif
  return temp;
}

/*-----------------------------------------------------------------------------------------------------


  \param base
  \param data
-----------------------------------------------------------------------------------------------------*/
static inline void LPUART_WriteByte(LPUART_Type *base, uint8_t data)
{
  base->DATA = data;
}

/*-----------------------------------------------------------------------------------------------------


  \param base

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static inline uint8_t LPUART_ReadByte(LPUART_Type *base)
{
#if defined(FSL_FEATURE_LPUART_HAS_7BIT_DATA_SUPPORT) && FSL_FEATURE_LPUART_HAS_7BIT_DATA_SUPPORT
  uint32_t ctrl = base->CTRL;
  uint8_t result;
  bool isSevenDataBits =(((ctrl & LPUART_CTRL_M7_MASK) != 0U) ||
                        (((ctrl & LPUART_CTRL_M7_MASK) == 0U) && ((ctrl & LPUART_CTRL_M_MASK) == 0U) &&
                        ((ctrl & LPUART_CTRL_PE_MASK) != 0U)));

  if (isSevenDataBits)
  {
    result = (uint8_t)(base->DATA & 0x7FU);
  }
  else
  {
    result = (uint8_t)base->DATA;
  }

  return result;
#else
  return (uint8_t)(base->DATA);
#endif
}


/**************************************************************************//*!
*
* @brief    Serial communication initialization
*
******************************************************************************/
static FMSTR_BOOL _FMSTR_SerialLpuartInit(void)
{
  /* Valid runtime module address must be assigned */
  if (!fmstr_serialBaseAddr) return FMSTR_FALSE;


#if FMSTR_SERIAL_SINGLEWIRE
  /* Enable single wire mode and force TX and RX to be enabled all the time. */
  fmstr_serialBaseAddr->CTRL |= LPUART_CTRL_LOOPS_MASK | LPUART_CTRL_RSRC_MASK;
  LPUART_EnableTx(fmstr_serialBaseAddr, 1);
  LPUART_EnableRx(fmstr_serialBaseAddr, 1);
#endif

  return FMSTR_TRUE;
}

/**************************************************************************//*!
*
* @brief    Enable/Disable Serial transmitter
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableTransmit(FMSTR_BOOL enable)
{
#if FMSTR_SERIAL_SINGLEWIRE
  /* In single-wire mode, the Transmitter is never physically disabled since it was
     enabled in the Init() call. The TX pin direction is being changed only. */
  if (enable) fmstr_serialBaseAddr->CTRL |= LPUART_CTRL_TXDIR_MASK;
  else fmstr_serialBaseAddr->CTRL &= ~LPUART_CTRL_TXDIR_MASK;
#else
  /* In normal mode (or "external" single-wire mode) the Transmitter is controlled. */
  LPUART_EnableTx(fmstr_serialBaseAddr, enable);
#endif
}

/**************************************************************************//*!
*
* @brief    Enable/Disable Serial receiver
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableReceive(FMSTR_BOOL enable)
{
#if FMSTR_SERIAL_SINGLEWIRE
  /* In single-wire mode, the Receiver is never physically disabled since it was
     enabled in the Init() call. The TX pin direction is being changed only. */
  FMSTR_UNUSED(enable);
#else
  /* In normal mode (or "external" single-wire mode) the Receiver is controlled. */
  LPUART_EnableRx(fmstr_serialBaseAddr, enable);
#endif
}

/**************************************************************************//*!
*
* @brief    Enable/Disable interrupt from transmit register empty event
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableTransmitInterrupt(FMSTR_BOOL enable)
{
  if (enable) LPUART_EnableInterrupts(fmstr_serialBaseAddr, kLPUART_TxDataRegEmptyInterruptEnable);
  else LPUART_DisableInterrupts(fmstr_serialBaseAddr, kLPUART_TxDataRegEmptyInterruptEnable);
}

/**************************************************************************//*!
*
* @brief    Enable/Disable interrupt from transmit complete event
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
  if (enable) LPUART_EnableInterrupts(fmstr_serialBaseAddr, kLPUART_TransmissionCompleteInterruptEnable);
  else LPUART_DisableInterrupts(fmstr_serialBaseAddr, kLPUART_TransmissionCompleteInterruptEnable);
}

/**************************************************************************//*!
*
* @brief    Enable/Disable interrupt from receive register full event
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableReceiveInterrupt(FMSTR_BOOL enable)
{
  if (enable) LPUART_EnableInterrupts(fmstr_serialBaseAddr, kLPUART_RxDataRegFullInterruptEnable);
  else LPUART_DisableInterrupts(fmstr_serialBaseAddr, kLPUART_RxDataRegFullInterruptEnable);
}

/**************************************************************************//*!
*
* @brief    Returns TRUE if the transmit register is empty, and it's possible to put next char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitRegEmpty(void)
{
  uint32_t sr = LPUART_GetStatusFlags(fmstr_serialBaseAddr);

  return (sr & kLPUART_TxDataRegEmptyFlag);
}

/**************************************************************************//*!
*
* @brief    Returns TRUE if the receive register is full, and it's possible to get received char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialLpuartIsReceiveRegFull(void)
{
  uint32_t sr = LPUART_GetStatusFlags(fmstr_serialBaseAddr);

  return (sr & kLPUART_RxDataRegFullFlag);
}

/**************************************************************************//*!
*
* @brief    Returns TRUE if the transmitter is still active
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitterActive(void)
{
  uint32_t sr = LPUART_GetStatusFlags(fmstr_serialBaseAddr);

  return !(sr & kLPUART_TransmissionCompleteFlag);
}

/**************************************************************************//*!
*
* @brief    The function puts the char for transmit
*
******************************************************************************/

static void _FMSTR_SerialLpuartPutChar(FMSTR_BCHR  ch)
{
  LPUART_WriteByte(fmstr_serialBaseAddr, ch);
}

/**************************************************************************//*!
*
* @brief    The function gets the received char
*
******************************************************************************/
static FMSTR_BCHR _FMSTR_SerialLpuartGetChar(void)
{
  return LPUART_ReadByte(fmstr_serialBaseAddr);
}

/**************************************************************************//*!
*
* @brief    The function sends buffered data
*
******************************************************************************/
static void _FMSTR_SerialLpuartFlush(void)
{
}

/**************************************************************************//*!
*
* @brief    Assigning FreeMASTER communication module base address
*
******************************************************************************/
void FMSTR_SerialSetBaseAddress(LPUART_Type *base)
{
  fmstr_serialBaseAddr = base;
}

/**************************************************************************//*!
*
* @brief    API: Interrupt handler call
*
* This Interrupt Service Routine handles the UART  interrupts for the FreeMASTER
* driver. In case you want to handle the interrupt in the application yourselves,
* call the FMSTR_ProcessSerial function which
* does the same job but is not compiled as an Interrupt Service Routine.
*
* In poll-driven mode (FMSTR_POLL_DRIVEN) this function does nothing.
*
******************************************************************************/

void FMSTR_SerialIsr(void)
{
  /* process incomming or just transmitted byte */
#if (FMSTR_LONG_INTR) || (FMSTR_SHORT_INTR)
  FMSTR_ProcessSerial();
#endif
}

  #else /* (!(FMSTR_DISABLE)) */

void FMSTR_SerialSetBaseAddress(LPUART_Type *base)
{
  FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

  #endif /* (!(FMSTR_DISABLE)) */
#endif /* (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_MCUX_LPUART_ID) */
