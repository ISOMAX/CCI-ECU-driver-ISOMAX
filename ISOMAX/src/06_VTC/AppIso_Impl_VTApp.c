
//******************************************************************************/
/*!
   \file           

   \brief          Implement application 
  
   \author         Erwin Hammerl
   \date           Created 17.09.2007 

   \copyright      Wegscheider Hammerl Ingenieure Partnerschaft

   \par HISTORY:

*/
/* **************************  includes ********************************** */
//#include <iostream>

#include <string.h>
#include <stdio.h>

#include "IsoDef.h"

#if defined(_LAY6_) && (!defined(WBZ_GRAPHIC_AUX_SAMPLE) || !defined(ISO_VTC_GRAPHIC_AUX))

#include "MyProject1.iop.h"        // ISO Object Pool
#include "MyProject1.c.h"

#include "VIEngine.h"

#include "AppIso_Implement.h"
#include "GeometryHandling.h"
#include "General_Settings.h"

#include "esp_log.h"

#include "AppIso_Impl_VTApp.h"
#include "SpeedDistanceAreaHelper.h"
#include "sdkconfig.h"

// Define for ISO User
//#define IMPLEMENT__ECU1         144           ///< Wished default source address of ISO implement user
#define ALARM_AMOUNT            50            ///< Maximum number of alarms

#define ALARM_EMER_JOYTASTE     1             ///< Alarms
#define ALARM_EMER_INPUT_WRONG  2
#define ALARM_PROCESS           3
#define ALARM_INFO              4

#define ALARM_PROCESS_MASK      1202
#define ALARM_INFO_MASK         1203

//********************************************************************************************
// Each command has several Unique Features. here they are encapsulated !
//********************************************************************************************
typedef struct {
	//
	//! The Commandnumber is here redundant, however a Testdriver can check for Validity !!!
	//
	iso_u16 objectIdOfAuxObject; // objectIdOfAuxObject
	iso_u16 objectIdOfButtonObject; // objectIdOfButtonObject

} CommandTranslateObject;

CommandTranslateObject CommandTranslate[] = {
//{ AuxFunction2_PLUS_1,		SoftKey_PLUS_1		}, //	0
//{ AuxFunction2_ON_OFF,		SoftKey_ON_OFF			} //	1

};


//make this Defines (Right side)
#define FIRST_AUX 0//AuxFunction2_PLUS_1
#define LAST_AUX 1//AuxFunction2_ON_OFF
//do not Change this. 
#define NUM_AUX_FUNCTIONS ((LAST_AUX-FIRST_AUX)+1)
iso_s32 InputSignalData_old_value1[NUM_AUX_FUNCTIONS];

void APP_VITask_INIT(void) {
	int i;
	for (i = 0; i < NUM_AUX_FUNCTIONS; i++) {
		InputSignalData_old_value1[i] = AUX_PRESS_OFF;
	}


}
#define USE_PREF_ASSIGN_CB                   ///< use prefered assignment callback function 

/* ****************************** local data   *************************** */
// for test use
//static int            i_NrCmd = 0;                   // only for tests
//static iso_bool       b_CounterAktiv = ISO_TRUE;
//static iso_u16        w_Counter = 0;
//static iso_bool       q_AlarmInputActive = ISO_FALSE;
//static iso_bool       b_Alternate = ISO_FALSE;

// Necessary for application 
static ISOALARM_T     asAlarm[ALARM_AMOUNT];     ///< Structure for alarms
iso_s16        s16CfHandleWs;           
iso_bool       q_ProOnScreen = ISO_FALSE; ///< ISO_TRUE if pool is open on terminal
iso_bool       q_ProOnScreen_and_Connected = ISO_FALSE; ///< ISO_TRUE if pool is open on terminal
iso_bool       q_Connected = ISO_FALSE; ///< ISO_TRUE if pool is open on terminal
iso_u16        w_Page;                    ///< Actual data mask open on terminal

iso_u8 initVIofApplication = ISO_FALSE;
iso_u8 actualLanguage[6];
static iso_u8 ReloadLanguagePool = ISO_FALSE;


/* ****************************** function prototyps ****************************** */
static int  AppPoolSettings         ( void );
static int  AppCyclicActionsHandler        ( void );
static void CbVtStatus              ( const ISOVT_STATUS_DATA_T* psStatusData );
static void CbVtMessages            ( const ISOVT_MSG_STA_T * pIsoMsgSta );
static void CbVtConnCtrl            ( const ISOVT_EVENT_DATA_T* psEvData );
static void CbAuxPrefAssignment     ( VT_AUXAPP_T asAuxAss[], iso_s16* ps16MaxNumberOfAssigns, ISO_USER_PARAM_T userParam );

