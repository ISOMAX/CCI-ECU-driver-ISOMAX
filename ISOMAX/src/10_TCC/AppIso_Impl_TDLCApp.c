/* ********************************************************************* */
/*!
 \file

 \brief          ISO Datalogger Client Demo Application

 \author         Erwin Hammerl
 \date           Created 11.05.2015
 \copyright      Wegscheider Hammerl Ingenieure Partnerschaft

 \par
 07.01.2015 - author E. Hammerl

 */
/* ********************************************************************* */

//#define ISO_DEBUG_ENABLED

#include "IsoDef.h"
#include "stdio.h"
#include "string.h"
#include "AppIso_Implement.h"
#include "DDI.h"
#include "DDIDesignator.h"
#include "VIEngine.h"

#include "MyProject1.iop.h"
#include "General_Settings.h"
#include "SpeedDistanceAreaHelper.h"

#if defined(_LAY10_) 

// Defines for task controller client
enum AppIso_Impl_TCApp_Object_ID {
	/** \brief
	 Device element Number
	 */
	DE_NUM_DEVICE,
	DE_NUM_CONN,
	DE_NUM_SECTION,

	/** \brief
	 Device element Object ID
	 */
	OBJID_DET_DEVICE,
	OBJID_DET_CONN,

	/** \brief
	 Device Process Data ObjectID
	 */
	OBJID_DPD_ACTUAL_WORK_STATE,
	OBJID_DPT_MAXIMUM_WORKING_WIDTH,
	OBJID_DPT_ACTUAL_WORKING_WIDTH,
	OBJID_DPT_OFFSET_X_BOOM,
	OBJID_DPT_OFFSET_Y_BOOM,

	OBJID_DPT_OFFSET_X_CONNECTOR,
	OBJID_DPT_OFFSET_Y_CONNECTOR,
	OBJID_DPT_CONNECTOR_TYPE,

	OBJID_DPD_ACTUAL_CULTURAL_PRACTICE,
	OBJID_DPD_LIFETIME_TOTAL_TIME,
	OBJID_DPD_LIFETIME_TOTAL_AREA,
	OBJID_DPD_LIFETIME_TOTAL_DISTANCE,

	/** \brief
	 Device Value Presentation ObjectID
	 */
	OBJID_DVP_ACTUAL_WORK_STATE,
	OBJID_DVP_UNIT_M,
	OBJID_DVP_UNIT_HOUR,
	OBJID_DVP_UNIT_SEC,
	OBJID_DVP_UNIT_HA

};

#define OBJID_DPD_DEFAULT_DDI					59999
#define OBJID_DVP_NO_UNIT						0xFFFF

// Local data and functions
static void AppDLC_DDOPSet(iso_u8 u8ClNu);
static void CbDlConnCtrl(const ISO_TCCBCONN_T *psTcCbConn);
static void CbDlExData(ISO_TCLINK_T*);
static void AppDLC_ReloadDDOs(iso_u8 u8ClNu);

static iso_u8 u8_TCCHa = TCCLIENT_UNKNOWN;
static iso_s16 s16TCCHandle = HANDLE_UNVALID; /* CF client handle*/
static iso_u16 u16TCprefBoottime = 10u;   // Boottime for first connecting
static iso_bool qNozzlesWorkState = ISO_FALSE;
static iso_bool qTotalsActive = ISO_FALSE;

/* Structure label and extended structure label */
static iso_u8 au8StructLabel[] = { '7', '7', '7', '7', '7', '7', '!' }; // 7 bytes
/* Extended structure label: Bytearray till first '/0' or max. 32 bytes */
static iso_u8 au8ExtStructLabel[] = { "Vers1.00 DLClient" };
/* Default Localisation label  */
static iso_u8 au8LocalLabel[] = { 'e', 'n', 0u, 0u, 0u, 0u, 0xFFu };
/* Actual Language command data -> update Localisation label */
static iso_u8 au8LCDatAct[7] = { 0xFFu, 0xFFu, 0u, 0u, 0u, 0u, 0xFFu };


