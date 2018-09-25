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
 * Date:          27/03/2018 (dd MM YYYY)
 * Author:        Yoann Rebischung
 * Description:   Flash Storage Interface for SID 
 *
 */
 
/****************************************************************************************
 * Include Files
 ****************************************************************************************/
#include <stdint.h>
#include <string.h>

#include "BoardConfig.h"
#include "GlobalDefs.h"

/* Libraries include */
#include "Libraries/Memory.h"

/* Self include */
#include "SID_Storage.h"

/****************************************************************************************
 * Defines
 ****************************************************************************************/
#define MAX_SIZE_DATA         ((uint8_t)20) /* Max size for config */
#define NOT_FOUND             ((uint16_t)0xFFFF)
#define UNKNOWN_IDX           ((uint8_t) 0xFF)

#define FILEID_NAME           ((uint16_t)0x1111)
#define KEY_NAME              ((uint16_t)0x0110)

#define FILEID_FRAMECFG       ((uint16_t)0x2222)
#define KEY_FRAMECFG          ((uint16_t)0x0220)

#define DEFAULT_NAME          "Exo_SysID"

/****************************************************************************************
 * Private type declarations
 ****************************************************************************************/
 
/****************************************************************************************
 * Private function declarations
 ****************************************************************************************/
static void vReadNameCallback(void * p_pvData, uint16_t p_u16WordLength);
static void vReadStorageCallback(void * p_pvData, uint16_t p_u16WordLength);

/****************************************************************************************
 * Variable declarations
 ****************************************************************************************/
static uint8_t g_u8IsMemInitialized = 0u;

static uint32_t g_acu32Data[MAX_SIZE_DATA] = { 0u };
static uint32_t const * g_pcu32Data = g_acu32Data;
static volatile uint16_t g_u16ReadWordSize = 0u;
static volatile uint16_t g_u16Readu8Size = 0u;

/****************************************************************************************
 * Public functions
 ****************************************************************************************/ 
/**@brief Initialize SID Flash Storage.
 * @return Error Code
 */
e_SIDStorage_Error_t eStorage_Init(void)
{
   e_SIDStorage_Error_t l_eErrCode;
   char l_achDeviceName[NAME_SIZE] = { '\0' };
   uint8_t l_u8NameSize = 0u;
   
   if(eMemory_Init() != MEM_ERROR_NONE)
   {
      g_u8IsMemInitialized = 0u;
      l_eErrCode = SID_STORAGE_ERROR_INIT;
   }
   else
   {
      g_u8IsMemInitialized = 1u;
      if(eStorage_DeviceNameRead(l_achDeviceName, &l_u8NameSize) == SID_STORAGE_ERROR_NOT_FOUND)
      {
         l_u8NameSize = strlen(DEFAULT_NAME);
         memcpy(l_achDeviceName, DEFAULT_NAME, l_u8NameSize);
         if(eStorage_DeviceNameWrite(l_achDeviceName, l_u8NameSize) == SID_STORAGE_ERROR_WRITE)
         {
            g_u8IsMemInitialized = 0u;
         }
      }
//      uint8_t l_au8Data[6u] = { 0x01, 0x00, 0x72, 0x32, 0x10, 0xFF };
//      if(eStorage_FrameConfigWrite(l_au8Data, 6u) != SID_STORAGE_ERROR_NONE)
//      {
//         __nop();
//      }
      
      l_eErrCode = SID_STORAGE_ERROR_NONE;
   }
   
   return l_eErrCode;
}

/**@brief Write Frame configuration for CC430 in Flash Storage.
 * @param[in] p_pau8Config Frame configuration
 * @param[in] p_u8Size Size of Config(must be multiple of FRAME_CFG_WINDOWS_SIZE)
 * @return Error Code
 */
e_SIDStorage_Error_t eStorage_FrameConfigWrite(uint8_t * p_pau8Config, uint8_t p_u8Size)
{
   e_SIDStorage_Error_t l_eErrCode;
   
   if(g_u8IsMemInitialized == 1u)
   {      
      memset((void*)g_pcu32Data, 0u, MAX_SIZE_DATA);
      memcpy((void*)g_pcu32Data, p_pau8Config, p_u8Size);
      if(eMemory_Write(FILEID_FRAMECFG, KEY_FRAMECFG, (void*)g_pcu32Data, (uint16_t)p_u8Size) == MEM_ERROR_NONE)
      {
         l_eErrCode = SID_STORAGE_ERROR_NONE;
      }
      else
      {
         l_eErrCode = SID_STORAGE_ERROR_WRITE;
      }
   }
   else
   {
      l_eErrCode = SID_STORAGE_ERROR_INIT;
   }
   
   return l_eErrCode;
}

/**@brief Read Frame configuration for CC430 in Flash Storage.
 * @param[in] p_pau8Config Frame configuration
 * @param[in] p_pu8Size Size of Config(must be multiple of FRAME_CFG_WINDOWS_SIZE)
 * @return Error Code
 */
