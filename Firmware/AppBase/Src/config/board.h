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

#ifndef BOARD_H
#define BOARD_H

/************************************************************************
 * Include Files
 ************************************************************************/
#include "BoardConfig.h"

/************************************************************************
 * Defines
 ************************************************************************/
#define SPIM_SCLK       14u
#define SPIM_MOSI       13u
#define SPIM_MISO       8u

#define INT_CC          4u
#define IRQ_CC          3u

#define AS3933_NCS      15u
#define AS3933_DAT      17u
#define AS3933_CL_DAT   16u
#define AS3933_WAKE     18u

#define LIS2_INT        7u
#define ST25DV_INT      19u
#define LSM6_INT1       6u
#define LSM6_INT2       5u

#define VSEL            30u
#define VIN_MON         2u   

#define SCL_PIN         11u
#define SDA_PIN         12u

#define SKY66112_CTX    22u
#define SKY66112_CRX    23u
#define SKY66112_CPS    20u
#define SKY66112_CHL    21u

/************************************************************************
 * Type definitions
 ************************************************************************/

/************************************************************************
 * Public function declarations
 ************************************************************************/
 
#endif /* BOARD_H */

