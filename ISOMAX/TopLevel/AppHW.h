/* ************************************************************************ */
/*! \file
   \brief      Hardware simulation interface
   \author     Wegscheider Peter
   \date       Created XX.02.15
   \copyright  Wegscheider Hammerl Ingenieure Partnerschaft
   \par        History:
   \par
   02.02.2015 - author P. Wegscheider
   - created
   \par
   08.12.2016 - author P. Wegscheider
   - updated

*/
/* ************************************************************************ */
#ifndef DEF_APP_HW_H
#define DEF_APP_HW_H
/* ************************************************************************ */


#include "esp_log.h"

#ifndef _lint
#include <stdint.h>
#else
typedef signed char        int8_t;
typedef short              int16_t;
typedef long               int32_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned long      uint32_t;
#endif

/* ************************************************************************ */
#ifdef __cplusplus
extern "C" {
#endif
/* ************************************************************************ */

   typedef char char_t;

/* ************************************************************************ */

   void     hw_Init(void);
   void     hw_Shutdown(void);
   uint8_t  hw_PowerSwitchIsOn(void);

   void     hw_DebugPrint(const char_t format[], ...); /*lint !e960 */
   void     hw_LogError(const char_t format[], ...);   /*lint !e960 */

   int32_t  hw_GetTimeMs(void);

   void     hw_CanInit(uint8_t maxCanNodes_u8);
   void     hw_CanClose(void);
   int16_t  hw_CanSendMsg(uint8_t canNode_u8, uint32_t canId_u32, const uint8_t canData_au8[], uint8_t canDataLength_u8);
   int16_t  hw_CanReadMsg(uint8_t canNode_u8, uint32_t *canId_pu32, uint8_t canData_pau8[], uint8_t *canDataLength_pu8);
   int16_t  hw_CanGetFreeSendMsgBufferSize(uint8_t canNode_u8);


#define AppHW_TAG             "AppHW"
#define hw_LogError( format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_WARN,    AppHW_TAG, format, ##__VA_ARGS__)
#define hw_DebugPrint( format, ... ) ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO,    AppHW_TAG, format, ##__VA_ARGS__)


extern uint32_t  u32SeriNoGet(void);
extern int Serial_Number_VariableGet(char * c_Serial_Number_Variable);


   
   
/* ************************************************************************ */
#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* DEF_APP_HW_H */
/* ************************************************************************ */

