/* 
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( ( oO )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 */
#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>\n
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
 
/****************************************************************************************
 * Defines
 ****************************************************************************************/
// <h> Sensors Configuration
//==========================================================
// <q> BME280 Present
// <i> The Bosch BME280 sensor measures temperature, pressure and humidity data.
#ifndef EN_BME280
#define EN_BME280 1
#endif
// <q> LSM6DSL Present
// <i> The LSM6DSL sensor measures acceleration and gyroscope data on x, y and z axis.
#ifndef EN_LSM6DSL
#define EN_LSM6DSL 0
#define LSM6DSL_SENSOR_NB  2u
#endif
// <q> LIS2MDL Present
// <i> The LSM6DSL sensor measures acceleration and gyroscope data on x, y and z axis.
#ifndef EN_LIS2MDL
#define EN_LIS2MDL 1
#endif
// <q> Battery Measurement
#ifndef EN_BATT
#define EN_BATT 0
#endif

#ifndef SENSOR_NUMBER
#define SENSOR_NUMBER ( (EN_LSM6DSL*LSM6DSL_SENSOR_NB)      +  \
                        EN_LIS2MDL                          +  \
                        EN_BME280                           +  \
                        EN_BATT )
#endif

// </h>

//==========================================================
// <h> BLE Configuration
//==========================================================
// <e.0> BLE Enabled
// <i> ENABLE_BLE
#ifndef ENABLE_BLE
#define ENABLE_BLE 1
#endif
// <o> Fast Advertise Interval - mseconds <10-1000:5>
#ifndef FAST_ADV_INTERVAL_BLE
#define FAST_ADV_INTERVAL_BLE 500
#endif
#define FAST_ADV_INT_BLE_COMP (FAST_ADV_INTERVAL_BLE<<3)/5
// <o> Fast Advertise Time out - seconds <0-600:5>
// <i> Timeout between 0 sec(unlimited) and 600 sec with step of 5 sec - FAST_ADV_TIMEOUT_BLE
#ifndef FAST_ADV_TIMEOUT_BLE
#define FAST_ADV_TIMEOUT_BLE 60
#endif
// <o> Slow Advertise Interval - mseconds <1000-30000:5>
#ifndef SLOW_ADV_INTERVAL_BLE
#define SLOW_ADV_INTERVAL_BLE 1000
#endif
#define SLOW_ADV_INT_BLE_COMP (SLOW_ADV_INTERVAL_BLE<<3)/5
// <o> Slow Advertise Time out - seconds <0-3600:5>
// <i> Timeout between  0 sec(unlimited) and 1h with step of 5 sec - SLOW_ADV_TIMEOUT_BLE
#ifndef SLOW_ADV_TIMEOUT_BLE
#define SLOW_ADV_TIMEOUT_BLE 0
#endif
// <o> WakeUp BLE after end of advertising Time out - seconds <00-600:10>
// <i> Timeout between 0 sec(unlimited) and 10 min with step of 10 sec - WAKE_ADV_TIMEOUT_BLE
#ifndef WAKE_ADV_TIMEOUT_BLE
#define WAKE_ADV_TIMEOUT_BLE 0
#endif
#define WAKE_ADV_TIMEOUT_BLE_MS (uint32_t)(WAKE_ADV_TIMEOUT_BLE * 1000u)
// <s> Default Prefix BLE Name
// <i> Always start with PREFIX "SF_" - DEFAULT_PREFIX_BLE_DEVICE_NAME
#ifndef DEFAULT_PREFIX_BLE_DEVICE_NAME
#define DEFAULT_PREFIX_BLE_DEVICE_NAME "SF_"
#endif
// <s> Default BLE Name
// <i> DEFAULT_BLE_DEVICE_NAME
#ifndef DEFAULT_BLE_DEVICE_NAME
#define DEFAULT_BLE_DEVICE_NAME "SIDv4"
#endif
// </e> BLE DISABLED
// </h> 
//==========================================================

// <h> Modules Configuration
//==========================================================
// <o> Board Type
// <i> Select board type for specific advertise/Ext update (Baliz, Graal, SID)
// <0=> None 
// <1=> Baliz
// <2=> Graal
// <3=> SID
// <4=> other
#ifndef BOARD_TYPE
#define BOARD_TYPE 3
#endif
// <o> Use Case number <0-255:1>
#ifndef USE_CASE_NUMBER
#define USE_CASE_NUMBER 255
#endif
// <q> Enable NFC
// <i> Check it if you want to enable NFC functionnality - EN_NFC
#ifndef EN_NFC
#define EN_NFC 0
#endif
// <q> Enable RFID
// <i> Check it if you want to enable RFID functionnality - EN_RFID
#ifndef EN_RFID
#define EN_RFID 0
#endif

// </h> 
//==========================================================

// <h> Debug Configuration
//==========================================================
// <e> Enable LOG
// <i> EN_LOG
#ifndef EN_LOG
#define EN_LOG 1
#endif
// <q> Log BLE Transmit
// <i> LOG_BLE_TX
#ifndef LOG_BLE_TX
#define LOG_BLE_TX 1
#endif
// <q> Log BLE Received
// <i> LOG_BLE_RX
#ifndef LOG_BLE_RX
#define LOG_BLE_RX 1
#endif
// <q> Log CC430
// <i> LOG_CC430
#ifndef LOG_CC430
#define LOG_CC430 1
#endif
// </e>

// <q> WakeUp On Motion
// <i> EN_WAKEUP_ON_MOTION
#ifndef EN_WAKEUP_ON_MOTION
#define EN_WAKEUP_ON_MOTION 0
#endif

// <e> Enable WDT
// <i> EN_WDT_PROTECTION
#ifndef EN_WDT_PROTECTION
#define EN_WDT_PROTECTION 0
#endif
// <o> Watchdog Timeout - msec <15-4294967295:1>
// <i> Timeout between 0 sec(unlimited) and 10 min with step of 10 sec - WDT_TIMEOUT
#ifndef WDT_TIMEOUT
#define WDT_TIMEOUT 2000
#endif
// </e>

// <q> CC430 not soldered
// <i> NO_CC430_ON_BOARD
#ifndef NO_CC430_ON_BOARD
#define NO_CC430_ON_BOARD 1
#endif

// <q> Disable Frame Check
// <i> NO_FRAME_CHECK
#ifndef NO_FRAME_CHECK
#define NO_FRAME_CHECK 0
#endif

// <q> CC430 update when connected
// <i> CC430_STREAM_DATA
#ifndef CC430_STREAM_DATA
#define CC430_STREAM_DATA 0
#endif

// <q> Automatic Stream BLE Data once connected
// <i> AUTOMATIC_STREAM_DATA
#ifndef AUTOMATIC_STREAM_DATA
#define AUTOMATIC_STREAM_DATA 0
#endif

// </h> 
//==========================================================

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
 
// <<< end of configuration section >>>
#endif /* BOARDCONFIG_H */