void AppDLClientSet(iso_s16 s16CfHandle, iso_s16 s16CfSrvHandle) {
	// CF rpabName have to be a Working set master
	ISO_USER_PARAM_T userParamDefault = ISO_USER_PARAM_DEFAULT;
	iso_u8 u8ClNum;
	// Open TC client instance
	u8ClNum = IsoTC_InstCmd(s16CfHandle, userParamDefault, CbDlExData,
			CbDlConnCtrl, IsoDlOpenInst);
	if (u8ClNum != TCCLIENT_UNKNOWN) {
		ISO_CF_INFO_T sDLServDat;
		// Set user parameter and additional connection callback function
		// deprecated - IsoTC_InstallCbConnection(u8ClNum, CbTcConnCtrl);

		// The selected TC is set as preferred TC
		iso_NmGetCfInfo(s16CfSrvHandle, &sDLServDat);
		IsoTC_SetPrefDL(s16CfHandle, &sDLServDat.au8Name, u16TCprefBoottime);
		// Storage of handles where the TCC belonged to it
		u8_TCCHa = u8ClNum;
		s16TCCHandle = s16CfHandle;
	}
}

/***************************************************************************/
/*!
 \brief      Releasing DL client shutdown

 This function is called if "Key Switch state not Off" to  "Key Switch state Off"

 \param[in]      \wp{s16CfHandle, iso_s16}
 CF handle of TC client
 */
void AppDLClientLogOut(iso_s16 s16CfHandle) {
	ISO_USER_PARAM_T userParamDefault = ISO_USER_PARAM_DEFAULT;
	iso_u8 u8ClNum;
	u8ClNum = IsoTC_InstCmd(s16CfHandle, userParamDefault, 0, 0,
			IsoDlGetPointOfInst);
	if (u8ClNum != TCCLIENT_UNKNOWN) {
		IsoTC_InstCmd(s16CfHandle, userParamDefault, 0, 0, IsoDlLogOut); // IsoEvCmdSafeState is called
	}
}

