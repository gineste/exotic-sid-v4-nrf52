/* 
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 * Date:          10/10/2017 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Sub State Machine Module
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
/* Proprietary includes */
#include <stdint.h>
#include <string.h>

/* Config includes */
#include "board.h"
#include "BoardConfig.h"

/* HAL includes */
#include "HAL/HAL_GPIO.h"
#include "HAL/HAL_RTC.h"
#include "HAL/HAL_I2C.h"
#include "HAL/HAL_SPI.h"

/* BLE includes */
#include "BLE/BLE_Application.h"

/* Libraries includes */

/* Application includes */
#include "PA_LNA.h"
#include "CC430_Interface.h"

/* Self include */
#include "PowerUpSM.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
 
/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief At Power up of board, initialize required modules/registers of µC that are needed
 *        in order to launch others code functions.
 * @return None
 */
void vPowerUp_Process(void)
{
   /* Since it's the first process we run, there is no entry point for PowerUp SM */
   /* Module : SPI, I2C, etc. */
   vHal_I2C_Init();
   vHal_SPI_Init();
   
   /* RTC */
   vHal_RTC_Init();   
   
   /* UART/CC430 */
   vCC430_ITF_Init();
   
   /* Stack BLE */
   vBLE_Init();
   
   /* PA/LNA */   
   vPA_LNA_Init(SKY66112_CTX, SKY66112_CRX);   
}

/**@brief Check point of PowerUp State.
 * @return None
 */
void vPowerUp_Check(void)
{
   /* Just go in Init State */
   vMSM_StateMachineSet(MSM_INIT);
}
/****************************************************************************************
 * Private functions
 ****************************************************************************************/

/****************************************************************************************
 * End Of File
 ****************************************************************************************/
 