void Init_VI(void);
void SecondaryAdaptation(OBJTYP_e eObjectType, iso_u16 u16ObjectId, iso_u8 HUGE_C pau8Object[], iso_u32* pu32ObjectSize, ISO_USER_PARAM_T userParam);
void AppVTClientSet( iso_s16 s16CfHandle )
{
   ISO_USER_PARAM_T  userParamDefault = ISO_USER_PARAM_DEFAULT;
   ISO_CF_NAME_T abNamePreferredVT = { 3, 0, 0xc0, 0xc, 8, 0x1d, 0, 0xa0 }; // NT02 G2 extern assignment
   //ISO_CF_NAME_T abNamePreferredVT[8] = { 0x40, 0xb2, 0x54, 0x2d, 8, 0x1d, 0, 0xa0 }; // CCI
   //iso_u8 abNameMember[8] = { 0, 0, 0x74, 0x2b, 0, 0x77, 0, 0xa0 };
   
   // Init VT control client - recommended: use connection callback 
   IsoVTInit(s16CfHandle, userParamDefault, CbVtStatus, CbVtMessages, CbVtConnCtrl, &abNamePreferredVT );  // Init of VT instance
   IsoPoolSecondaryAdaptInit(SecondaryAdaptation);

#if !defined(ISO_VTC_DISABLE_AUX_N) && defined(USE_PREF_ASSIGN_CB)
   // This optional callback can be set after IsoVTInit()
   (void)IsoAuxPrefAssignmentCbSet(&CbAuxPrefAssignment);
#endif

   IsoVTDataSet( VT_BOOTTIME, 4 );                     // Setting (EE-stored) boottime of preferred VT
   //IsoVTDataSet( WS_VERSION_NR, 3 );                // Setting WS Version (Default is ISO_VTVERSION 5 ) 
   // Init alarms 
   #if defined(ISO_VTC_ENABLE_ALARM)
   
   
   
//TODO !!!


   #endif /* defined(ISO_VTC_ENABLE_ALARM) */
// Store own NAME for later usage
   s16CfHandleWs = s16CfHandle;
}


static void CbVtConnCtrl(const ISOVT_EVENT_DATA_T* psEvData)
{
   iso_u8 abLCData[6];

   switch (psEvData->eEvent)
   {
   case IsoEvConnSelectPreferredVT:
      /* preferred VT not alive, but one or more other VTs */
      /* Sample: Select VT instance 0 or the last one in list */
      {
         iso_s16 as16VtHandles[5];
         iso_s16 s16NumbOfVtHa = 0, s16I, s16MnHandle = HANDLE_UNVALID, s16Re;

         (void)IsoClientsReadListofExtHandles(virtual_terminal, 5, as16VtHandles, &s16NumbOfVtHa);

         for (s16I = 0; s16I < s16NumbOfVtHa; s16I++)
         {
            ISOUSERAPP_T sUserInfo;
            s16MnHandle = as16VtHandles[s16I];
            s16Re = iso_NmGetDataOfMember(s16MnHandle, &sUserInfo);
            if ( (s16Re == E_NO_ERR) &&
                 ((sUserInfo.psName->bFunctionInst == 0u) || ((s16I + 1) >= s16NumbOfVtHa) ) )
            {  /* set new preferred VT */
               (void)IsoVTSetNewPreferredVT(sUserInfo.pacIsoUserName);
               break;
            }
         }
      }
      break;
   case IsoEvMaskServerVersAvailable:
#ifdef ISO_VTC_UT3
      //IsoVTObjTypeParsableSet(GraphicDataObject);  // for test purposes (must be called here)
#endif
      break;
   case IsoEvMaskLanguageCmd:
      /* Event is called if Language command has changed ! */
      IsoReadWorkingSetLanguageData(s16CfHandleWs, abLCData);

      break;
   case IsoEvMaskTechDataV4Request:
      /* If VT >= V4 then application requests some more technical data */
#ifdef ISO_VTC_UT3
      IsoGetSupportedObjects();
      IsoGetWindowMaskData();
      //IsoGetSupportedWidechar(...)
#endif
      break;
   case IsoEvMaskLoadObjects: 
      /* If Language command data known before connection starts 
         event IsoEvMaskLanguageCmd is not called ! */
      IsoReadWorkingSetLanguageData(s16CfHandleWs, abLCData);
      AppPoolSettings();
      /* Boottime of VT can be read and stored here in EEPROM
      wBoottime = IsoGetVTStatusInfo ( VT_BOOTTIME ); */
      break;
   case IsoEvMaskReadyToStore:
      /* VTC is ready to store the uploaded object pool. 
      If ISO_VTC_WAITBEFORESTORE is defined in IsoConf.h the application will stay in this state.
      In this case the event is triggered until IsoVtContinue() is called. 
      */
      break;
   case IsoEvMaskActivated:
      #ifdef ISO_VTC_UT3
      if (IsoVTObjTypeIsSupported(WindowMaskObject) == ISO_TRUE)
      #endif 
      {
      #ifdef _LAY78_
         AppActivatePGNsForVT();    /* Stop Button handling ... */
      #endif
      }
		//this case is only active if the applications appears for the first time on the VT
		//set update flags for VI so that the hole VI will be updated after an Next VT event
		Init_VI();
      break;
   case IsoEvMaskTick:      // Called only after successful login
      AppCyclicActionsHandler();   // Sending of commands etc. for mask instance
      if (IsoGetVTStatusInfo(WS_COUPLE_STATE) == Ws_MaskAux)
      {  } // Cyclic actions for auxiliary support (in case of VT has function instance 0)
      break;

      // Only if an additional connection to VT function instance 0 for auxiliary support exists 
   case IsoEvMaskLoginAborted: 
      // Login failed - application has to decide if login shall be repeated and how often
      // AppVTClientSet(abNameCl);
      break;
   case IsoEvMaskPoolReloadFinished:
      // Pool reload started with IsoPoolReload() is finished
      break;
   case IsoEvConnSafeState: 
      // Connection closed ( VT lost, VT_LOGOUT (delete object pool response was received ) )
      break;  
   case IsoEvAuxServerVersAvailable: 
      break;
   case IsoEvAuxLanguageCmd:
      //IsoClServ_ReadLCOfServer( , );
      break;
   case IsoEvAuxTechDataV4Request:
      break;
   case IsoEvAuxLoadObjects:
      AppPoolSettings();
      break;
   case IsoEvAuxReadyToStore:
      /* VTC is ready to store the uploaded object pool.
      If ISO_VTC_WAITBEFORESTORE is defined in IsoConf.h the application will stay in this state.
      In this case the event is triggered until IsoVtContinue() is called.
      */
      break;
   case IsoEvAuxActivated:
      break;
   case IsoEvAuxTick: 
      break;
   case IsoEvAuxLoginAborted: 
      // Login failed - application has to decide if login shall be repeated and how often
      
      break;
   case IsoEvAuxPoolReloadFinished:
      // Pool reload started with IsoPoolReload() is finished
      break;

   default: break;
   }

#ifdef ISO_DEBUG_ENABLED   // only for debug tracing 
   switch (psEvData->eEvent)
   {
   case IsoEvConnSelectPreferredVT:
      iso_DebugPrint("VT - Event: Select new preferred VT: \n"); break;
   case IsoEvMaskServerVersAvailable:
      iso_DebugPrint("VT - Event: MaskInst VT Version: %d \n", IsoGetVTStatusInfo(VT_VERSIONNR) ); break;
   case IsoEvMaskLanguageCmd:
	   iso_DebugPrint("VT - Event: MaskInst New LC: %d %d \n",abLCData[0],abLCData[1]); break;
   case IsoEvMaskTechDataV4Request:
      iso_DebugPrint("VT - Event: MaskInst RequestV4  \n"); break;
   case IsoEvMaskLoadObjects:
      iso_DebugPrint("VT - Event: MaskInst Technical Messages received \n"); break;
   case IsoEvMaskActivated:
      OutputVTCoupleState(IsoGetVTStatusInfo(WS_COUPLE_STATE)); break;
   case IsoEvMaskTick:  break;
   case IsoEvMaskLoginAborted:
      iso_DebugPrint("VT - Event: MaskInst connection login aborted ! \n");  break;
   case IsoEvMaskPoolReloadFinished:
      iso_DebugPrint("VT - Event: MaskInst pool reload finished ! \n"); break;
   case IsoEvMaskStateChanged:
      iso_DebugPrint("VT - Event: MaskInst changed State: %d \n", IsoGetVTStatusInfo(VT_STATEOFANNOUNCING)); break;
   case IsoEvAuxServerVersAvailable:
      iso_DebugPrint("VT - Event: AuxInst VT Version: %d \n", IsoGetVTStatusInfo(VT_VERSIONNR)); break;
   case IsoEvAuxActivated:
      OutputVTCoupleState(IsoGetVTStatusInfo(WS_COUPLE_STATE)); break;
   case IsoEvAuxLoginAborted:
      iso_DebugPrint("VT - Event: AuxInst connection login aborted ! \n"); break;
   case IsoEvAuxPoolReloadFinished:
      iso_DebugPrint("VT - Event: AuxInst pool reload finished ! \n"); break;
   case IsoEvAuxStateChanged:
      iso_DebugPrint("VT - Event: AuxInst changed State: %d \n", IsoGetVTStatusInfo(VT_STATEOFANNOUNCING)); break;
   case IsoEvConnSafeState:
      iso_DebugPrint("VT - Event: Connection deactive -> Safe State \n");  break;
   default: break;
   }
#endif 
}


