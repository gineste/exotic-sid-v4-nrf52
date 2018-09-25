/* 
 *  ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 * (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *  ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
 * (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef CUSTOM_BOARD_H
#define CUSTOM_BOARD_H

/************************************************************************
 * Include Files
 ************************************************************************/

/************************************************************************
 * Defines
 ************************************************************************/
#define UART_TX         6u
#define UART_RX         5u

#define SCL_PIN         11u
#define SDA_PIN         12u

#define SPIM_SCLK       28u
#define SPIM_MOSI       27u
#define SPIM_MISO       26u

#define ADXL_NCS        25u
#define ADXL_INT1       30u   /* INT1 is named INT2 on schematics... */
#define ADXL_INT2       2u    /* INT2 is named INT1 on schematics... */

#define AD7150_INT1     8u
#define AD7150_INT2     7u

#define AS3933_NCS      31u
#define AS3933_DAT      3u
#define AS3933_WAKE     4u

#define NFC1				10u
#define NFC2			   9u

#define SKY66112_CTX    22u
#define SKY66112_CRX    23u
#define SKY66112_CPS    20u
#define SKY66112_CHL    21u

   
#define BUTTONS_NUMBER 0

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1 }
 
#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_NUMBER    0
                                 
/************************************************************************
 * Type definitions
 ************************************************************************/

/************************************************************************
 * Public function declarations
 ************************************************************************/
 
#endif /* CUSTOM_BOARD_H */

