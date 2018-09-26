/* 
 *    ____  _  _   __   ____   __    ___    ____  _  _  ____  ____  ____  _  _  ____
 *   (  __)( \/ ) /  \ (_  _) (  )  / __)  / ___)( \/ )/ ___)(_  _)(  __)( \/ )/ ___)
 *    ) _)  )  ( (  O )  )(    )(  ( (__   \___ \ )  / \___ \  )(   ) _) / \/ \\___ \     
 *   (____)(_/\_) \__/  (__)  (__)  \___)  (____/(__/  (____/ (__) (____)\_)(_/(____/
 *
 * Copyright (c) 2018 EXOTIC SYSTEMS. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO WARRANTY 
 * of ANY KIND is provided. This heading must NOT be removed from the file.
 *
 * Date:          06/03/2018 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Battery voltage ADC. 
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>
#include <nrf52.h>

#include "nrf_drv_saadc.h"

#include "GlobalDefs.h"

/* Self include */
#include "Battery.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define BUFFER_NUMBER         (uint8_t)1u
#define SAMPLES_IN_BUFFER     (uint8_t)1u
#define ADC_10BITS_RANGE      (float)1023.0f
#define RANGE_MAX_MV          (float)3600.0f // For Gain 1/6
#define ADC_CHANNEL_IDX       (uint8_t)0u
#define VOLTAGE_RATIO         (float)2.0f
#define MODULO_READ           (uint8_t)5u

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/

/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/
static void vSAADC_Callback(nrf_drv_saadc_evt_t const * p_pcsEvent);

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
static nrf_drv_saadc_config_t g_sSAADC_Cfg = NRF_DRV_SAADC_DEFAULT_CONFIG;

static const nrf_saadc_channel_config_t g_sChannel_Cfg = {
   .reference = NRF_SAADC_REFERENCE_INTERNAL,    //Set internal reference of fixed 0.6 volts
   .gain = NRF_SAADC_GAIN1_6,                    //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
   .acq_time = NRF_SAADC_ACQTIME_40US,           //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS. 
   .mode = NRF_SAADC_MODE_SINGLE_ENDED,          //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
   .pin_p = NRF_SAADC_INPUT_AIN0,                 //Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
   .pin_n = NRF_SAADC_INPUT_DISABLED,            //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
   .resistor_p = NRF_SAADC_RESISTOR_DISABLED,    //Disable pullup resistor on the input pin
   .resistor_n = NRF_SAADC_RESISTOR_DISABLED,    //Disable pulldown resistor on the input pin
   .burst = NRF_SAADC_BURST_ENABLED,             //Enable Burst mode
};//NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);

//static nrf_saadc_value_t g_sSAADCValue[BUFFER_NUMBER][SAMPLES_IN_BUFFER];
#ifndef DEBUG
static
#endif
float g_fBattery_mV = 3000.0f;
#ifndef DEBUG
static
#endif
float g_fBatteryAvg_mV = 3000.0f;
uint16_t g_u16BattCnt = 0u;
static uint8_t g_u8ModuloCnt = MODULO_READ;
static uint8_t g_u8TimeToRead = 0u;
/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
void vBattery_Init(void)
{ 
   uint32_t l_u32ErrCode = 0u;

   /* Initialize the SAADC with configuration and callback function. 
    * The application must then implement the saadc_callback function, which will be called when SAADC interrupt is triggered */
   l_u32ErrCode = nrf_drv_saadc_init(&g_sSAADC_Cfg, vSAADC_Callback);
   APP_ERROR_CHECK(l_u32ErrCode);
   
   /* Initialize SAADC channel 0 with the channel configuration */
   l_u32ErrCode = nrf_drv_saadc_channel_init(ADC_CHANNEL_IDX, &g_sChannel_Cfg);                            
   APP_ERROR_CHECK(l_u32ErrCode);
   
   /* Set SAADC buffer. The SAADC will start to write to this buffer */
//      l_u32ErrCode = nrf_drv_saadc_buffer_convert(g_sSAADCValue[0u], SAMPLES_IN_BUFFER);    
//      APP_ERROR_CHECK(l_u32ErrCode); 
      
//      l_u32ErrCode = nrf_drv_saadc_buffer_convert(g_sSAADCValue[1u], SAMPLES_IN_BUFFER);    
//      APP_ERROR_CHECK(l_u32ErrCode);       
      
   /* ISSUE SAADC */
//      nrf_drv_saadc_abort();
//      while(NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy << SAADC_STATUS_STATUS_Pos));
   
   /* Enable SAADC END interrupt to do maintainance and printing of values. */
//      NRF_SAADC->INTENSET = SAADC_INTENSET_END_Enabled << SAADC_INTENSET_END_Pos;
//      NVIC_EnableIRQ(SAADC_IRQn);
}