static void AppDLC_DDOPSet(iso_u8 u8ClNu) {
	iso_u8 *pau8LC;
	/* In case of valid LC data use this as localisation label else default LC */
	if (IsoTC_ReadLCDataofConnection(u8ClNu, au8LCDatAct) == E_NO_ERR) {
		pau8LC = au8LCDatAct;
	} else {
		pau8LC = au8LocalLabel;  // Default LC
	}
	/* else use Default Language command data */

	// The descriptors are not adapted to the used language in this sample !!
	// Take care of order - first DEs then DPDs and DPTs 
	/* rpcDeviceDesignator,       = B
	 rpcDeviceSoftwareVersion,  = C
	 rpbSerialNumber,           = E
	 rpcDeviceStructureLabel,   = F
	 rpcDeviceLocalisationLabel = G
	 rpcExtendedStructureLabel  = H */
	//                                         B           C       E
	IsoTC_DeviceExt_Set(u8ClNu, "ISOMAX", "ISM", "01", au8StructLabel, pau8LC,
			au8ExtStructLabel);

	/* rwDeviceElementObjectID,   = B
	 reDeviceElementType,       = C
	 rpcDeviceElementDesignator = D
	 rwDeviceElementNumber,     = E
	 rwParentObjectID,          = F   */
	//                             B        C        D        E     F
	IsoTC_DeviceElement_Set(u8ClNu, OBJID_DET_DEVICE, de_device, "ISOMAX",
			DE_NUM_DEVICE, 0u);

	IsoTC_DeviceElement_Set(u8ClNu, OBJID_DET_CONN, de_connector,
			"Front Connector", DE_NUM_CONN, OBJID_DET_DEVICE);
	IsoTC_AddDPDObject(u8ClNu, OBJID_DPT_OFFSET_X_CONNECTOR); // Device Element Offset X
	IsoTC_AddDPTObject(u8ClNu, OBJID_DPT_OFFSET_Y_CONNECTOR); // Device Element Offset Y
	IsoTC_AddDPTObject(u8ClNu, OBJID_DPT_CONNECTOR_TYPE);
	IsoTC_DeviceElement_End(u8ClNu);

	IsoTC_AddDPDObject(u8ClNu, OBJID_DPD_ACTUAL_WORK_STATE);
	IsoTC_AddDPDObject(u8ClNu, OBJID_DPT_MAXIMUM_WORKING_WIDTH);
	IsoTC_AddDPDObject(u8ClNu, OBJID_DPT_ACTUAL_WORKING_WIDTH);
	IsoTC_AddDPDObject(u8ClNu, OBJID_DPT_OFFSET_X_BOOM); // Device Element Offset X
	IsoTC_AddDPTObject(u8ClNu, OBJID_DPT_OFFSET_Y_BOOM); // Device Element Offset Y

	IsoTC_AddDPDObject(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_TIME);
	IsoTC_AddDPDObject(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_AREA);
	IsoTC_AddDPDObject(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_DISTANCE);

	IsoTC_AddDPDObject(u8ClNu, OBJID_DPD_DEFAULT_DDI); // Request for default DDI

	IsoTC_DeviceElement_End(u8ClNu);

	/* rwProcessDataObjectID,       = A
	 rwProcessDataDDI,            = B
	 rbProcessDataProperty,       = C  - TC_PROP_DEFAULT_SET, TC_PROP_SETABLE
	 rbProcessDataTriggMethods,   = D  - trigger 1=time, 2=distance, 4=threshold, 8=on change, 16=total
	 rpcProcessDataDesignator,    = E
	 rwDeviceValuePresentObjectID = F   */
	//Device ISOMAX
	IsoTC_DeviceProcessData_Set(u8ClNu, OBJID_DPD_ACTUAL_WORK_STATE,     	// A
			DDI_ACTUAL_WORK_STATE,      									// B
			TC_PROP_DEFAULT_SET,                             				// C
			TC_TRIG_TIMEINTERVAL | TC_TRIG_ON_CHANGE, 						// D
			DDI_ACTUAL_WORK_STATE_DESIGNATOR, OBJID_DVP_ACTUAL_WORK_STATE); // E,  F

	IsoTC_DeviceProcessData_Set(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_TIME,		// A
			DDI_LIFETIME_EFFECTIVE_TOTAL_TIME,               				// B
			TC_PROP_DEFAULT_SET,                                     		// C
			TC_TRIG_TIMEINTERVAL | TC_TRIG_TOTAL,          					// D
			DDI_LIFETIME_EFFECTIVE_TOTAL_TIME_DESIGNATOR, OBJID_DVP_UNIT_HOUR); // E,  F

	IsoTC_DeviceProcessData_Set(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_AREA,		// A
			DDI_LIFETIME_TOTAL_AREA,               					// B
			TC_PROP_DEFAULT_SET,                                     		// C
			TC_TRIG_TIMEINTERVAL | TC_TRIG_TOTAL,          					// D
			DDI_LIFETIME_TOTAL_AREA_DESIGNATOR, OBJID_DVP_UNIT_HA);   	// E,  F

	IsoTC_DeviceProcessData_Set(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_DISTANCE,	// A
			DDI_LIFETIME_EFFECTIVE_TOTAL_DISTANCE,               			// B
			TC_PROP_DEFAULT_SET,                                     		// C
			TC_TRIG_TIMEINTERVAL | TC_TRIG_TOTAL,          					// D
			DDI_LIFETIME_EFFECTIVE_TOTAL_DISTANCE_DESIGNATOR, OBJID_DVP_UNIT_M); // E,  F

	IsoTC_DeviceProcessData_Set(u8ClNu, OBJID_DPD_ACTUAL_CULTURAL_PRACTICE,	// A
			DDI_ACTUAL_CULTURAL_PRACTICE,                      			// B
			TC_PROP_DEFAULT_SET,                                            // C
			TC_TRIG_TIMEINTERVAL | TC_TRIG_ON_CHANGE,                       // D
			DDI_ACTUAL_CULTURAL_PRACTICE_DESIGNATOR, OBJID_DVP_NO_UNIT);// E,  F

	IsoTC_DeviceProcessData_Set(u8ClNu,
	OBJID_DPD_DEFAULT_DDI,											// A
			DDI_REQUEST_DEFAULT_PROCESS_DATA,  // A,  B Request for default data
			0,                                               // C
			31,                           // D - all triggermethods see standard
			DDI_REQUEST_DEFAULT_PROCESS_DATA_DESIGNATOR, OBJID_DVP_NO_UNIT); // E,  F

	/* Setting Default trigger - necessary for all belonging to "Member of default set" */
	/* rwProcessDataObjectID
	 rbPdDeftTriggMethods
	 rlPdDeftValTimeInt
	 rlPdDeftValDistanceInt
	 rlPdDeftValThreshMini
	 rlPdDeftValThreshMaxi
	 rlPdDeftValThreshChan   */
	IsoTC_ProcessDataDefTrigger_Set(u8ClNu, OBJID_DPD_ACTUAL_WORK_STATE, // Object ID
			TC_TRIG_TIMEINTERVAL | TC_TRIG_ON_CHANGE,     // Used Triggermethods
			3000L, 1000L, 10L, 60L, 1L);                  // Values in ms, mm ..


	IsoTC_ProcessDataDefTrigger_Set(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_TIME, // Object ID
			TC_TRIG_TIMEINTERVAL | TC_TRIG_TOTAL,         // Used Triggermethods
			5000L, 1000L, 10L, 60L, 3L);                  // Values in ms, mm ..

	IsoTC_ProcessDataDefTrigger_Set(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_AREA, // Object ID
			TC_TRIG_TIMEINTERVAL | TC_TRIG_TOTAL,         // Used Triggermethods
			5000L, 1000L, 10L, 60L, 3L);                  // Values in ms, mm ..

	IsoTC_ProcessDataDefTrigger_Set(u8ClNu, OBJID_DPD_LIFETIME_TOTAL_DISTANCE, // Object ID
			TC_TRIG_TIMEINTERVAL | TC_TRIG_TOTAL,         // Used Triggermethods
			5000L, 1000L, 10L, 60L, 3L);                  // Values in ms, mm ..

	/* rwDevicePropertyObjectID,    = A
	 rwDevicePropertyDDI,         = B
	 rlDevicePropertyValue,       = C
	 rpcDevicePropertyDesignator, = D
	 rwDeviceValuePresentObjectID = E   */
	/*                               A      B               C         D              E         */

	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_CONNECTOR_TYPE,
			DDI_CONNECTOR_TYPE, 1, DDI_CONNECTOR_TYPE_DESIGNATOR,
			OBJID_DVP_NO_UNIT);
	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_MAXIMUM_WORKING_WIDTH,
			DDI_MAXIMUM_WORKING_WIDTH,
			3000,
			DDI_MAXIMUM_WORKING_WIDTH_DESIGNATOR, OBJID_DVP_UNIT_M);
	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_ACTUAL_WORKING_WIDTH,
			DDI_ACTUAL_WORKING_WIDTH,
			3000,
			DDI_ACTUAL_WORKING_WIDTH_DESIGNATOR, OBJID_DVP_UNIT_M);
	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_OFFSET_X_BOOM,
			DDI_DEVICE_ELEMENT_OFFSET_X,
			0, DDI_DEVICE_ELEMENT_OFFSET_X_DESIGNATOR,
			OBJID_DVP_UNIT_M);
	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_OFFSET_Y_BOOM,
			DDI_DEVICE_ELEMENT_OFFSET_Y,
			0, DDI_DEVICE_ELEMENT_OFFSET_Y_DESIGNATOR,
			OBJID_DVP_UNIT_M);
	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_OFFSET_X_CONNECTOR,
			DDI_DEVICE_ELEMENT_OFFSET_X,
			0, DDI_DEVICE_ELEMENT_OFFSET_X_DESIGNATOR,
			OBJID_DVP_UNIT_M);
	IsoTC_DeviceProperty_Set(u8ClNu, OBJID_DPT_OFFSET_Y_CONNECTOR,
			DDI_DEVICE_ELEMENT_OFFSET_Y,
			0, DDI_DEVICE_ELEMENT_OFFSET_Y_DESIGNATOR,
			OBJID_DVP_UNIT_M);
	/* rwDeviceValuePresentObjectID, = A
	 rlOffset,                     = B
	 rfScale,                      = C
	 rbNumberOfDecimals,           = D
	 rpcUnitDesignator             = E    */
	/*                                 		  A   						B         C        				D	  E         */
	IsoTC_DeviceValuePresent_Set(u8ClNu, OBJID_DVP_ACTUAL_WORK_STATE, 0,
			(iso_f32) 1, 0, "n.a.");
	IsoTC_DeviceValuePresent_Set(u8ClNu, OBJID_DVP_UNIT_HA, 0,
			(iso_f32) 0.0000001, 2, "ha"); //TODO !
	IsoTC_DeviceValuePresent_Set(u8ClNu, OBJID_DVP_UNIT_HOUR, 0,
			(iso_f32) 0.00027777777, 2, "h"); //0.00027777777 ist definitiv richtig !
	IsoTC_DeviceValuePresent_Set(u8ClNu, OBJID_DVP_UNIT_SEC, 0, 1, 0, "sec");
	IsoTC_DeviceValuePresent_Set(u8ClNu, OBJID_DVP_UNIT_M, 0, (iso_f32) 0.001,
			1, "m");

	// Calling IsoTcClosePool is not necessary anymore - deprecated with driver version 8
}

