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
 * Date:          09 08 2017 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Sensor Manager file to get data value from Sensors. 
 *
 */
 
/************************************************************************
 * Include Files
 ************************************************************************/
#include <stdint.h>
#include <string.h>
#include <nrf_delay.h>
#include "app_util.h"

#include "BoardConfig.h"
#include "GlobalDefs.h"

/* HAL include */
#include "HAL/HAL_SPI.h"
#include "HAL/HAL_I2C.h"
#include "HAL/HAL_GPIO.h"
#include "HAL/HAL_Timer.h"

/* Sensor Drivers include */
#if (EN_LSM6MDL == 1)
   #include "LSM6DSL/LSM6DSL.h"
#endif
#if (EN_LIS2MDL == 1)
   #include "LIS2MDL/LIS2MDL.h"
#endif
#if (EN_BME280 == 1)
   #include "BME280/BME280.h"
#endif
#if (EN_BATT == 1)
   #include "Battery/Battery.h"
#endif
#if (EN_RFID == 1)
   #include "AS3933/AS3933.h"
#endif

/* Application include */
#include "ES_Commands.h"
#include "MainStateMachine.h"
#include "ModeManagement.h"


/* Self include */
#include "SensorManager.h"

/************************************************************************
 * Defines
 ************************************************************************/
#define SENSOR_MNGR_NUMBERS      SENSOR_NUMBER+1u

/************************************************************************
 * Private type declarations
 ************************************************************************/
typedef void (*fpvSensor_t)(void);
typedef void (*fpvSensorGet_t)(uint8_t * p_pau8Data, uint8_t * p_pu8Size);

typedef struct _SENSOR_MNGNT_ {
   fpvSensor_t    fpvSensorsShutdown;
   fpvSensor_t    fpvSensorsWakeUp;
   fpvSensorGet_t fpvSensorsDataGet;
}s_SensorManagement_t;

/************************************************************************
 * Private function declarations
 ************************************************************************/
static void vInitnSleepAllSensors(void);

#if (EN_BME280 == 1)
   static void vTPHStop(void);
   static void vTPHActivate(void);
   static void vTPHGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size);
#endif

#if (EN_LSM6DSL == 1)
   static void vAccelShutdown(void);
   static void vAccelWakeUp(void);
   static void vAccelGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size);
   
   static void vGyroShutdown(void);
   static void vGyroWakeUp(void);
   static void vGyroGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size);
#endif

#if (EN_LIS2MDL == 1)
   static void vMagShutdown(void);
   static void vMagWakeUp(void);
   static void vMagGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size);
#endif

#if (EN_BATT == 1)
   static void vBatteryStop(void);
   static void vBatteryActivate(void);
   static void vBatteryGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size);
#endif

/************************************************************************
 * Variable declarations
 ************************************************************************/
#if (EN_BME280 == 1)
   static s_BME280_Context_t g_sBME280Context = {
      .eInterface = BME280_I2C_ITF,
      .sI2CCfg = {
         .eI2CAddr = BME280_I2C_ADDR_GND,
         .fp_u32I2C_Write = &u32Hal_I2C_Write,
         .fp_u32I2C_Read = &u32Hal_I2C_WriteAndRead
      },
      .fp_vTimerDelay_ms = &vHal_Timer_DelayMs
   };
#endif /* EN_BME280 */
   
#if (EN_LSM6DSL == 1)
static s_LSM6DSL_Context_t g_sLSM6DSLContext = {
   .u8I2CAddress = LSM6DSL_SLAVE_ADDR_VCC,                  /* Sensor Address */
   /* Function pointer to a read I2C transfer */
   .fp_u32I2C_Read = &u32Hal_I2C_WriteAndReadNoStop,
   /* Function pointer to a write I2C transfer */
   .fp_u32I2C_Write = &u32Hal_I2C_Write,
   /* Function pointer to a timer in ms */
   .fp_vDelay_ms = &vHal_Timer_DelayMs,
   
   .eAccelFullScale = LSM6DSL_ACCEL_RANGE_8G,
   .eAccelODR = LSM6DSL_ODR_104Hz,
   .eGyroFullScale = LSM6DSL_GYRO_RANGE_500DPS,
   .eGyroODR = LSM6DSL_ODR_52Hz
   };