void AppVTClientLogOut( )
{
   if (IsoWsSetMaskInst(s16CfHandleWs) == E_NO_ERR)
   {
      /* Log out -> Delete object pool is sent first */
      IsoVTDataSet(VT_LOGOUT, ISO_DONT_CARE_W);
      /* If available Aux instance is destroyed too */
   }
}


static int AppPoolSettings( )
{
   // Only if VT and VTC is Version 5 or higher 32 bytes are used else the first 7 bytes
	iso_u16 wSKM_Scal = ISO_FACTOR_ONE; // Scaling factor * 10000
	iso_u16 wMDM_Scal = ISO_FACTOR_ONE;   // Scaling factor * 10000


	IsoPoolInit((const iso_u8*)ISO_VERSION_LABEL, // Version
		(iso_u8 HUGE_C*)(isoOP_MyProject1), // PoolAddress
		0,// ( PoolSize not needed )
		ISO_OP_MyProject1_ObjectNumber, // Number of objects
		colour_256, // Graphic typ,
		ISO_DESIGNATOR_WIDTH, ISO_DESIGNATOR_HEIGHT, ISO_MASK_SIZE);// SKM width and height, DM res.
   /* use callback function to get the preferred assignment */
#if !defined(ISO_VTC_DISABLE_AUX_N) && defined(USE_PREF_ASSIGN_CB)
  // We have already set the CB after IsoVTInit()
  // (void)IsoAuxPrefAssignmentCbSet( & CbAuxPrefAssignment );
#endif

   	wMDM_Scal   = (iso_u16)IsoPoolReadInfo ( PoolDataMaskScalFaktor );          // Calling after PoolInit!!!
   	wSKM_Scal = (iso_u16)IsoPoolReadInfo ( PoolSoftKeyMaskScalFaktor );

	if (wSKM_Scal <= 11111u && wSKM_Scal >= 9000u ) //erst ab 111% saklieren, vorher nicht ... damit wird bei 80x80 und 72x72 nicht skaliert. 
	{
		wSKM_Scal = ISO_FACTOR_ONE;
	}
   //(void) IsoPoolDMActiveWSIdentSet( Page_2, pi_key_morePage );
   //(void) IsoPoolWSSelectableSet(ISO_FALSE);
   
   /*  Common:
       Centering is only practical and usable in top level objects like data/alarm mask, key, 
       working set descriptor, auxiliary function and input 
   */

   
   // DM:  Normal scaling
   // SKM: Mixed mode, keys scaled with SKM scaling factor and then centered 
 
	//for Testing comment this in
	//wSKM_Scal = wSKM_Scal / 2;
	//wMDM_Scal = wMDM_Scal / 2;


	/********************************************************************************************************/
	//Scale the Workingset-Object: 

	//Center after the Scaling is done on the TOP Level Element!
	IsoPoolSetIDRangeMode(WorkingSet_0, WorkingSet_0, wSKM_Scal, Scaling);  // Working set object
	/********************************************************************************************************/


	/********************************************************************************************************/
	//Scale the Soft-Keys: 

	//Center after the Scaling is done on the TOP Level Element!
	IsoPoolSetIDRangeMode(5000, 5999, wSKM_Scal, Centering);  // Working set picture; SoftKey
	//Scale the Containers aka ORIGINS of the Polygons !!
	IsoPoolSetIDRangeMode(3500, 3999, wSKM_Scal, Scaling);
	//Scale the Objects itself aka 
	//do modify the Type of Polygons

	// 10.6.2018: no more Strings on SK Mask.
	IsoPoolSetIDRangeMode(14500, 14899, wSKM_Scal, Scaling);//Rectangle
	// do NOT SCALE the Rectangles 14900-14999 !!! they are Background Rectangles. no need to scale them down. 
	// also no need to Center them, because Origin i 0,0 and Centering will not affect 0.0 Coordinate. 
	IsoPoolSetIDRangeMode(15500, 15999, wSKM_Scal, Scaling);//Ellipse
	IsoPoolSetIDRangeMode(16500, 16999, wSKM_Scal, Scaling); //polygon
	IsoPoolSetIDRangeMode(24500, 24999, wSKM_Scal, Scaling); //LineAttributes
	/********************************************************************************************************/


	/********************************************************************************************************/
	//Scale the DataMask 

	//Center after the Scaling is done on the TOP Level Element!
	IsoPoolSetIDRangeMode(1000, 1999, wMDM_Scal, Centering);  // Datamask
	//Scale the Containers aka ORIGINS of the Polygons !!
	IsoPoolSetIDRangeMode(3000, 3499, wMDM_Scal, Scaling); //Container

	//Scale the Button !!
	IsoPoolSetIDRangeMode(6000, 6999, wMDM_Scal, Scaling); //Buttons


	//Scale the Objects itself Strings
	IsoPoolSetIDRangeMode(11000, 11499, wMDM_Scal, Scaling); //Strings
	//Scale the Objects itself aka Font Attribute
	IsoPoolSetIDRangeMode(23000, 23499, wMDM_Scal, Scaling); //Font Attribute


	//Scale the Objects itself Polygons
	//do modify the Type of Polygons
	IsoPoolSetIDRangeMode(16000, 16499, wMDM_Scal, Scaling);

	//Scale the Objects itself LinearBargraph
	IsoPoolSetIDRangeMode(18000, 18499, wMDM_Scal, Scaling);
	IsoPoolSetIDRangeMode(12000, 12499, wMDM_Scal, Scaling);
	IsoPoolSetIDRangeMode(14000, 14499, wMDM_Scal, Scaling);
	IsoPoolSetIDRangeMode(15000, 15499, wMDM_Scal, Scaling);
	IsoPoolSetIDRangeMode(9000, 9499, wMDM_Scal, Scaling); //InputNumber
	IsoPoolSetIDRangeMode(24000, 24499, wMDM_Scal, Scaling); //LineAttributes
	/********************************************************************************************************/



	IsoPoolSetIDRangeMode(29000, 29999, wSKM_Scal, Centering); // Top Level Component: Centering. 
	if (IsoGetVTStatusInfo(VT_VERSIONNR) == VT_V2_FE)
	{
		// Transforming Auxiliary function Type 2 into Type 1
		IsoPoolSetIDRangeMode(29000, 29999, 0, AuxToV2);
	}

	return 0;
}









