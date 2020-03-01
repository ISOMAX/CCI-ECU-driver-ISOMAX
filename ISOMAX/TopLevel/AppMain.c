/* ************************************************************************ */
/*!
   \file
   \brief      Sample main function
   \details    This file contains a simple ISOBUS ECU driver sample how to setup the library. \n
               This file is only used for generating the doxygen documentation.

   \author     Wegscheider Peter
   \date       Created 13.04.2015

   \copyright  Wegscheider Hammerl Ingenieure Partnerschaft

   \par  History:
   \par
   13.04.2015 - author P. Wegscheider
   - Created to use with doxygen
*/
/* ************************************************************************ */


#include "IsoDef.h"
#include "AppHW.h"
#include "../AddOn/AppIso_Output.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stddef.h"
#include "GeometryHandling_initNVS.h"
#include "VIEngine.h"
#include "SpeedDistanceAreaHelper.h"

#include "sdkconfig.h"


/*! \brief used in the samples
   \details Replace this function with our own implementation ... */
#define TimeMs_get hw_GetTimeMs

   /*! \name Forward declaration of the public implement sample functions
    @{ */
extern void AppImpl_Ignition(iso_bool qIgnition);
/*! @} */

/* **************************  function declarations  ********************* */

void AppHW_Init(void);
void AppIso_Init(void);
void AppIso_Cyclic(void);

/* **************************  declare private functions ****************** */

/* Needed callback functions for the ISOBUS driver*/
static void    CB_Watchdog(void);
static void    CB_ReportError_iso_Core(iso_u8 functionId_u8, iso_u8 locationId_u8, iso_s16 errorCode_si);
static void    CB_ReportError_iso_Base(iso_u8 functionId_u8, iso_u8 locationId_u8, iso_s16 errorCode_si);
static void    CB_ReportError_IsoClients(iso_u8 functionId_u8, iso_u8 locationId_u8, iso_s16 errorCode_si);
static iso_s16 CB_CanSend(iso_u8 canNode_u8, iso_u32 canId_u32, const iso_u8 canData_au8[], iso_u8 canDataLength_u8);
static iso_s16 CB_GetSendMsgFiFoSize(iso_u8 u8CanNode, iso_u8 u8MsgPrio);
static iso_s32 CB_GetTimeMs(void);

/* CAN message receive and forward function */
static void Do_ReceiveCanMessages(void);

/* **************************  const data initialization ****************** */

/* **************************  module global data  ************************ */

#ifdef ISO_EXTERNAL_POOLBUFFER
/*!\brief Use external pool RAM buffer array  */
//static iso_u8 HUGE_C m_au8PoolRAMBuffer[RAMSIZEPOOL];
#endif

#if defined(ISO_EXTERNAL_DDOPBUFFER)
/*!\brief external RAM buffer for the TCC DDOP */
//static iso_u8  HUGE_C  m_au8TccDevDescBuffer[DEVICEDESCRIPTION_SIZE];
#endif

static void MainLoop(void *arg) {
	/* sample main loop */
	while (hw_PowerSwitchIsOn()) {
		/* run cyclic application function */
		AppIso_Cyclic();
        vTaskDelay(pdMS_TO_TICKS(10)); //10ms
	}
    vTaskDelete(NULL);
}

/* ************************************************************************ */
/*! \brief Sample main function */
#define RX_TASK_PRIO            8