#if defined(WIN32) // Not intended usage in productive setup!
#define __huge 
#endif

/***************************************************************************/
/*!
 \brief       Callback for connection control

 \param[in]      \wp{psTcCbConn, ISO_TCCBCONN_T*}
 Pointer to TC link struct
 */
static void CbDlConnCtrl(const ISO_TCCBCONN_T *psTcCbConn) {
	ISO_TCVERS_T sVersDat;
	switch (psTcCbConn->eClEvent) {
	case IsoEvPrefServerNotAvailable: {
		iso_s16 s16Handles[5], s16Numb = 0, s16I, s16Re;
		ISOUSERAPP_T sTcInf;
		IsoClientsReadListofExtHandles(datalogger, 5, s16Handles, &s16Numb);
		// If there are DLs available select one
		for (s16I = 0; s16I < s16Numb; s16I++) {
			s16Re = iso_NmGetDataOfMember(s16Handles[0], &sTcInf);
			if ((s16Re == E_NO_ERR))
			// && (sTcInf.psName->wManufCode == ...) Select a TC
			{
				IsoTC_SetPrefDL(s16TCCHandle, sTcInf.pacIsoUserName,
						u16TCprefBoottime);
			}
		}
	}
		break;
	case IsoEvServerVersionAvailable:
		IsoTC_ReadVersionOfServer(psTcCbConn->u8ClNum, &sVersDat);
		/* NAME and boottime should be stored for next session as preferred TC/DL
		 - NAME of TC/DL can be determined with IsoTC_Read_TCHandle() and iso_NmGetCfInfo()
		 - sVersDat.u8Boottime of TC/DL ( >= version 3 else 0xFF )    */

		/* Own version message has to be adapted to Server version ( see Standard "connection management" ) */
		if (sVersDat.u8VersionNumber == TC_R2DIS) {
			IsoTC_SetVersionMsg(psTcCbConn->u8ClNum, TC_R2DIS, /* must be 3 here !!! */
			TC_SUPPORT_DOCU,
			TC_SUPPORT_OPT2, 1u, //u8NumberBoomsForSC,
					1u, //u8NumberSectionsForSC,
					1u  //u8NumberControlChannels
					);
		} else if (sVersDat.u8VersionNumber >= TC_R2IS) {
			IsoTC_SetVersionMsg(psTcCbConn->u8ClNum, TC_R2IS,
			TC_SUPPORT_DOCU,
			TC_SUPPORT_OPT2, 1u, //u8NumberBoomsForSC,
					1u, //u8NumberSectionsForSC,
					1u  //u8NumberControlChannels
					);
		} else { /* Version 1 or 2 -> we work with defaults ( Version 2 ) */
		}

		// e. g. sVersDat.u8NumberBoomsForSC for adjusting the client DDOP
		// Note: There is a TC which requests the version after first TC status ( not conform! )
		/* Own version message has to be adapted to Server version ( see Standard "connection management" ) */
		//Store for later usage
		break;
	case IsoEvLanguageCmdReceived:
		if (IsoTC_StatusInfoGet(psTcCbConn->u8ClNum, TCC_STATEOFANNOUNCING)
				== TcRunning) {
			// Language or metrics updated during runtime -> Reload updated objects and descriptors
			// Event during runtime only for version 4 or higher 
			IsoTC_ReloadDDObjects(u8_TCCHa, IsoClType_DontCare);
		}
		break;
	case IsoEvLoadObjects:
		// Driver requests Device description 
		AppDLC_DDOPSet(psTcCbConn->u8ClNum);
		break;
	case IsoEvActivated:
		// TCC/DLC successful logged in
		qNozzlesWorkState = ISO_TRUE;
		qTotalsActive = ISO_FALSE; // After unexpected shutdown during active task 1
		//IsoCmd_Attribute(EL_TOTALS_DATALOGGER_CONNECTED, AID_OE_FILL_ATT, FillAttributes_GREEN);
		break;
	case IsoEvDeactivated:
		// TCC/DLC successful logged out
		//IsoCmd_Attribute(EL_TOTALS_DATALOGGER_CONNECTED, AID_OE_FILL_ATT, FillAttributes_RED);
		break;
	case IsoEvCmdSafeState:
		//IsoCmd_Attribute(EL_TOTALS_DATALOGGER_CONNECTED, AID_OE_FILL_ATT, FillAttributes_RED);
		// Called if "Move to another TC" or "Unexpected shutdown" (TC server lost)
		break;
	case IsoEvTick:
		// Called every cyclic tick after login use for e. g. AppDLClientdoProcess();
		break;
	case IsoEvLoginAborted:

		break;
	case IsoEvReloadDDOs:
		AppDLC_ReloadDDOs(psTcCbConn->u8ClNum);
		break;
	case IsoEvReloadFinished:
		break;
	default:
		break;
	}

#ifdef ISO_DEBUG_ENABLED
	//AppTCC_LoginStateOutput(psTcCbConn->u8ClNum);
	switch (psTcCbConn->eClEvent) {
	case IsoEvServerVersionAvailable:
		iso_DebugPrint("DL - Event: DL Version: %d", sVersDat.u8VersionNumber);
		break;
		break;
	case IsoEvLanguageCmdReceived:
		iso_DebugPrint("DL - Event: LC available \n");
		break;
		break;
	case IsoEvLoadObjects:
		iso_DebugPrint("DL - Event: DD objects are loaded \n");
		break;
		break;
	case IsoEvActivated:
		iso_DebugPrint("DL - Event: is activated ( TC running ) \n");
		break;
		break;
	case IsoEvDeactivated:
		iso_DebugPrint("DL - Event: is deactivated \n");
		break;
		break;
	case IsoEvCmdSafeState:
		iso_DebugPrint("DL - Event: Safestate request to application \n");
		break;
	case IsoEvLoginAborted:
		iso_DebugPrint("DL - Event: Uncoverable error during login \n");

		break;
	case IsoEvReloadDDOs:
		iso_DebugPrint(
				"DL - Event: Reload started ( Changed DDIs to driver) \n");
		break;
	case IsoEvReloadFinished:
		iso_DebugPrint(
				"DL - Event: Reload finished ( Changed DDIs to driver) \n");
		break;
	default:
		break;
	}
#endif /* ISO_DEBUG_ENABLED */
}