/*******************************************************************
*       public static  Function AppCyclicActionsHandler
********************************************************************
*
*    FUNCTION:   AppCyclicActionsHandler
*	FULL NAME:	 AppCyclicActionsHandler
*
* DESCRIPTION:	 static int AppCyclicActionsHandler()
*
*      ACCESS:   public static
*
*
*     RETURNS:   int
*
* 	   AUTHOR:	 Franz_Hoepfinger
*		 DATE:   2018/05/12
*
*******************************************************************/
static int AppCyclicActionsHandler() {
	static long l__TimeCounter = 0;

	if (IsoGetVTStatusInfo(VT_STATEOFANNOUNCING) >= pref_assignment_send) {
		if (q_ProOnScreen) {
			if (l__TimeCounter + 2000L < IsoClientsGetTimeMs()) {
				l__TimeCounter = IsoClientsGetTimeMs();

				if (ReloadLanguagePool == ISO_FALSE) {
					IsoReadWorkingSetLanguageData(s16CfHandleWs,
							actualLanguage);

					ReloadLanguagePool = ISO_TRUE;
				}
			}
		}
		switch (w_Page) {
		case DataMask_1000:
#if defined(ISO_VTC_XCR)
			if (initVIofApplication == ISO_FALSE) {
				Init_VI();
				initVIofApplication = ISO_TRUE;
			}
			break;


#endif /* defined(ISO_VTC_XCR) */
			break;
		default:
			break;
		}



#if defined(ISO_VTC_ENABLE_ALARM)
		if (IsoAlarmCmd(&asAlarm[ALARM_EMER_JOYTASTE], AlQuitTest) == 2) {
			IsoAlarmCmd(&asAlarm[ALARM_EMER_JOYTASTE], AlQuitofApplication);

			//IsoAlarmCmd ( &asAlarm[ALARM_EMER_INPUT_WRONG], AlEngage  );
		}
		if (IsoAlarmCmd(&asAlarm[ALARM_EMER_INPUT_WRONG], AlQuitTest) == 2) {
			IsoAlarmSetReturnObjId(1001);
			IsoAlarmCmd(&asAlarm[ALARM_EMER_INPUT_WRONG], AlQuitofApplication);
		}
		if (IsoAlarmCmd(&asAlarm[ALARM_PROCESS], AlQuitTest) == 2) {
			IsoAlarmCmd(&asAlarm[ALARM_PROCESS], AlQuitofApplication);
		}
#endif /* defined(ISO_VTC_ENABLE_ALARM) */

#if !defined(ISO_VTC_DISABLE_AUX_O)
		{
			/* Detecting auxiliary device unit type 1 */
			static iso_bool qAuxType1 = ISO_FALSE;
			if (qAuxType1
					!= (iso_bool) IsoGetVTStatusInfo(AUXUNITS_TYPE1_ONBUS)) {
				qAuxType1 = (iso_bool) IsoGetVTStatusInfo(AUXUNITS_TYPE1_ONBUS);
#ifdef ISO_DEBUG_ENABLED   
             iso_DebugPrint("VT - AuxDeviceUnit Type 1 detected %8.4d \n", IsoClientsGetTimeMs());
#endif /* ISO_DEBUG_ENABLED */
			}
		}
#endif /* !defined(ISO_VTC_DISABLE_AUX_O) */
	}
	// Application time controlled alarm - Disengage
	return 0;
}