#endif

#if (EN_LIS2MDL == 1)
static s_LIS2MDL_Context_t g_sLIS2MDLContext = {
   .eCommunicationUsed = LIS2MDL_COMM_I2C,
   .sI2CCfg = {
      /* Function pointer to a read I2C transfer */
      .fp_u32I2C_Write = &u32Hal_I2C_Write,
      /* Function pointer to a write I2C transfer */
      .fp_u32I2C_Read = &u32Hal_I2C_WriteAndReadNoStop
   },
   /* Function pointer to a timer in ms */
   .fp_vDelay_ms = &vHal_Timer_DelayMs,
   };
#endif

#
static const s_SensorManagement_t g_cafpvSensorsMngt[SENSOR_MNGR_NUMBERS] = {
   /*Shutdown,                         WakeUp,                       DataUpdate */
   #if (EN_BME280 == 1)
      {vTPHStop,                          vTPHActivate,                 vTPHGet},                           /* TPH */
   #endif
   #if (EN_LSM6DSL == 1)
      {vAccelShutdown,                    vAccelWakeUp,                 vAccelGet},                         /* LSM6DSL Accel */
      {vGyroShutdown,                     vGyroWakeUp,                  vGyroGet},                          /* LSM6DSL Gyro */
   #endif
   #if (EN_LIS2MDL == 1)
      {vMagShutdown,                      vMagWakeUp,                   vMagGet},                           /* LIS2MDL */
   #endif
   #if (EN_BATT == 1)
      {vBatteryStop,                      vBatteryActivate,             vBatteryGet}                        /* Battery Value */
   #endif
   /* NO SENSORS */
   #if (SENSOR_MNGR_NUMBERS == 1)
      #warning "No Sensor activated !"
      {NULL, NULL, NULL}
   #endif
};

#ifndef DEBUG
static
#endif
s_SensorsData_t g_sSensorsData = { 0 };

uint16_t g_u16DbgWakeUpCnt = 0U;
uint16_t g_u16DbgShutdownCnt = 0U;

/************************************************************************
 * Public functions
 ************************************************************************/  
/**@brief   Function to Initialize sensor manager with the sensor from Configuration.
 * @return  None
 */
void vSensorMngr_Init(void)
{
   /* Init and put sensors in sleep */
   vInitnSleepAllSensors();

#if (EN_BATT == 1)
//   vBattery_Init();
#endif
}

/**@brief   Function to Shutdown all sensors.
 * @return  None
 */
void vSensorMngr_ShutDownAll(void)
{
   
}

/**@brief      Function to Shutdown specific sensors.
 * @param[in]  p_u32ActivatedSensors-
 * @return     None
 */
void vSensorMngr_ShutdownSensor(void)
{
   uint8_t l_u8Idx = 0u;
   g_u16DbgShutdownCnt++;
   for(l_u8Idx = 0u;l_u8Idx < SENSOR_MNGR_NUMBERS; l_u8Idx++)
   {
      if(g_cafpvSensorsMngt[l_u8Idx].fpvSensorsShutdown != NULL)
      {
         (*g_cafpvSensorsMngt[l_u8Idx].fpvSensorsShutdown)();
      }
   }
}

/**@brief      Function to Update current Sensor Configuration.
 * @param[in]  p_u32SensorCfg
 * @return     None
 */
void vSensorMngr_UpdateSensorCfg(void)
{
   
}

/**@brief      Function to WakeUp specific sensors.
 * @param[in]  p_u32ActivatedSensors
 * @return     None
 */
