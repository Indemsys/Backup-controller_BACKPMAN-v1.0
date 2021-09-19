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

#ifndef __FREEMASTER_SERIAL_LPUART_H
  #define __FREEMASTER_SERIAL_LPUART_H

/******************************************************************************
* Required header files include check
******************************************************************************/
  #ifndef __FREEMASTER_H
    #error Please include the freemaster.h master header file before the freemaster_serial_uart.h
  #endif

/* This low-level driver uses the MCUXpresso SDK peripheral structure types. */
  #include "fsl_device_registers.h"

  #ifdef __cplusplus
extern "C"
{
  #endif


  enum _lpuart_interrupt_enable
  {
    kLPUART_RxActiveEdgeInterruptEnable         =(LPUART_BAUD_RXEDGIE_MASK >> 8), /*!< Receive Active Edge. */
    kLPUART_TxDataRegEmptyInterruptEnable       =(LPUART_CTRL_TIE_MASK),          /*!< Transmit data register empty. */
    kLPUART_TransmissionCompleteInterruptEnable =(LPUART_CTRL_TCIE_MASK),         /*!< Transmission complete. */
    kLPUART_RxDataRegFullInterruptEnable        =(LPUART_CTRL_RIE_MASK),          /*!< Receiver data register full. */
    kLPUART_IdleLineInterruptEnable             =(LPUART_CTRL_ILIE_MASK),         /*!< Idle line. */
    kLPUART_RxOverrunInterruptEnable            =(LPUART_CTRL_ORIE_MASK),         /*!< Receiver Overrun. */
    kLPUART_NoiseErrorInterruptEnable           =(LPUART_CTRL_NEIE_MASK),         /*!< Noise error flag. */
    kLPUART_FramingErrorInterruptEnable         =(LPUART_CTRL_FEIE_MASK),         /*!< Framing error flag. */
    kLPUART_ParityErrorInterruptEnable          =(LPUART_CTRL_PEIE_MASK),         /*!< Parity error flag. */
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
    kLPUART_TxFifoOverflowInterruptEnable  =(LPUART_FIFO_TXOFE_MASK >> 8), /*!< Transmit FIFO Overflow. */
    kLPUART_RxFifoUnderflowInterruptEnable =(LPUART_FIFO_RXUFE_MASK >> 8), /*!< Receive FIFO Underflow. */
#endif
  };

  enum _lpuart_flags
  {
    kLPUART_TxDataRegEmptyFlag         =(LPUART_STAT_TDRE_MASK), /*!< Transmit data register empty flag, sets when transmit buffer is empty */
    kLPUART_TransmissionCompleteFlag   =(LPUART_STAT_TC_MASK),   /*!< Transmission complete flag, sets when transmission activity complete */
    kLPUART_RxDataRegFullFlag          =(LPUART_STAT_RDRF_MASK), /*!< Receive data register full flag, sets when the receive data buffer is full */
    kLPUART_IdleLineFlag               =(LPUART_STAT_IDLE_MASK), /*!< Idle line detect flag, sets when idle line detected */
    kLPUART_RxOverrunFlag              =(LPUART_STAT_OR_MASK),   /*!< Receive Overrun, sets when new data is received before data is read from receive register */
    kLPUART_NoiseErrorFlag             =(LPUART_STAT_NF_MASK),   /*!< Receive takes 3 samples of each received bit.  If any of these samples differ, noise flag sets */
    kLPUART_FramingErrorFlag           =(LPUART_STAT_FE_MASK),   /*!< Frame error flag, sets if logic 0 was detected where stop bit expected */
    kLPUART_ParityErrorFlag            =(LPUART_STAT_PF_MASK),   /*!< If parity enabled, sets upon parity error detection */
#if defined(FSL_FEATURE_LPUART_HAS_LIN_BREAK_DETECT) && FSL_FEATURE_LPUART_HAS_LIN_BREAK_DETECT
    kLPUART_LinBreakFlag               =(int)(LPUART_STAT_LBKDIF_MASK), /*!< LIN break detect interrupt flag, sets when LIN break char detected and LIN circuit enabled */
#endif
    kLPUART_RxActiveEdgeFlag           =(LPUART_STAT_RXEDGIF_MASK), /*!< Receive pin active edge interrupt flag, sets when active edge detected */
    kLPUART_RxActiveFlag               =(LPUART_STAT_RAF_MASK),     /*!< Receiver Active Flag (RAF), sets at beginning of valid start bit */
#if defined(FSL_FEATURE_LPUART_HAS_ADDRESS_MATCHING) && FSL_FEATURE_LPUART_HAS_ADDRESS_MATCHING
    kLPUART_DataMatch1Flag             = LPUART_STAT_MA1F_MASK,     /*!< The next character to be read from LPUART_DATA matches MA1*/
    kLPUART_DataMatch2Flag             = LPUART_STAT_MA2F_MASK,     /*!< The next character to be read from LPUART_DATA matches MA2*/
#endif
#if defined(FSL_FEATURE_LPUART_HAS_EXTENDED_DATA_REGISTER_FLAGS) && FSL_FEATURE_LPUART_HAS_EXTENDED_DATA_REGISTER_FLAGS
    kLPUART_NoiseErrorInRxDataRegFlag  =(LPUART_DATA_NOISY_MASK >> 10),   /*!< NOISY bit, sets if noise detected in current data word */
    kLPUART_ParityErrorInRxDataRegFlag =(LPUART_DATA_PARITYE_MASK >> 10), /*!< PARITY bit, sets if noise detected in current data word */
#endif
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
    kLPUART_TxFifoEmptyFlag            =(LPUART_FIFO_TXEMPT_MASK >> 16), /*!< TXEMPT bit, sets if transmit buffer is empty */
    kLPUART_RxFifoEmptyFlag            =(LPUART_FIFO_RXEMPT_MASK >> 16), /*!< RXEMPT bit, sets if receive buffer is empty */
    kLPUART_TxFifoOverflowFlag         =(LPUART_FIFO_TXOF_MASK >> 16),  /*!< TXOF bit, sets if transmit buffer overflow occurred */
    kLPUART_RxFifoUnderflowFlag        =(LPUART_FIFO_RXUF_MASK >> 16),  /*!< RXUF bit, sets if receive buffer underflow occurred */
#endif
  };


  void     FMSTR_SerialSetBaseAddress(LPUART_Type *base);
  void     FMSTR_SerialIsr(void);

  uint32_t LPUART_GetStatusFlags(LPUART_Type *base);
  void     LPUART_EnableInterrupts(LPUART_Type *base, uint32_t mask);
  void     LPUART_DisableInterrupts(LPUART_Type *base, uint32_t mask);


#ifdef __cplusplus
}
#endif


#endif /* __FREEMASTER_SERIAL_UART_H */