e_SIDStorage_Error_t eStorage_FrameConfigRead(uint8_t * p_pau8Config, uint8_t * p_pu8Size)
{   
   e_SIDStorage_Error_t l_eErrCode;
   e_MemoryErrCode_t l_eMemErrCode;
   uint16_t l_u16Size = 0u;
   
   if(g_u8IsMemInitialized == 1u)
   {
      l_eMemErrCode = eMemory_Read(FILEID_FRAMECFG, KEY_FRAMECFG, vReadStorageCallback);
      if(l_eMemErrCode == MEM_ERROR_NONE)
      {
         l_u16Size = (g_u16ReadWordSize << 2u);
         memcpy(p_pau8Config, (void*)g_pcu32Data, l_u16Size);
         (*p_pu8Size) = l_u16Size;
         l_eErrCode = SID_STORAGE_ERROR_NONE;
      }
      else if(l_eMemErrCode == MEM_ERROR_NOT_FOUND)
      {   
         l_eErrCode = SID_STORAGE_ERROR_NOT_FOUND;
      }
      else
      {   
         l_eErrCode = SID_STORAGE_ERROR_READ;
      }
   }
   else
   {
      l_eErrCode = SID_STORAGE_ERROR_INIT;
   }
   
   return l_eErrCode;
}

/**@brief Write Device Name in Flash Storage.
 * @param[in] p_pau8Name Device Name
 * @param[in] p_u8Size Size of Name
 * @return Error Code
 */
e_SIDStorage_Error_t eStorage_DeviceNameWrite(char * p_pachName, uint8_t p_u8Size)
{
   e_SIDStorage_Error_t l_eErrCode;
   uint8_t l_u8NameLght = 0u;
   char l_achName[NAME_SIZE] = { '\0' };
   
   if(g_u8IsMemInitialized == 1u)
   {
      l_u8NameLght = (p_u8Size + 1u);
      if(l_u8NameLght < NAME_SIZE)
      {
         memcpy(l_achName, p_pachName, p_u8Size);
         memset((void*)g_pcu32Data, 0u, MAX_SIZE_DATA);
         l_achName[l_u8NameLght] = '\0';
         memcpy((void*)g_pcu32Data, l_achName, l_u8NameLght);

         if(eMemory_Write(FILEID_NAME, KEY_NAME, (void*)g_pcu32Data, (uint16_t)NAME_SIZE) == MEM_ERROR_NONE)
         {
            l_eErrCode = SID_STORAGE_ERROR_NONE;
         }
         else
         {
            l_eErrCode = SID_STORAGE_ERROR_WRITE;
         }
      }
      else
      {
         l_eErrCode = SID_STORAGE_ERROR_PARAM;
      }
   }
   else
   {
      l_eErrCode = SID_STORAGE_ERROR_INIT;
   }
   
   return l_eErrCode;
}

/**@brief Read Device Name in Flash Storage.
 * @param[in] p_pachName Device Name
 * @param[in] p_pu8Size Size of Name
 * @return Error Code
 */
e_SIDStorage_Error_t eStorage_DeviceNameRead(char * p_pachName, uint8_t * p_pu8Size)
{
   e_SIDStorage_Error_t l_eErrCode;
   e_MemoryErrCode_t l_eMemErrCode;
   
   if(g_u8IsMemInitialized == 1u)
   {
      l_eMemErrCode = eMemory_Read(FILEID_NAME, KEY_NAME, vReadNameCallback);
      if(l_eMemErrCode == MEM_ERROR_NONE)
      {
         strcpy(p_pachName, (char*)g_pcu32Data);   
         (*p_pu8Size) = strlen(p_pachName);
         
         l_eErrCode = SID_STORAGE_ERROR_NONE;
      }
      else if(l_eMemErrCode == MEM_ERROR_NOT_FOUND)
      {   
         l_eErrCode = SID_STORAGE_ERROR_NOT_FOUND;
      }
      else
      {   
         l_eErrCode = SID_STORAGE_ERROR_READ;
      }
   }
   else
   {
      l_eErrCode = SID_STORAGE_ERROR_INIT;
   }
   
   return l_eErrCode;
}

/****************************************************************************************
 * Private functions
 ****************************************************************************************/
static void vReadNameCallback(void * p_pvData, uint16_t p_u16WordLength)
{
   memcpy((void*)g_pcu32Data, p_pvData, NAME_SIZE);
}

static void vReadStorageCallback(void * p_pvData, uint16_t p_u16WordLength)
{
   g_u16ReadWordSize = p_u16WordLength;
   memcpy((void*)g_pcu32Data, p_pvData, (p_u16WordLength << 2u));
}
/****************************************************************************************
 * End Of File
 ****************************************************************************************/


