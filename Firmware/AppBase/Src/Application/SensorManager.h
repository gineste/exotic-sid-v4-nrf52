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
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

/************************************************************************
 * Include Files
 ************************************************************************/
#include <stdint.h>

/************************************************************************
 * Defines
 ************************************************************************/

/************************************************************************
 * Type definitions
 ************************************************************************/
typedef struct _SENSORS_DATA_ {
	int16_t s16Temperature;
	uint16_t u16Pressure;
   int16_t s16AccelX;
   int16_t s16AccelY;
   int16_t s16AccelZ;
   int16_t s16GyroX;
   int16_t s16GyroY;
   int16_t s16GyroZ;
   int16_t s16MagX;
   int16_t s16MagY;
   int16_t s16MagZ;
   uint16_t u16BatteryVoltage;
	uint8_t u8Humidity;
   uint8_t u8BatteryPercent;   
}s_SensorsData_t;

/************************************************************************
 * Public function declarations
 ************************************************************************/
void vSensorMngr_Init(void);
void vSensorMngr_ShutDownAll(void);
void vSensorMngr_ShutdownSensor(void);
void vSensorMngr_UpdateSensorCfg(void);
void vSensorMngr_WakeUpSensor(void);
void vSensorMngr_DataUpdate(void);
void vSensorMngr_DataGet(s_SensorsData_t ** p_ppsSensorsData);

void vSensorMngr_TemperatureGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
void vSensorMngr_PressureGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
void vSensorMngr_HumidityGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
void vSensorMngr_AccelerometerGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
void vSensorMngr_GyrometerGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
void vSensorMngr_MagnetomerGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
void vSensorMngr_BatteryGet(uint8_t * p_pu8Data, uint8_t * p_pu8DataSize);
   
uint8_t u8SensorMngr_IsSensorsDead(void);


#endif /* SENSOR_MANAGER_H */