void app_main()
{

    esp_log_level_set("*", ESP_LOG_VERBOSE);

	initNVS();


   /* Initialize application */
   AppHW_Init();

   /* Initialize ISOBUS library and samples */
   AppIso_Init();


   xTaskCreatePinnedToCore(MainLoop, "MainLoop", 8192, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
   xTaskCreatePinnedToCore(speedDistanceAreaHelper_Task, 				 "speedDistanceAreaHelper_Task", 				 4096, NULL, RX_TASK_PRIO, NULL, portNUM_PROCESSORS - 1);
   xTaskCreatePinnedToCore(speedDistanceAreaHelper_Task_StorageLifetime, "speedDistanceAreaHelper_Task_StorageLifetime", 4096, NULL, RX_TASK_PRIO, NULL, portNUM_PROCESSORS - 1);



   //hw_Shutdown();
}

/* **************************  implementation - public functions  ********* */

/*! \brief Sample application initialization */
void AppHW_Init(void)
{  /* Initialize the hardware*/
   hw_Init();
   hw_CanInit(ISO_CAN_NODES);
}

/*! \brief Sample ISOBUS library initialization */
void AppIso_Init(void)
{
   iso_s16 s16Ret, s16FnRet, u16I;
   iso_u32 u32version;

   u32version = IsoDriverVersionRead();
   hw_DebugPrint("ISOBUS library version: %d \n", u32version);

   /* Initialize the core ISOBUS driver library package */
   /* V10 replaced with iso_CoreInit()
   iso_DlCallbacksInit(CB_GetTimeMs, CB_Watchdog, CB_ReportError);
   iso_DlDrvCanInit(CB_CanSend, CB_GetSendMsgFiFoSize); */
   s16Ret = iso_CoreInit( CB_GetTimeMs, CB_Watchdog, CB_ReportError_iso_Core,
                          CB_CanSend, CB_GetSendMsgFiFoSize,
                          IsoCbBaseDataDistributor, IsoCbBaseNetworkDistributor, 0 );
   /* do next step for each CAN node */
   for (u16I = 0; u16I < ISO_CAN_NODES; u16I++)
   {  /* set number of (E)TP messages for each cycle (V10: Replaced iso_DlTPRepeatSet() with iso_CoreTPRepeatSet()) */
      s16FnRet = iso_CoreTPRepeatSet(u16I, 5);
      s16Ret = (s16Ret == E_NO_ERR) ? s16FnRet : s16Ret;
   }

   /* Initialize the base ISOBUS driver library package */
   s16FnRet = iso_BaseInit( CB_GetTimeMs, CB_Watchdog, CB_ReportError_iso_Base,
                            IsoCbDataOfBasicDistributor );
   s16Ret = (s16Ret == E_NO_ERR) ? s16FnRet : s16Ret;

#if defined(_LAY6_) || defined(_LAY10_) || defined(_LAY13_)
   /* Initialize the ISOBUS driver library clients modules (Hint: LAY14 only in combination with LAY6) */
   /* V10: done with iso_BaseInit() IsoDrvCbBasicDataInstall(IsoCbDataOfBasicDistributor); */
   s16FnRet = IsoClientsInit(CB_GetTimeMs, CB_Watchdog, CB_ReportError_IsoClients);
   s16Ret = (s16Ret == E_NO_ERR) ? s16FnRet : s16Ret;

#ifdef ISO_EXTERNAL_POOLBUFFER
   /* Set external RAM buffer for the VTC pool service */
   (void) IsoPoolSetBuffer(malloc(RAMSIZEPOOL));
#endif
#if defined(ISO_EXTERNAL_DDOPBUFFER)
   /* Set external RAM buffer for the TCC DDOP */
   (void)IsoTcPoolSetBuffer(malloc(DEVICEDESCRIPTION_SIZE));
#endif

#endif /* defined(_LAY6_) || defined(_LAY10_) || defined(_LAY13_) */

   /* Initialize implement sample */
   AppImpl_Ignition(ISO_TRUE);


   if (s16Ret != E_NO_ERR)
   {
      hw_LogError("AppIso_Init error: %i \n", s16Ret);
   }
}

/* ************************************************************************ */
/*! \brief Sample: cyclic function */
void AppIso_Cyclic(void)
{
   /* Get the incoming CAN messages and forward them to the ISOBUS driver */
   Do_ReceiveCanMessages();

   /* Call the ISOBUS driver cyclic functions */
   /* iso_DlCyclicGlobal(); removed with V10 */
   iso_CoreCyclic();
   iso_BaseCyclic();
#ifdef  ISO_MODULE_CLIENTS // same as #if defined(_LAY6_) || defined(_LAY10_) || defined(_LAY13_)
   (void) IsoClientsCyclicCall();
#endif
}


/* **************************  implementation - static functions  ********* */

/*! \brief ISOBUS driver dummy watchdog callback function */
static void CB_Watchdog(void)
{

}

/*! \brief ISOBUS driver sample error report callback function */
static void CB_ReportError_iso_Core(iso_u8 functionId_u8, iso_u8 locationId_u8, iso_s16 errorCode_si)
{
   hw_LogError("iso_Core error: %i, location: %i, function: %i \n", errorCode_si, locationId_u8, functionId_u8);
}

/*! \brief ISOBUS driver sample error report callback function */
static void CB_ReportError_iso_Base(iso_u8 functionId_u8, iso_u8 locationId_u8, iso_s16 errorCode_si)
{
   hw_LogError("iso_Base error: %i, location: %i, function: %i \n", errorCode_si, locationId_u8, functionId_u8);
}

/*! \brief ISOBUS driver sample error report callback function */
static void CB_ReportError_IsoClients(iso_u8 functionId_u8, iso_u8 locationId_u8, iso_s16 errorCode_si)
{
	if (errorCode_si == -8
	//TODO !!! Check why we get so many Errors before Pool is loaded !!
	//must do when we have JTAG Board available.
			&& locationId_u8 == 15
			&& functionId_u8 == 2) {
		return;
	}
	if (errorCode_si == -8
	//TODO !!! Check why we get so many Errors before Pool is loaded !!
	//must do when we have JTAG Board available.
			&& locationId_u8 == 14
			&& functionId_u8 == 168) {
		return;
	}

	switch (errorCode_si) {
	case E_OUT_OF_MEMORY:
		switch (locationId_u8) {
		case M_TCCONN:
			switch (functionId_u8) {
			case ISOTC_DEVICEELEMENT_SET:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICEELEMENT_SET", "M_TCCONN", "E_OUT_OF_MEMORY");
				break;
			default:
				break;
			}
			break;
		case M_TCPOOL:
			switch (functionId_u8) {
			case ISOTC_SUBCMDPOOL:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_SUBCMDPOOL", "M_TCPOOL", "E_OUT_OF_MEMORY");
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case E_OVERFLOW:
		switch (locationId_u8) {
		case M_TCPOOL:
			switch (functionId_u8) {
			case ISOTC_DEVICE_SET:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICE_SET", "M_TCPOOL", "E_OVERFLOW");
				break;
			case ISOTC_DEVICEELEMENT_SET:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICE_SET", "M_TCPOOL", "E_OVERFLOW");
				break;
			case ISOTC_ADDDPTOBJECT:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICE_SET", "M_TCPOOL", "E_OVERFLOW");
				break;
			case ISOTC_DEVICEPROPERTY_SET:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICE_SET", "M_TCPOOL", "E_OVERFLOW");
				break;
			case ISOTC_DEVICEPROCESSDATA_SET:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICEPROCESSDATA_SET", "M_TCPOOL", "E_OVERFLOW");
				break;
			case ISOTC_DEVICEVALUEPRESENT_SET:
				hw_LogError("IsoClients error: %s, location: %s, function: %s \n", "ISOTC_DEVICEVALUEPRESENT_SET", "M_TCPOOL", "E_OVERFLOW");
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

   hw_LogError("IsoClients error: %i, location: %i, function: %i \n", errorCode_si, locationId_u8, functionId_u8);
}

/*! \brief ISOBUS driver CAN message send callback function */
static iso_s16 CB_CanSend(iso_u8 canNode_u8, iso_u32 canId_u32, const iso_u8 canData_au8[], iso_u8 canDataLength_u8)
{
   return hw_CanSendMsg(canNode_u8, canId_u32, canData_au8, canDataLength_u8);
}

/*! \brief ISOBUS driver "Get CAN message FIFO size" callback function */
static iso_s16 CB_GetSendMsgFiFoSize(iso_u8 u8CanNode, iso_u8 u8MsgPrio)
{  /* we don't check here and return always 20 free buffer entries ... */
   return 20;
   /* return hw_CanGetFreeSendMsgBufferSize(u8CanNode); */
}

/* ************************************************************************ */

/*! \brief ISOBUS driver Timestamp callback function
   \details Must return the time in milliseconds. \n
   The return value must be greater or equal zero. \n
   An overflow is not allowed.... */
static iso_s32 CB_GetTimeMs(void)
{
   return (iso_s32)hw_GetTimeMs();
}

/*! \brief CAN message receive and forward function
   \details This sample function forwards the incoming CAN messages to the ISOBUS driver */
   /*! [Do_ReceiveCanMessages] */
static void Do_ReceiveCanMessages(void)
{
   uint8_t  canNode_u8;
   uint32_t canId_u32;
   uint8_t  canData_au8[8];
   uint8_t  canDataLength_u8;

   iso_bool msgFound;
   uint8_t  msgCount = 0u;

   do
   {
      msgFound = ISO_FALSE;

      for (canNode_u8 = 0u; canNode_u8 < ISO_CAN_NODES; canNode_u8++)
      {
         int16_t   ret_s16;
         ret_s16 = hw_CanReadMsg(canNode_u8, &canId_u32, canData_au8, &canDataLength_u8);

         if (ret_s16 > 0)
         {  /* call the ISOBUS library receive function */
            /* V10 Renamed iso_DlCanMsgRec() to iso_CoreCanMsgRec()*/
            iso_CoreCanMsgRec(canNode_u8, canId_u32, canData_au8, canDataLength_u8);
            msgFound = ISO_TRUE;
            msgCount++;
         }
      } /* end for */
   } while ((msgFound == ISO_TRUE) && (msgCount <= 40u));
}
/*! [Do_ReceiveCanMessages] */

/* ************************************************************************ */