/***************************************************************************/
/*!
 \brief       Application-Interface to the task controller client

 This function is called of the driver if sending a new process data is necessary
 or error codes are reveived.

 \code
 struct _tclink_
 {
 iso_u16        wDDObjID;             Object ID of device description object
 iso_u16        wDevElementNumb;      Requested element number of TC (only process data)
 iso_u16        wDDI;                 Data dictionary identifier (only process data)
 iso_s32       lValueNew;            Depends on ePDCmd:
 - IsoValueCommand:        New value for application
 - IsoRequestValueCommand: Application must set new value for TC
 ISO_TCAPP_e ePDCmd;               Request value command, value command or Response to command,
 IsoTaskStartStop ..
 };
 \endcode
 \param[in]      \wp{psTcLink, ISO_TCLINK_T*}
 Pointer to TC link struct

 \note  \par
 Following exceptions for data are available: \n
 \code
 ePDCmd = IsoTCVersionReceived ( deprecated - see connection callback )
 - wDDObjID:            Version of TC received of Version message
 - wDevElementNumb:     Boot time of TC in seconds received of Version message
 - wDDI:                Provided options of TC ( Low == Byte4, High == Byte5 )
 - lValueNew:           Number .. ( Low == Byte6, Second = Byte7, Third = Byte8 )
 ePDCmd = IsoTCSizeOfDD
 - lValueNew: Real size of the DD in bytes ( for development )
 ePDCmd = IsoTCPDACKReceived
 - lValueNew:           LSB: PD error code, Second iso_u8: PD command
 \endcode

 */