void vSensorMngr_WakeUpSensor(void)
{
   uint8_t l_u8Idx = 0u;
   g_u16DbgWakeUpCnt++;
   for(l_u8Idx = 0u;l_u8Idx < SENSOR_MNGR_NUMBERS; l_u8Idx++)
   {
      if((*g_cafpvSensorsMngt[l_u8Idx].fpvSensorsWakeUp) != NULL)
      {
         (*g_cafpvSensorsMngt[l_u8Idx].fpvSensorsWakeUp)();            
      } 
   }
}

/**@brief      Background function to Update Sensors.
 * @return     None
 */
void vSensorMngr_DataUpdate(void)
{
   uint8_t l_u8Idx = 0u;
   
   for(l_u8Idx = 0u;l_u8Idx < SENSOR_MNGR_NUMBERS; l_u8Idx++)
   {
      if((*g_cafpvSensorsMngt[l_u8Idx].fpvSensorsDataGet) != NULL)
      {
         (*g_cafpvSensorsMngt[l_u8Idx].fpvSensorsDataGet)(NULL, NULL);            
      } 
   }   
}

/**@brief      Get Sensor Data Structure.
 * @param[out] p_ppsSensorsData : return the address of Sensor Data Structure
 * @return     None
 */
void vSensorMngr_DataGet(s_SensorsData_t ** p_ppsSensorsData)
{
   (*p_ppsSensorsData) = &g_sSensorsData;
}

/**@brief   Function to get sensor data from Temperature sensor in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return Temperature MSB first
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_TemperatureGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{
   uint8_t l_u8Size = 0u;
   
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16Temperature);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16Temperature);
   
   (*p_pu8DataSize) = l_u8Size;
}
/**@brief   Function to get sensor data from Pressure sensor in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return Pressure MSB first
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_PressureGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{
   uint8_t l_u8Size = 0u;
   
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.u16Pressure);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.u16Pressure);
   
   (*p_pu8DataSize) = l_u8Size;
}
/**@brief   Function to get sensor data from humidity sensor in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return humidity 
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_HumidityGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{ 
   uint8_t l_u8Size = 0u;
   
   p_pu8Data[l_u8Size++] = g_sSensorsData.u8Humidity;
   
   (*p_pu8DataSize) = l_u8Size;
}
/**@brief   Function to get sensor data from Accelerometer sensor in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return Accelerometer (X,Y,Z) MSB first
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_AccelerometerGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{
   uint8_t l_u8Size = 0u;
   
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16AccelX);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16AccelX);
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16AccelY);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16AccelY);
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16AccelZ);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16AccelZ);
   
   (*p_pu8DataSize) = l_u8Size;
}

/**@brief   Function to get sensor data from Gryoscope sensor in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return Gyrometer (X,Y,Z) MSB first
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_GyrometerGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{
   uint8_t l_u8Size = 0u;
   
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16GyroX);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16GyroX);
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16GyroY);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16GyroY);
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16GyroZ);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16GyroZ);
   
   (*p_pu8DataSize) = l_u8Size;
}

/**@brief   Function to get sensor data from Magnetometer sensor in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return Magnetometer (X,Y,Z) MSB first
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_MagnetomerGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{   uint8_t l_u8Size = 0u;
   
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16MagX);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16MagX);
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16MagY);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16MagY);
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.s16MagZ);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.s16MagZ);
   
   (*p_pu8DataSize) = l_u8Size;
}


/**@brief   Function to get battery value from LTC2942 in FrameBuilder
 *          typedef.
 * @param[out] p_pu8Data : return Battery value MSB first
 * @param[out] p_pu8DataSize : return size of data
 * @return     None
 */