void vBattery_ShutDown(void)
{
   nrf_drv_saadc_uninit();
}

void vBattery_Read(uint8_t * p_pau8Data, uint8_t * p_pu8Size)
{
   uint8_t l_u8Size = 0u;
   uint32_t l_u32ErrCode = 0u;
      
   g_u8TimeToRead = ((g_u8ModuloCnt % MODULO_READ) == 0u)?1u:0u;
   
   if(g_u8TimeToRead == 1u)
   {
      l_u32ErrCode = nrf_drv_saadc_sample_convert(ADC_CHANNEL_IDX, (nrf_saadc_value_t*)&g_u16BattCnt );
      APP_ERROR_CHECK(l_u32ErrCode); 
      
      g_fBattery_mV = ((((float)g_u16BattCnt) * VOLTAGE_RATIO) / ADC_10BITS_RANGE) * RANGE_MAX_MV;
      
      g_fBatteryAvg_mV += g_fBattery_mV;
      g_fBatteryAvg_mV = g_fBatteryAvg_mV / 2.0f;
      g_u8ModuloCnt = 1u;
   }
   else
   {
      g_u8ModuloCnt++;
   }
      
   p_pau8Data[l_u8Size++] = XMSB_32B_TO_8B(g_fBattery_mV);
   p_pau8Data[l_u8Size++] =  MSB_32B_TO_8B(g_fBattery_mV);
   p_pau8Data[l_u8Size++] =  LSB_32B_TO_8B(g_fBattery_mV);
   p_pau8Data[l_u8Size++] = XLSB_32B_TO_8B(g_fBattery_mV);
}

uint16_t u16Battery_VoltagemVGet(void)
{
   return (uint16_t)g_fBattery_mV;
}
/****************************************************************************************
 * Private functions
 ****************************************************************************************/
static void vSAADC_Callback(nrf_drv_saadc_evt_t const * p_pcsEvent)
{
   uint32_t l_u32ErrCode;      
   
   switch(p_pcsEvent->type)
   {
      case NRF_DRV_SAADC_EVT_CALIBRATEDONE:
         __nop();
         break;
      case NRF_DRV_SAADC_EVT_LIMIT:
         __nop();
         break;
      case NRF_DRV_SAADC_EVT_DONE:      
         //l_u32ErrCode = nrf_drv_saadc_sample_convert(ADC_CHANNEL_IDX, (nrf_saadc_value_t*)&g_u16BattCnt );
         l_u32ErrCode = nrf_drv_saadc_buffer_convert(p_pcsEvent->data.done.p_buffer, SAMPLES_IN_BUFFER);
         APP_ERROR_CHECK(l_u32ErrCode);            
                
//         nrf_drv_saadc_uninit();
//         NRF_SAADC->INTENCLR = (SAADC_INTENCLR_END_Clear << SAADC_INTENCLR_END_Pos);
//         NVIC_ClearPendingIRQ(SAADC_IRQn);
         g_fBattery_mV = ((float)p_pcsEvent->data.done.p_buffer[0u] / 1023.0f) * RANGE_MAX_MV;    
         break;
      default:
         break;
   }
}

/****************************************************************************************
 * End Of File
 ****************************************************************************************/