static void CbDlExData(ISO_TCLINK_T *psTcLink) {


	switch (psTcLink->ePDCmd) {
	case IsoTCVersionReceived:
		break;
	case IsoTaskStartStop:
		if (psTcLink->lValueNew == 1) {
			qTotalsActive = ISO_TRUE;
			//IsoCmd_Attribute(EL_TOTALS_DATALOGGER_ACTIVE, AID_OE_FILL_ATT, FillAttributes_GREEN);
		} else {
			qTotalsActive = ISO_FALSE;
			//IsoCmd_Attribute(EL_TOTALS_DATALOGGER_ACTIVE, AID_OE_FILL_ATT, FillAttributes_RED);
		}
		break;
	case IsoRequestValueCommand:
		// Actual values from machine for TC
		// Receiving TC_DDI_REQUEST_DEFAULT_PD is possible too but handling is automated
		// Requests are also responded if Task is not active
		switch (psTcLink->wDevElementNumb) {
		case DE_NUM_DEVICE:
			switch (psTcLink->wDDI) {
			case DDI_ACTUAL_WORK_STATE:
				psTcLink->lValueNew = 01; // = enabled / on;
				break;

			case DDI_LIFETIME_EFFECTIVE_TOTAL_TIME:
				psTcLink->lValueNew = speedDistance_get_pd274LifetimeEffectiveTotalTime_us();
				break;

			case DDI_LIFETIME_TOTAL_AREA:
				psTcLink->lValueNew = speedDistance_get_pd271LifetimeTotalArea_mm2();
				break;

			case DDI_LIFETIME_EFFECTIVE_TOTAL_DISTANCE:
				psTcLink->lValueNew = speedDistance_get_pd272LifetimeEffectiveTotalDistance_mm();
				break;

			case DDI_ACTUAL_CULTURAL_PRACTICE: //179 - Actual Cultural Practice
				psTcLink->lValueNew = 3; //3=Crop Protection
				break;

			case DDI_ACTUAL_VOLUME_PER_AREA_APPLICATION_RATE:
				psTcLink->lValueNew = (iso_s32) (5 * 10.0); //TODO
				break;

			case DDI_ACTUAL_WORKING_WIDTH:
				psTcLink->lValueNew = 3000;
				break;

			case DDI_MAXIMUM_WORKING_WIDTH:
				psTcLink->lValueNew = 3000;
				break;

			case DDI_DEVICE_ELEMENT_OFFSET_X:
				psTcLink->lValueNew = 0;
				break;

			case DDI_DEVICE_ELEMENT_OFFSET_Y:
				psTcLink->lValueNew = 0;
				break;

			default:
				break;
			}
			break;

		case DE_NUM_CONN:
			switch (psTcLink->wDDI) {
			case DDI_DEVICE_ELEMENT_OFFSET_X:
				psTcLink->lValueNew = 0;
				break;

			case DDI_DEVICE_ELEMENT_OFFSET_Y:
				psTcLink->lValueNew = 0;
				break;

			case DDI_CONNECTOR_TYPE:
				psTcLink->lValueNew = 0;
				break;

			default:
				break;
			}

			break;

		case DE_NUM_SECTION:
			switch (psTcLink->wDDI) {
			case DDI_DEVICE_ELEMENT_OFFSET_X:
				psTcLink->lValueNew = 0;
				break;

			case DDI_DEVICE_ELEMENT_OFFSET_Y:
				psTcLink->lValueNew = 0;
				break;

			case DDI_ACTUAL_WORKING_WIDTH:
				psTcLink->lValueNew = 3000;
				break;

			default:
				break;
			}

			break;
		}
		break;
	case IsoValueCommand:
	case IsoValueCommandAcknow:
		// Setpoint values from TC for machine
		switch (psTcLink->wDDObjID) {

// nothing to Set on a DL !

		default:
			break;
		}
		//no Break here !!!!
		/* "Set value and acknowledge" use value new as error code for PDACK:
		 no_Pd_error, err_PdNotConformDDIDefinition, err_PdOutsideOperationalRange viable
		 other error codes are handled intern ! */
		if (psTcLink->ePDCmd == IsoValueCommandAcknow) {
			psTcLink->lValueNew = no_Pd_error;
		}
		break; /* End ValueCommand */
	case IsoTCPDACKReceived:

		break;
	default:
		break;
	}
	//#define ISO_DEBUG_ENABLED 1
#ifdef ISO_DEBUG_ENABLED
#define iso_DebugTrace printf
	if (qDoReport == ISO_TRUE) {
		switch (psTcLink->ePDCmd) {
		case IsoRequestValueCommand:
			iso_DebugTrace("TC ReqCmd ElNr: %4x  DDI: %4x  Wert: %d \n",
					psTcLink->wDevElementNumb, psTcLink->wDDI,
					psTcLink->lValueNew);
			break;
		case IsoValueCommand:
			iso_DebugTrace("TC ValCmd ElNr: %4x  DDI: %4x  Wert: %d \n",
					psTcLink->wDevElementNumb, psTcLink->wDDI,
					psTcLink->lValueNew);
			break;
		case IsoResponseToCommand:
			iso_DebugTrace("TC-Work - Respo change design  Wert: %d\n",
					psTcLink->lValueNew);
			break;
		case IsoTaskStartStop:
			iso_DebugTrace("TC-Work - Task Start/Stop      Wert: %d\n",
					psTcLink->lValueNew);
			break;
		case IsoTCSizeOfDD:
			iso_DebugTrace("TC-Work - Size of DD           Wert: %d\n",
					psTcLink->lValueNew);
			break;
		case IsoTCVersionReceived:
			iso_DebugTrace("TC-Work - TC Version received  Wert: %d\n",
					psTcLink->lValueNew);
			break;
		case IsoTCPDACKReceived:
			iso_DebugTrace("TC-Work - PDACK of TC/DL       Wert: %d\n",
					psTcLink->lValueNew);
			break;
		default:
			break;
		}
	}
#endif
#undef ISO_DEBUG_ENABLED
}

