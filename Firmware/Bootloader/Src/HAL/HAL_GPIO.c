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
 * File name:		Hal_GPIO.c
 * Date:			   27 06 2017(dd MM YYYY)
 * Author:			Yoann Rebischung
 * Description:	Hardware Abstraction Layer for GPIO configuration
 *
 */
 
/************************************************************************
 * Include Files
 ************************************************************************/
#include <nrf_delay.h>
 
#include "boards.h"

#include "HAL_GPIO.h"

/************************************************************************
 * Defines
 ************************************************************************/

/************************************************************************
 * Private type declarations
 ************************************************************************/

/************************************************************************
 * Private function declarations
 ************************************************************************/

/************************************************************************
 * Variable declarations
 ************************************************************************/

/************************************************************************
 * Public functions
 ************************************************************************/  
/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT pin for output, 
 * and configures GPIOTE to give an interrupt on pin change.
 */
void vHal_GPIO_Init(void)
{   
   /* SPI */
	vGPIO_OutputCfg(SPIM_SCLK, HALGPIO_PIN_NOPULL); 
   vHal_GPIO_Set(SPIM_SCLK);   
   vGPIO_OutputCfg(ADXL_NCS, HALGPIO_PIN_PULLUP); 
   vHal_GPIO_Set(ADXL_NCS);   

   /* Interrupt */
   vHal_GPIO_Cfg(ADXL_INT1, HALGPIO_PINDIR_INPUT, HALGPIO_PININ_CONNECT,
            HALGPIO_PIN_NOPULL, HALGPIO_PINDRV_D0S1, HALGPIO_PIN_SENSEHIGH);
   vHal_GPIO_Cfg(ADXL_INT2, HALGPIO_PINDIR_INPUT, HALGPIO_PININ_CONNECT,
            HALGPIO_PIN_NOPULL, HALGPIO_PINDRV_D0S1, HALGPIO_PIN_SENSEHIGH);
   
   /* UART */
   vGPIO_InputCfg(UART_RX, HALGPIO_PIN_NOPULL);
   vGPIO_InputCfg(UART_TX, HALGPIO_PIN_NOPULL);
  // vHal_GPIO_Clear(UART_TX);   
	
   /* TWI */
   vGPIO_InputCfg(SDA_PIN, HALGPIO_PIN_PULLUP);
   vGPIO_InputCfg(SCL_PIN, HALGPIO_PIN_PULLUP); 
   
   /* LF125K - AS3933 */
   vGPIO_OutputCfg(AS3933_NCS, HALGPIO_PIN_PULLDOWN); 
   vHal_GPIO_Clear(AS3933_NCS);   
   vGPIO_InputCfg(AS3933_WAKE, HALGPIO_PIN_NOPULL);
   vGPIO_InputCfg(AS3933_DAT, HALGPIO_PIN_NOPULL);
   
   /* SKY66112 */
   vGPIO_OutputCfg(SKY66112_CTX, HALGPIO_PIN_NOPULL);
   vGPIO_OutputCfg(SKY66112_CRX, HALGPIO_PIN_NOPULL);
   vGPIO_OutputCfg(SKY66112_CPS, HALGPIO_PIN_NOPULL);
   vHal_GPIO_Clear(SKY66112_CPS);   
   vGPIO_OutputCfg(SKY66112_CHL, HALGPIO_PIN_NOPULL);
   vHal_GPIO_Set(SKY66112_CHL);   
}

void vHal_GPIO_Cfg(uint32_t p_u32Pin, e_HalGPIO_PinDir_t p_ePinDir, e_HalGPIO_PinInConn_t p_ePinInConn,
               e_HalGPIO_PinPull_t p_ePinPull, e_HalGPIO_PinDrive_t p_ePinDrive, e_HalGPIO_PinSense_t p_ePinSense)
{
   if(p_ePinDir == HALGPIO_PINDIR_OUTPUT)
   {
      p_ePinInConn = HALGPIO_PININ_DISCONNECT;
   }
   
   nrf_gpio_cfg(p_u32Pin,(nrf_gpio_pin_dir_t)p_ePinDir,(nrf_gpio_pin_input_t)p_ePinInConn,
        (nrf_gpio_pin_pull_t)p_ePinPull,(nrf_gpio_pin_drive_t)p_ePinDrive,(nrf_gpio_pin_sense_t)p_ePinSense);
   
}
/************************************************************************
 * Private functions
 ************************************************************************/

/************************************************************************
 * End of File
 ************************************************************************/

