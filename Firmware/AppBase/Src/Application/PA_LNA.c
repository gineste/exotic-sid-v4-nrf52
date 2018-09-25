/* 
 ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
(  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \
(____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2017 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * Date:          02/02/2018 (dd MM YYYY)
 * Author:        Nordic
 * Description:   PA LNA 
 *
 */
 
/******************************************************************************
 * Include Files
 ******************************************************************************/
/* Proprietary includes */
#include <stdint.h>
#include <string.h>

#include "ble.h"
#include "app_error.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_ppi.h"

/* Self Include */
#include "PA_LNA.h"

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
 
void vPA_LNA_Init(uint8_t p_u8Pa_Pin, uint8_t p_u8LNA_Pin)
{
   ble_opt_t l_sBleOpt;
   uint32_t l_u32GPIOTE_Ch = NULL;
   ret_code_t l_u32ErrCode;        

   nrf_ppi_channel_t l_sPPI_SetCh;
   nrf_ppi_channel_t l_sPPI_ClrCh;
   
   nrf_drv_gpiote_out_config_t l_sGPIOTEConfig = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false);
   
   memset(&l_sBleOpt, 0, sizeof(ble_opt_t));

   l_u32ErrCode = nrf_drv_gpiote_init();
   if(l_u32ErrCode != NRF_ERROR_INVALID_STATE)
   {
      APP_ERROR_CHECK(l_u32ErrCode);
   }

   l_u32ErrCode = nrf_drv_ppi_init();
   if(l_u32ErrCode != NRF_ERROR_MODULE_ALREADY_INITIALIZED)
   {
      APP_ERROR_CHECK(l_u32ErrCode);
   }   

   l_u32ErrCode = nrf_drv_ppi_channel_alloc(&l_sPPI_SetCh);
   APP_ERROR_CHECK(l_u32ErrCode);

   l_u32ErrCode = nrf_drv_ppi_channel_alloc(&l_sPPI_ClrCh);
   APP_ERROR_CHECK(l_u32ErrCode);
    
   if((p_u8Pa_Pin == NULL) && (p_u8LNA_Pin == NULL))
   {
      l_u32ErrCode = NRF_ERROR_INVALID_PARAM;
      APP_ERROR_CHECK(l_u32ErrCode);
   }    

   if(p_u8Pa_Pin != NULL)
   {
      if(l_u32GPIOTE_Ch == NULL)
      {
         l_u32ErrCode = nrf_drv_gpiote_out_init((uint32_t)p_u8Pa_Pin, &l_sGPIOTEConfig);
         APP_ERROR_CHECK(l_u32ErrCode);

         l_u32GPIOTE_Ch = nrf_drv_gpiote_out_task_addr_get((uint32_t)p_u8Pa_Pin); 
      }

      // PA config
      l_sBleOpt.common_opt.pa_lna.pa_cfg.active_high = 1;   // Set the pin to be active high
      l_sBleOpt.common_opt.pa_lna.pa_cfg.enable      = 1;   // Enable toggling
      l_sBleOpt.common_opt.pa_lna.pa_cfg.gpio_pin    = p_u8Pa_Pin; // The GPIO pin to toggle tx  
   }

   if(p_u8LNA_Pin != NULL)
   {
      if(l_u32GPIOTE_Ch == NULL)
      {
         l_u32ErrCode = nrf_drv_gpiote_out_init((uint32_t)p_u8LNA_Pin, &l_sGPIOTEConfig);
         APP_ERROR_CHECK(l_u32ErrCode);        

         l_u32GPIOTE_Ch = nrf_drv_gpiote_out_task_addr_get((uint32_t)p_u8LNA_Pin); 
      }

      // LNA config
      l_sBleOpt.common_opt.pa_lna.lna_cfg.active_high  = 1; // Set the pin to be active high
      l_sBleOpt.common_opt.pa_lna.lna_cfg.enable       = 1; // Enable toggling
      l_sBleOpt.common_opt.pa_lna.lna_cfg.gpio_pin     = p_u8LNA_Pin;  // The GPIO pin to toggle rx
   }

   // Common PA/LNA config
   l_sBleOpt.common_opt.pa_lna.gpiote_ch_id  = (l_u32GPIOTE_Ch - NRF_GPIOTE_BASE) >> 2;   // GPIOTE channel used for radio pin toggling
   l_sBleOpt.common_opt.pa_lna.ppi_ch_id_clr = l_sPPI_ClrCh;   // PPI channel used for radio pin clearing
   l_sBleOpt.common_opt.pa_lna.ppi_ch_id_set = l_sPPI_SetCh;   // PPI channel used for radio pin setting

   l_u32ErrCode = sd_ble_opt_set(BLE_COMMON_OPT_PA_LNA, &l_sBleOpt);
   APP_ERROR_CHECK(l_u32ErrCode);    
}

/************************************************************************
 * Private functions
 ************************************************************************/

/************************************************************************
 * End Of File
 ************************************************************************/