/* This function is called in case of every page change - you can do e. g. initialisations ...  */

static void CbVtStatus(const ISOVT_STATUS_DATA_T *psStatusData) {

	if (!q_ProOnScreen && psStatusData->qWsActive) {
		q_ProOnScreen  = psStatusData->qWsActive;    // for global usage
		ESP_LOGI("----UT----", "------------------------------------------------------------");
		ESP_LOGI("----UT----", "we are now on Screen");
		ESP_LOGI("----UT----", "------------------------------------------------------------");



	}



   q_ProOnScreen  = psStatusData->qWsActive;    // for global usage
   w_Page         = psStatusData->wPage;


   switch (w_Page)
   {
   case DataMask_1000:
       break;
   default:
       break;
   }
}
/*******************************************************************
*       public  Function BUTTON_InputSignalCallback_RELEASED
********************************************************************
*
*    FUNCTION:   BUTTON_InputSignalCallback_RELEASED
*	FULL NAME:	 BUTTON_InputSignalCallback_RELEASED
*
* DESCRIPTION:
*
*      ACCESS:   public
*
*  PARAMETERS:	 struct ButtonActivation_S * pButtonData
*
*     RETURNS:   void
*
* 	   AUTHOR:	 Franz_Hoepfinger
*		 DATE:   2018/05/12
*
*******************************************************************/
void BUTTON_InputSignalCallback_RELEASED(struct ButtonActivation_S * pButtonData) {

	// can use one callback for multiple buttons

	// what button was released
	switch (pButtonData->objectIdOfButtonObject)
	{
	case 1://Button_xyz:

		break;
	default:
		break;
	}





}
/*******************************************************************
*       public  Function BUTTON_InputSignalCallback
********************************************************************
*
*    FUNCTION:   BUTTON_InputSignalCallback
*	FULL NAME:	 BUTTON_InputSignalCallback
*
* DESCRIPTION:
*
*      ACCESS:   public
*
*  PARAMETERS:	 struct ButtonActivation_S * pButtonData
*
*     RETURNS:   iso_u8
*
* 	   AUTHOR:	 Franz_Hoepfinger
*		 DATE:   2018/05/12
*
*******************************************************************/
iso_u8 BUTTON_InputSignalCallback(struct ButtonActivation_S *pButtonData) {


	switch (pButtonData->keyActivationCode)
	{
	case BUTTON_STATE_RELEASED:
		BUTTON_InputSignalCallback_RELEASED(pButtonData);
		break;
	case BUTTON_STATE_PRESSED:
		//BUTTON_InputSignalCallback_PRESSED(pButtonData);
		break;
	case BUTTON_STATE_HELD:
		//BUTTON_InputSignalCallback_HELD(pButtonData);
		break;
	case BUTTON_STATE_ABORTED:
		//BUTTON_InputSignalCallback_ABORTED(pButtonData);
		break;
	}


	return (ISO_TRUE);
}


/*******************************************************************
*
*    FUNCTION: Callback for Input Numbers
*
* DESCRIPTION:
*
*  PARAMETERS: N/A
*
*     RETURNS:   N/A
*
*******************************************************************/
iso_u8 IN_InputSignalCallback(struct InputNumber_S *pInputNumberData) {

	IN_InputSignalCallback_GeometryHandling(pInputNumberData);

	return (ISO_TRUE);
}