void vSensorMngr_BatteryGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize)
{
   uint8_t l_u8Size = 0u;
   
	/* Percent 1byte */
   p_pu8Data[l_u8Size++] = g_sSensorsData.u8BatteryPercent;
	
	/* Voltage 2 bytes */
   p_pu8Data[l_u8Size++] = MSB_16B_TO_8B(g_sSensorsData.u16BatteryVoltage);
   p_pu8Data[l_u8Size++] = LSB_16B_TO_8B(g_sSensorsData.u16BatteryVoltage);
	   
   (*p_pu8DataSize) = l_u8Size;
}

/**@brief Check if sensors are dead
 * @return 1 if sensor dead else 0.
 */
uint8_t u8SensorMngr_IsSensorsDead(void)
{
   uint8_t l_u8IsAvailable = 0u;
   uint8_t l_u8SensorNb = 0u;
   
#if (EN_BME280 == 1)
   l_u8IsAvailable += u8BME280_IsAvailable();
   l_u8SensorNb++;
#endif   
   
#if (EN_LSM6DSL == 1)
   l_u8IsAvailable += u8LSM6DSL_IsAvailable();
   l_u8SensorNb++;
#endif
   
#if (EN_LIS2MDL == 1)
   l_u8IsAvailable += u8LIS2MDL_IsAvailable();
   l_u8SensorNb++;
#endif
         
   return (l_u8IsAvailable == l_u8SensorNb)?0u:1u;
}

/************************************************************************
 * Private functions
 ************************************************************************/
/**@brief   Function to Initialize all sensors and put in them in deepsleep mode
 *          if possible.
 * @return  None
 */
static void vInitnSleepAllSensors(void)
{
#if (EN_LSM6DSL == 1)
   if(eLSM6DSL_Initialization(g_sLSM6DSLContext) != LSM6DSL_ERROR_NONE)
   {
      __nop();
   }   
#endif
   
#if (EN_LIS2MDL == 1)
   if(eLIS2MDL_ContextSet(g_sLIS2MDLContext) == LIS2MDL_ERROR_NONE)
   {
      eLIS2MDL_LowPower(1u);
   }      
#endif
   
#if (EN_BME280 == 1)
   eBME280_ContextSet(g_sBME280Context);
   eBME280_IIRFilterSet(BME280_FILTER_COEFF_OFF);
   eBME280_ModeSet(BME280_FORCED);
#endif

#if (EN_RFID == 1)
   vAS3933_SPIContextSet();
   (void)eAS3933_Init();
#endif
}

#if (EN_BME280 == 1)
static void vTPHStop(void)
{
   eBME280_OSRTemperatureSet(BME280_NO_OVERSAMPLING);
   eBME280_OSRPressureSet(BME280_NO_OVERSAMPLING);
   eBME280_OSRHumiditySet(BME280_NO_OVERSAMPLING);
}
static void vTPHActivate(void)
{
   eBME280_OSRTemperatureSet(BME280_OVERSAMPLING_1X);
   eBME280_OSRPressureSet(BME280_OVERSAMPLING_1X);
   eBME280_OSRHumiditySet(BME280_OVERSAMPLING_1X);
}

static void vTPHGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size)
{
   float l_f32Data = 0.0f;
   
   eBME280_TPHRead();
   eBME280_TemperatureGet(&l_f32Data);
   g_sSensorsData.s16Temperature = (int16_t)(l_f32Data * 10.0f);
   eBME280_PressureGet(&l_f32Data);
   g_sSensorsData.u16Pressure = (uint16_t)(l_f32Data * 10.0f);
   eBME280_HumidityGet(&l_f32Data);
   g_sSensorsData.u8Humidity = (uint16_t)(l_f32Data)  ;
}
#endif /* (EN_BME280 == 1) */

#if (EN_LSM6DSL == 1)
/**@brief Function to stop accelerometer acquisition of data from sensor.
 */
static void vAccelShutdown(void)
{
//   vLSM6DSL_AccelEnable(0u);
}
/**@brief Function to activate accelerometer acquisition of data from sensor.
 */