static void AppDLC_ReloadDDOs(iso_u8 u8ClNu) {
	iso_u8 *pau8LC;
	// Only reloading of DVC- and DVP-objects are allowed
	if (IsoTC_ReadLCDataofConnection(u8ClNu, au8LCDatAct) == E_NO_ERR) {
		pau8LC = au8LCDatAct;
		// Analysing LC or metrics is not implemented in this sample ...
		/* rpcDeviceDesignator,       = B
		 rpcDeviceSoftwareVersion,  = C
		 rpbSerialNumber,           = E
		 rpcDeviceStructureLabel,   = F
		 rpcDeviceLocalisationLabel = G */
		//                           B           C       E      F            G
		IsoTC_DeviceExt_Set(u8ClNu, "ISOMAX", "ISM", "01", au8StructLabel,
				pau8LC, au8ExtStructLabel);

		/* rwDeviceValuePresentObjectID, = A
		 rlOffset,                     = B
		 rfScale,                      = C
		 rbNumberOfDecimals,           = D
		 rpcUnitDesignator             = E    */
		//                                    A    B         C        D        E
	}
}

void AppDLC_MoveToAnotherDL() {
	iso_s16 s16DLHandleCurrent = HANDLE_UNVALID;
	iso_s16 s16Handles[5], sNumb = 0;
	iso_u8 u8I;
	ISO_CF_INFO_T sDLServDat;

	s16DLHandleCurrent = IsoTC_Read_DLHandle(s16TCCHandle);
	IsoClientsReadListofExtHandles(datalogger, 5, s16Handles, &sNumb);
	for (u8I = 0; u8I < sNumb; u8I++) {
		if (s16Handles[u8I] != s16DLHandleCurrent) {
			iso_NmGetCfInfo(s16Handles[u8I], &sDLServDat);
			/* Connect with another TC */
			IsoTC_MoveToAnotherTCoDL(u8_TCCHa, &sDLServDat.au8Name, 5);
		}
	}
}

#endif

#undef ISO_DEBUG_ENABLED