/*******************************************************************
*
*    FUNCTION: Callback for Input Strings
*
* DESCRIPTION:
*
*  PARAMETERS: N/A
*
*     RETURNS:   N/A
*
*******************************************************************/
iso_u8 IS_InputSignalCallback(struct InputString_S *pInputStringData) {

#if defined(CONFIG_CONNECT_WIFI)
	IS_InputSignalCallback_wifi_connect(pInputStringData);
#endif
	return (ISO_TRUE);
}
/*******************************************************************
*
*    FUNCTION: AUX_InputSignalCallback
*
* DESCRIPTION:
*
*  PARAMETERS: N/A
*
*     RETURNS:   N/A
*
*******************************************************************/
iso_u8 AUX_InputSignalCallback(AUX_InputSignalData_T * InputSignalData) {
	iso_u8 rVal = ISO_TRUE;
	struct ButtonActivation_S pButtonData;


	// Application may decide to lookup the object ID of the function assigned to this input
	// and then check the status data reported...
	//functionObjID = <maybe you have a lookup to get function object Id from (InputSignalData.inputObjectID)>;

	// Store the sprintf format string in HUGE ROM.
	//static SprinthfFormatChar_T format[] = "Pressed = %s, Status= %i\n";


	if (InputSignalData->objectIdOfAuxObject < FIRST_AUX)
	{
		return rVal;
	}



	pButtonData.objectIdOfButtonObject = CommandTranslate[InputSignalData->objectIdOfAuxObject - FIRST_AUX].objectIdOfButtonObject;
	pButtonData.wPara2 = (iso_u16)InputSignalData->value1;


	switch (InputSignalData->value1) {
	case AUX_PRESS_OFF:
		switch (InputSignalData_old_value1[InputSignalData->objectIdOfAuxObject - FIRST_AUX]) {
		case AUX_PRESS_OFF:
			//NOOP!
			break;
		default:
			pButtonData.keyActivationCode = BUTTON_STATE_RELEASED;
			BUTTON_InputSignalCallback(&pButtonData);
			break;
		}
		break;
	case AUX_PRESS_MOMENTARY:
		pButtonData.keyActivationCode = BUTTON_STATE_PRESSED;
		BUTTON_InputSignalCallback(&pButtonData);
		break;
	case AUX_PRESS_HELD:
		switch (InputSignalData_old_value1[InputSignalData->objectIdOfAuxObject - FIRST_AUX]) {
		case AUX_PRESS_OFF:
			pButtonData.keyActivationCode = BUTTON_STATE_PRESSED;
			BUTTON_InputSignalCallback(&pButtonData);
			break;
		default:
			pButtonData.keyActivationCode = BUTTON_STATE_HELD;
			BUTTON_InputSignalCallback(&pButtonData);
			break;
		}
		break;
	default:
		break;

	}

	InputSignalData_old_value1[InputSignalData->objectIdOfAuxObject - FIRST_AUX] = InputSignalData->value1;
	return rVal;
}




