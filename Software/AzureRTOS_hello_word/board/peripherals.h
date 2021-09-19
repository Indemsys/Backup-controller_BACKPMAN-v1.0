/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_common.h"
#include "fsl_lpuart.h"
#include "fsl_clock.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Definition of peripheral ID */
#define LPUART0_PERIPHERAL LPUART0
/* Definition of the clock source frequency */
#define LPUART0_CLOCK_SOURCE 60000000UL
/* Rx transfer buffer size. */
#define LPUART0_RX_BUFFER_SIZE 10
/* Rx transfer buffer size. */
#define LPUART0_TX_BUFFER_SIZE 10
/* LPUART0 interrupt vector ID (number). */
#define LPUART0_SERIAL_RX_IRQN LPUART0_RX_IRQn
/* LPUART0 interrupt vector ID (number). */
#define LPUART0_SERIAL_TX_IRQN LPUART0_TX_IRQn

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
extern const lpuart_config_t LPUART0_config;
extern lpuart_handle_t LPUART0_handle;
extern uint8_t LPUART0_rxBuffer[LPUART0_RX_BUFFER_SIZE];
extern const lpuart_transfer_t LPUART0_rxTransfer;
extern uint8_t LPUART0_txBuffer[LPUART0_TX_BUFFER_SIZE];
extern const lpuart_transfer_t LPUART0_txTransfer;

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/

void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
