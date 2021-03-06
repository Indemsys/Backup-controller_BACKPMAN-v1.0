/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*! @name PORTA2 (number 73), J15[A8]/U4[6]/LPI2C0_SDA
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_I2C_SDA_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_I2C_SDA_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_I2C_SDA_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*! @name PORTA3 (number 72), J15[A7]/U4[4]/LPI2C0_SCL
  @{ */

/* Symbols to be used with PORT driver */
#define BOARD_ACCEL_I2C_SCL_PORT PORTA               /*!<@brief PORT peripheral base pointer */
#define BOARD_ACCEL_I2C_SCL_PIN 3U                   /*!<@brief PORT pin number */
#define BOARD_ACCEL_I2C_SCL_PIN_MASK (1U << 3U)      /*!<@brief PORT pin mask */
                                                     /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