/******************************************************************************************/    
/*!                                                                               
   \brief       Receiving all messages of VT                                      
   \code                                                                               
   Callback function for responses, VT activation messages ...                                                                                            
      
    VT-Function:                Parameter of       Meaning:
                                ISOVT_MSG_STA_T:      
                                                   
    softkey activation:         wObjectID          key object ID                   
                                wPara1             parent object ID                
                                bPara              key number (hard coded)         
                                lValue             activation code (0, 1, 2, 3(Version 4)) see [1]       
    button activation:          wObjectID          button object ID                
                                wPara1             parent object ID                
                                bPara              button key number               
                                lValue             activation code (0, 1, 2, 3(Version 4)) see [1] 
    pointing event:             wPara1             X position in pixel             
                                wPara2             Y position in pixel  
                                bPara              touch state ( 0xFF for VT 3 and prior )              
    VT select input object:     wObjectID          input object ID                 
                                wPara1             Selected/Deselected
                                wPara2             Bitmask (Version 5 and later)
    VT esc                      wObjectID          ID of input field where aborted
                                iErrorCode         error code see ISO Doku.        
    VT change numeric value     wObjectID          ID of input object              
                                lValue             new value                       
    VT change active mask       wObjectID          momentan active mask            
                                iErrorCode         error code see ISO Doku.        
    VT change soft key mask     wObjectID          data or alarm mask object ID    
                                wPara1             soft key mask object ID         
                                iErrorCode         error code see ISO Doku         
    VT change string value      wObjectID          ID of String object             
                                bPara              Length of string                
                                pabVtData          Pointer to characters
    end_of_pool_transfer        iErrorCode         Error codes from Byte 2 ( 0 == no error)
                                wObjectID          Faulty object ID, Bytes 5,6
                                wPara1             Parent object ID, Bytes 3,4
                                bPara              Object Pool Error Codes (0 = no errors)
    ( Version 4 )                                  
    VT_onUserLayout_hideShow    wObjectID          Object ID of WM, DM, SKM, KG 
                                wPara2             Hide/show
                                wPara1             Object ID of second WM, DM, SKM, KG
                                bPara              Hide/show of second
    get_attribute_value         wObjectID          Object ID
                                bPara              AID
                                wPara1             Current value of attribute
                                iErrorCode         ErrorCode (see F.59)
    ( Version 3 )                                  
    preferred_assignment        wObjectID          Auxiliary function object if faulty
                                iErrorCode         Error code see ISO Doku. 
    auxiliary_assign_type_1, 2  wObjectID          Object ID auxiliary function    
                                wPara1             Object ID auxiliary input (or input number type 1) 
                                                   0xFFFF for unassign             
                                wPara2             Type of auxiliary incl. Attribute bits see [2] 
                                bPara              ISO_TRUE: Store as pref. assign, else not (only type 2) 
                                lValue             Bit 16 - 27: Manufacturer code,         
                                                   Bit  0 - 15 Model Identification code of auxiliary input 
                                                   (only type 2) 
    aux_input_status_type_1,2   wObjectID          Object ID Auxiliary function type          
                                wPara1             Input object ID (type 1 = input number )    
                                lValue             Value 1                                    
                                wPara2             Value 2                                    
                                iErrorCode         E_NO_ERR, E_CANMSG_MISSED (Alive of input)
    ( Version 5 )
    auxiliary_capabilities      bPara              Number of Auxiliary Units
                                pabVtData          Pointer to data ( Byte 3 ... )

                  [1] Timeout control of softkeys and buttons have to be done of application !
                  [2] Attribute bits are given to application as additional information
                  For getting the Auxiliary type application have to mask out it.
   \endcode
                                                                                  
  \param[in]       \wpp{pIsoMsgSta, const #ISOVT_MSG_STA_T *}                                                   
                    Pointer on received IS0 messages               
                                                                              
                                                                    
*/                                                                                
static void CbVtMessages( const ISOVT_MSG_STA_T * pIsoMsgSta )
{
   // when using pure ISO, corresponding part in AppRun must be inactivated
   OutputVtMessages( pIsoMsgSta );

   if (pIsoMsgSta->iErrorCode != E_NO_ERR)
   {  /* we have an error: <E_NO_ERR : E_CANMSG_MISSED (Timeout or TP abort); >E_NO_ERR: response error code  */
      /* Add here your error processing (cmd retry, ....) */
      return;
   }

   switch ( pIsoMsgSta->iVtFunction )
   {
   case VT_change_softkey_mask :
       // active soft key mask
       // ID (wObjectID) == active soft key mask
       break;
   case softkey_activation :
   case button_activation :
		BUTTON_InputSignalCallback((struct ButtonActivation_S *)(pIsoMsgSta));
           break;
	case aux_input_status_type_1:
	case auxiliary_input_status_type_2:
		/* Auxiliary_input_status received -> wObjectID means assigned auxiliary_function */
		AUX_InputSignalCallback((AUX_InputSignalData_T *)(pIsoMsgSta));
		break;
   case VT_change_numeric_value :
       /* Numeric value */
		IN_InputSignalCallback((struct InputNumber_S *)(pIsoMsgSta));
           break;
   case VT_change_string_value :
       /* pIsoMsgSta->bPara       Length of string */
       /* pIsoMsgSta->pabVtData   Pointer to data  */
	   IS_InputSignalCallback((struct InputString_S *)(pIsoMsgSta));
       break;
   case preferred_assignment:
       break;
   case auxiliary_assign_type_1 :
       //pIsoMsgSta->iErrorCode = 1;
       break;
   case auxiliary_assign_type_2 :
       /* Assignment is stored only in case of Byte 10, Bit 7 is zero (use as preferred assignment) */
       if ( pIsoMsgSta->bPara == ISO_TRUE )
       {
#if !defined(ISO_VTC_DISABLE_AUX_N) 
           /* Data of auxiliary assignment messages( remove all -> several calls ) */
           static iso_s16  iNumberOfFunctions = 0;
           static VT_AUXAPP_T asAuxAss[AUXINPUTMAX];
           /* Reading the complete actual assignment and storing this in a file or EE */
           /* Dont change the order */
           IsoAuxAssignmentRead( asAuxAss, &iNumberOfFunctions );

#ifndef USE_PREF_ASSIGN_CB
           /* Update number of assignments - 
              e. g. AuxInput switched off then on -> preferred assignment is (intern) repeated */
           IsoAuxAssignmentSet( asAuxAss, iNumberOfFunctions );
#endif
           /* Attention: */
           /* Remove assignments because of losing auxiliary devices have to handle separate */
           /* -> Store bit == ISO_FALSE in combination with unassignment must be ignored */
           
           /* Assignments as result of previos preferred assignment have Store bit = ISO_FALSE */
           /* -> Storage not necessary */
           
           IsoAuxWriteAssignToFile( asAuxAss, iNumberOfFunctions );  // Assignment -> File
#endif /* !defined(ISO_VTC_DISABLE_AUX_N) */
       }
       break;
   case get_attribute_value :
       switch ( pIsoMsgSta->wObjectID )
       {
       case 30000 :
           /* Manipulation necessary ?
              Compare picture width on VT with SCM graphics objects size */
           #ifdef _LAY14_
          {
             iso_s16 s16CfHndCl;
             s16CfHndCl = IsoGetVTStatusInfo(CF_HND);
             if (pIsoMsgSta->wPara1 != IsoSC_StatusInfoGet(s16CfHndCl, SCM_CAP_PIXELXAXIS))
             {
                AppSCOPManipulation();  /* Manipulate and store SCOP objects */
             }
          }
           #endif /* _LAY14_ */
           break;
       default:
           break;
       }
       break;
   case end_of_pool_transfer:
#ifdef ISO_DEBUG_ENABLED 
      if (pIsoMsgSta->iErrorCode != 0)
      { 
         iso_DebugPrint("VT - End Of Pool Transfer - Error: %2.2x ObjID: %4.4x Parent: %4.4x PoolError %2.2x Time: %8.4d \n",
            pIsoMsgSta->iErrorCode, pIsoMsgSta->wObjectID, pIsoMsgSta->wPara1, pIsoMsgSta->bPara, IsoClientsGetTimeMs());
      }
#endif /* ISO_DEBUG_ENABLED */
      break;
   default:
       break;
   }
}

/*! callback function for reading the preferred assignment

*/
static void CbAuxPrefAssignment(VT_AUXAPP_T asAuxAss[], iso_s16* ps16MaxNumberOfAssigns, ISO_USER_PARAM_T userParam )
{
   iso_s16 s16I, s16NumbOfPrefAssigns = 0;
   VT_AUXAPP_T asPrefAss[AUXINPUTMAX];

   /* Reading stored preferred assignment */
   s16NumbOfPrefAssigns = IsoAuxReadAssignOfFile( asPrefAss ); 

   for( s16I=0; s16I < s16NumbOfPrefAssigns; s16I++ )
   {
      asAuxAss[s16I] = asPrefAss[s16I];
   }
   *ps16MaxNumberOfAssigns = s16NumbOfPrefAssigns;
}  // CbAux