static void vAccelWakeUp(void)
{
//   vLSM6DSL_AccelEnable(1u);
}
/**@brief Function to get accelerometer measurement.
 */
static void vAccelGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size)
{   
   s_LSM6DSL_3AxisRawData_t l_sRawAxis;
   vLSM6DSL_AccelRead(&l_sRawAxis);
   g_sSensorsData.s16AccelX = l_sRawAxis.RawSigned.s16DataX;
   g_sSensorsData.s16AccelY = l_sRawAxis.RawSigned.s16DataY;
   g_sSensorsData.s16AccelZ = l_sRawAxis.RawSigned.s16DataZ;
}

/**@brief Function to stop gyro acquisition of data from sensor.
 */
static void vGyroShutdown(void)
{
//   vLSM6DSL_GyroEnable(0u);
}
/**@brief Function to activate gyro acquisition of data from sensor.
 */
static void vGyroWakeUp(void)
{
//   vLSM6DSL_GyroEnable(1u);
}
/**@brief Function to get Gyro measurement.
 */
static void vGyroGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size)
{
   s_LSM6DSL_3AxisRawData_t l_sRawAxis;
   vLSM6DSL_GyroRead(&l_sRawAxis);
   g_sSensorsData.s16GyroX = l_sRawAxis.RawSigned.s16DataX;
   g_sSensorsData.s16GyroY = l_sRawAxis.RawSigned.s16DataY;
   g_sSensorsData.s16GyroZ = l_sRawAxis.RawSigned.s16DataZ;
}

#endif /* (EN_LSM6DSL == 1) */



#if (EN_LIS2MDL == 1)
/**@brief Function to stop Mag value acquisition.
 */
static void vMagShutdown(void)
{
}
/**@brief Function to activate Mag value acquisition from sensor.
 */
static void vMagWakeUp(void)
{
}
/**@brief Function to get Mag measurement.
 */
static void vMagGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size)   
{
   int16_t l_s16MagX = 0;
   int16_t l_s16MagY = 0;
   int16_t l_s16MagZ = 0;
   if(eLIS2MDL_MagneticRead() == LIS2MDL_ERROR_NONE)
   {
      if(eLIS2MDL_MagDataGet(&l_s16MagX, &l_s16MagY, &l_s16MagZ) == LIS2MDL_ERROR_NONE)
      {
         g_sSensorsData.s16MagX = l_s16MagX;
         g_sSensorsData.s16MagY = l_s16MagY;
         g_sSensorsData.s16MagZ = l_s16MagZ;
      }
   }
}
#endif /* (EN_LIS2MDL == 1) */

#if (EN_BATT == 1)
/**@brief Function to stop battery value acquisition from SAADC.
 */
static void vBatteryStop(void)
{   
//   vBattery_ShutDown();
}
/**@brief Function to activate battery value acquisition from sensor.
 */
static void vBatteryActivate(void)
{   
//   vBattery_Init();
}
/**@brief Function to get battery measurement.
 */
static void vBatteryGet(uint8_t * p_pau8Data, uint8_t * p_pu8Size)
{
   uint8_t l_u8Size = 0u;
   uint8_t l_au8Battery_mV[4u] = { 0u };
   uint8_t l_u8BatteryPercent = 0u;
   uint16_t l_u16BatteryVoltage = 0u;
   
   vBattery_Init();
   
   vBattery_Read(l_au8Battery_mV,&l_u8Size);
   
   vBattery_ShutDown();
   
   l_u16BatteryVoltage = U8_TO_U16(l_au8Battery_mV[2u],l_au8Battery_mV[3u]);
   l_u8BatteryPercent = battery_level_in_percent(l_u16BatteryVoltage);
   
   g_sSensorsData.u8BatteryPercent = l_u8BatteryPercent;
   g_sSensorsData.u16BatteryVoltage = l_u16BatteryVoltage;
}
#endif  /* (EN_BATT == 1) */

/************************************************************************
 * End Of File
 ************************************************************************/


