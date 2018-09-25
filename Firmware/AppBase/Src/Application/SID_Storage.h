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
 */
#ifndef SID_STORAGE_H
#define SID_STORAGE_H

/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define NAME_SIZE    (uint8_t)20

/****************************************************************************************
 * Type definitions
 ****************************************************************************************/
typedef enum _SID_STORAGE_ERROR_ {
   SID_STORAGE_ERROR_NONE = 0u,
   SID_STORAGE_ERROR_INIT,
   SID_STORAGE_ERROR_NOT_FOUND,
   SID_STORAGE_ERROR_WRITE,
   SID_STORAGE_ERROR_READ,
   SID_STORAGE_ERROR_ERASE,
   SID_STORAGE_ERROR_FULL,
   SID_STORAGE_ERROR_PARAM
}e_SIDStorage_Error_t;

/****************************************************************************************
 * Public function declarations
 ****************************************************************************************/
e_SIDStorage_Error_t eStorage_Init(void);

e_SIDStorage_Error_t eStorage_FrameConfigWrite(uint8_t * p_pau8Config, uint8_t p_u8Size);
e_SIDStorage_Error_t eStorage_FrameConfigRead(uint8_t * p_pau8Config, uint8_t * p_pu8Size);

e_SIDStorage_Error_t eStorage_DeviceNameWrite(char * p_pachName, uint8_t p_u8Size);
e_SIDStorage_Error_t eStorage_DeviceNameRead(char * p_pachName, uint8_t * p_pu8Size);

#endif /* SID_STORAGE_H */