/*! Function which triggers ( forces ) the preferred assignment 

*/
void AppTriggerPreferredAssignment( )
{
#if !defined(ISO_VTC_DISABLE_AUX_N) 
   IsoAuxPrefAssignmentTrigger( );
#endif
}


iso_s16 NextVTButtonPressed(iso_u8 Buttonnumber)
{
#ifdef _LAY78_
   #define NUMBOFHAND   10

   if ( IsoWsSetMaskInst(s16CfHandleWs) == E_NO_ERR )
   {
       iso_s16 s16NumberOfVTs = 0, s16NumberAct = NUMBOFHAND, iI = 0;
       iso_s16 s16HndCurrentVT;
       iso_s16 as16HandList[NUMBOFHAND];
       iso_s16 s16NumberNextVT=NUMBOFHAND;
       // Determine list number of actual VT
       (void)IsoClientsReadListofExtHandles(virtual_terminal, NUMBOFHAND,
                                             as16HandList, &s16NumberOfVTs) ;

       s16HndCurrentVT = (iso_s16) IsoGetVTStatusInfo(VT_HND);       // Actual VT
       for ( iI = 0; iI < s16NumberOfVTs; iI++ )
       {
           if (s16HndCurrentVT == as16HandList[iI] )
           {
               s16NumberAct = iI;
               break;
           }
       }
       // Determine NAME of next VT
       if ( s16NumberAct != NUMBOFHAND )
       {
          ISO_CF_INFO_T sUserVT;
          s16NumberNextVT=Buttonnumber;

           iso_NmGetCfInfo( as16HandList[s16NumberNextVT], &sUserVT );
           IsoVTMultipleNextVT( &sUserVT.au8Name );

		   //TODO
		   //writeToEEPROM(PreferedVT, General_Settings.ptr_PreferedVTName);
           // NAME could be stored here in EEPROM
           // Application must go into safe state !!!
           // IsoVtContinue( );       // Continue after interrupting ISO (announcing) state machine
           // IsoVtContinue() - Not necessary with Version 8 because of Event "Safe State"
       }
   }
#endif
   return 0;
}

/* Reload of objects during normal work */
iso_s16 PoolReload( iso_u8 newPoolLanguage )
{
   iso_s16 iRet = E_NO_ERR;
   iso_bool PoolReload_State = ISO_FALSE;
   if ( IsoWsSetMaskInst(s16CfHandleWs) == E_NO_ERR )
   {
		switch (newPoolLanguage)
		{
		case German:
			break;

		default:
			break;
		}
		if (PoolReload_State)
       {

           /* Reload ranges */

           /* Manipulating these objects */

       }
       else
       {
           iRet = E_ERROR_INDI;
       }
   }
   return iRet;
}


void AppAlarmRelease( int Nummer_si )
{
#if defined(ISO_VTC_ENABLE_ALARM)
   switch ( Nummer_si )
   {
   case 1 :
       IsoAlarmCmd( &asAlarm[ALARM_INFO], AlEngage );
       IsoAlarmCmd( &asAlarm[ALARM_PROCESS], AlEngage );
       IsoAlarmCmd( &asAlarm[ALARM_EMER_INPUT_WRONG], AlEngage );
       IsoAlarmCmd( &asAlarm[ALARM_EMER_JOYTASTE], AlEngage );
       break;
   case 2 :
       IsoAlarmCmd( &asAlarm[ALARM_INFO], AlEngage );
       break;
   case 3 :
       IsoAlarmCmd( &asAlarm[ALARM_PROCESS], AlEngage );
       break;
   case 4 :
       IsoAlarmCmd( &asAlarm[ALARM_EMER_JOYTASTE], AlEngage );
       break;
   }
#endif /* defined(ISO_VTC_ENABLE_ALARM) */
}


/* Delete alarms in case of VT lost */
void AppAlarmDelete( void )
{
#if defined(ISO_VTC_ENABLE_ALARM)
   IsoAlarmCmd( &asAlarm[ALARM_EMER_INPUT_WRONG], AlDelete );
   IsoAlarmCmd( &asAlarm[ALARM_EMER_JOYTASTE], AlDelete );
   IsoAlarmCmd( &asAlarm[ALARM_PROCESS], AlDelete );
   IsoAlarmCmd( &asAlarm[ALARM_INFO], AlDelete );
#endif /* defined(ISO_VTC_ENABLE_ALARM) */
}

/* ************************************************************************ */
/*

*/
void Init_VI(void)
{
	//TODO!!
	//if Next VT happened, must do all things to reset. 
}


/*! \brief Macro for Flash and RAM access */
#define   ACCBYTE(ad, x)     ((ad)[(x)-1])


void SecondaryAdaptation(OBJTYP_e eObjectType, iso_u16 u16ObjectId, iso_u8 HUGE_C pau8Object[], iso_u32* pu32ObjectSize, ISO_USER_PARAM_T userParam)
{
	switch (eObjectType)
	{
	case TypPolygon:
		ACCBYTE(pau8Object, 4u)++; // width++ otherwise is cutted after Scaling
		ACCBYTE(pau8Object, 6u)++; // height++ otherwise is cutted after Scaling
		ACCBYTE(pau8Object, 12u) = 1; // Type of the Polygon = 1 = Non-Convex otherwise is COMPLEX because of Jetter Designer
		break;
	default:
		break;
	}
}
#endif /* defined(_LAY6_) && (!defined(WBZ_GRAPHIC_AUX_SAMPLE) || !defined(ISO_VTC_GRAPHIC_AUX)) */
/* ************************************************************************ */
