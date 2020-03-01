//**************************************************************************/
/*!
   \file           

   \brief          Basic layer test implementation 
  
   \author         Erwin Hammerl
   \date           Created 17.09.2007 

   \copyright      Wegscheider Hammerl Ingenieure Partnerschaft

   \par
   08.12.2011 - author E. Hammerl
   - Error output in IsoUserAnnounce()

*/
/* **************************  includes ********************************** */

#include <string.h>
#include "IsoDef.h"

#if defined(_LAY78_) && (!defined(WBZ_GRAPHIC_AUX_SAMPLE) || !defined(ISO_VTC_GRAPHIC_AUX))  /* compile only if _LAY78_ is defined */

#include "AppIso_Implement.h"
#include "AppIso_Diagnostic.h"
#include "AppHW.h"
#include "General_Settings.h"


#include "IsoStdTypes.h"
#include "VIEngine.h"
#include "MyProject1.iop.h"
#include "IsoCommandECHelper.h"
#include "SpeedDistanceAreaHelper.h"
#include "AppIso_Impl_NetDl.h"

#define F_TESTIMPLEMENT    0x80u    /**< Function of ISO user (must be different inside the ECU) */



//Additional SPN Files
#define SPN_COMPASSBEARING (165UL)
#define SPN_NAVIGATIONBASEDVEHICLESPEED (517UL)
#define SPN_ALTITUDE (583UL)
#define SPN_PITCH (580UL)
#define SPN_PITCH (580UL)

/* Network member handles */
iso_s16  m_CfHndImp = HANDLE_UNVALID; /**< CF handle of implement  */

/* ****************************** local data   *************************** */


/* Extern network member handles */
static iso_s16 s16NmHandTECU       = HANDLE_UNVALID;
static iso_s16 s16NmHandOwnedVT    = HANDLE_UNVALID;

/* PGN handles */
	   iso_s16 s16HaC1WhSpeedDis   = HANDLE_UNVALID;
	   iso_s16 s16HaC1GdSpeedDis   = HANDLE_UNVALID;
	   iso_s16 s16HaC1RearPTO  	   = HANDLE_UNVALID;
	   iso_s16 s16HaC1FrontPTO     = HANDLE_UNVALID;
	   iso_s16 s16HaC1RearHitch    = HANDLE_UNVALID;
	   iso_s16 s16HaC1FrontHitch   = HANDLE_UNVALID;
	   iso_s16 s16HaC1SelectedSpeed = HANDLE_UNVALID;
	   iso_s16 s16HaC1NavigationSpeed = HANDLE_UNVALID;
	   iso_s16 s16HaC1NMEA2000Speed = HANDLE_UNVALID;
static iso_s16 s16HaC1PriHitchSta  = HANDLE_UNVALID;
static iso_s16 s16HaC1MaintainPow  = HANDLE_UNVALID;
static iso_s16 s16HaC1TxCapMsg     = HANDLE_UNVALID;
static iso_s16 s16HaC1RxGenValEst  = HANDLE_UNVALID;
static iso_s16 s16HaC1RX_TDCmd     = HANDLE_UNVALID;
static iso_s16 s16HaC1TxNMEA1      = HANDLE_UNVALID;
static iso_s16 s16HaC1TXRequFacil  = HANDLE_UNVALID;
static iso_s16 s16HaC1RXFacility   = HANDLE_UNVALID;


/* Defines and variables for DM1/2 */
static iso_s16 s16HaC1TxDM1       = HANDLE_UNVALID;
static iso_s16 s16HaC1TxDM2       = HANDLE_UNVALID;

/* Application PGN data array */
static iso_u8  au8NameT[24] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 0xau, 0xbu,
                                0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 0xau, 0xbu};
static iso_u8  abNameT[8] = { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u };
#define DTC_ARRAYSIZE       20
static iso_s16 s16DM1NumbOfActDTCs = 0;
static iso_u8  au8DM2[DTC_ARRAYSIZE];   /* Array for DTC DM2 message */
static iso_bool q_Ignition = ISO_FALSE;


/* ****************************** function prototypes ***************************** */
static iso_s16   AppImp_Reset( void );
static void  AppImp_Logout( void );
static void  CbIsoNetwork     (const ISONETEVENT_T* psNetEv );
static void  CbIsoDataLink    ( ISO_TPREP_E, const ISO_TPINFO_T* );
static iso_s16  AppImpPGNInstallofPrimaryTECU( void );
static iso_s16  AppImpPGNInstall ( void );
static void  AppNetworkMemberList( void );
static void  CbPGNReceiveWheelbasedSpeed( const PGNDAT_T* psData );
static void  CbPGNReceiveGroundbasedSpeed( const PGNDAT_T* psData );
static void  CbPGNReceiveRearPTO( const PGNDAT_T* psData );
static void  CbPGNReceiveFrontPTO( const PGNDAT_T* psData );
static void  CbPGNReceiveRearHitch( const PGNDAT_T* psData );
static void  CbPGNReceiveFrontHitch( const PGNDAT_T* psData );
static void  CbPGNReceiveSelectedSpeed( const PGNDAT_T* psData );
static void  CbPGNReceiveNavigationSpeed( const PGNDAT_T* psData );
static void  CbPGNReceiveNMEA2000Speed( const PGNDAT_T* psData );
static void  CbPGNReceiveTimeDate( const PGNDAT_T* psData );
static void  CbPGNStopButton( const PGNDAT_T* psData );
static void  CbPGNReceiveFacilities( const PGNDAT_T* psData );
#if defined(_LAY6_) || defined(_LAY10_)
   static void  CbCfClientEvents( const ISOCFEVENT_T* psCfData );
#endif


void AppImpl_Ignition( iso_bool qIgnition )
{
   if( qIgnition == ISO_TRUE && q_Ignition == ISO_FALSE )
   {
      //Power on event
      //ImpCmdStartStop( 2 );     // CF test - Part 5 Test !
      AppImp_Reset( );
   }
   if( qIgnition == ISO_FALSE && q_Ignition == ISO_TRUE )
   { 
      //Power off event 
      AppImp_Logout( );
   }      
   q_Ignition = qIgnition;
}


static iso_s16 AppImp_Reset( )
{
   iso_u8  u8Instance = 0u;
   iso_s16 s16CfHandle;
   ISO_USER_PARAM_T userParam = ISO_USER_PARAM_DEFAULT;
	iso_u32  u32SeriNo = u32SeriNoGet();
   // work with unclaimed CFs
   // iso_NodeConfigSet( ISO_CAN_VT, CFG_PARAM_EXTERNAL_ADDR_CLAIM, CFG_VAL_OFF );
#ifdef TC_CONTROLSOURCE

   u8Instance = 1u;
#endif
   iso_NmSetName( ISO_TRUE,        /* Selfconfigurable */
                  2,               /* Industry group */
                  5,               /* Device class */
                  0,               /* Device class instance */
                  F_TESTIMPLEMENT, /* Function */
                  1048,             /* Manufacturer code for Agrivation UG*/
                  u32SeriNo,               /* Identity number (Serial number) */
                  u8Instance,     /* Function instance */
                  0,               /* ECU instance */
                  &abNameT );       /* NAME - return value */

   s16CfHandle = iso_BaseMemberAdd(ISO_CAN_VT,
		   	   	   	General_Settings.u8_SAImp,
                    &abNameT,
                    working_set_master,
                    userParam,
                    CbIsoNetwork,
                    CbIsoDataLink
                  );
  
   if (s16CfHandle < E_NO_ERR)
   {  /* error */
      s16CfHandle = HANDLE_UNVALID;
   }

   // Remember handle for TCC CbCfClientEvents() callback
   m_CfHndImp = s16CfHandle;
   #if defined(_LAY6_) || defined(_LAY10_)
   // Create working set for VT and TC client
   IsoSetWorkingSetMaster(s16CfHandle);
   // Start/Lost control of server/implements - must be called after IsoSetWorkingSetMaster() !
   IsoCFControlCbSet(s16CfHandle, userParam, CbCfClientEvents );
   #endif

   #ifdef _LAY6_
     AppVTClientSet(s16CfHandle);    // Init Virtual terminal client
   #endif
   #ifdef _LAY14_  /* Only in combination with _LAY6_ */
     AppSCClientSet(s16CfHandle);    // Init Sequenz control client 
   #endif 
   return 0;
}



static void AppImp_Logout( )
{
   ISO_CF_INFO_T sUserDa;
   iso_s16 iRet = E_NO_ERR;

   iRet = iso_NmGetCfInfo(m_CfHndImp, &sUserDa );
   #ifdef _LAY6_
    if ( iRet == E_NO_ERR && sUserDa.au8Name ) AppVTClientLogOut( );
   #endif
   #ifdef _LAY10_
    AppTCClientLogOut(m_CfHndImp);
    AppDLClientLogOut(m_CfHndImp);
   #endif
   #ifdef _LAY14_
     AppSCClientLogOut(m_CfHndImp);
   #endif
   if( s16HaC1WhSpeedDis  != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1WhSpeedDis );
   if( s16HaC1PriHitchSta != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1PriHitchSta );
   if( s16HaC1MaintainPow != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1MaintainPow );
   if( s16HaC1TxCapMsg    != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1TxCapMsg );
   if( s16HaC1RxGenValEst != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1RxGenValEst );
   if( s16HaC1RX_TDCmd    != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1RX_TDCmd );
   if( s16HaC1TxNMEA1     != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1TxNMEA1 );
   if( s16HaC1TxDM1       != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1TxDM1 );
   if( s16HaC1TxDM2       != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1TxDM2 );
   if( s16HaC1TXRequFacil != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1TXRequFacil );
   if( s16HaC1RXFacility  != HANDLE_UNVALID ) iso_AlPgnDeactivate( s16HaC1RXFacility );
   
   // Waiting that all clients are completely shut down
   /* VT client sends a Delete object pool in case of logout 
      -> iso_NmMemberLoggedOut() should be called after receiving the response  */
   //iso_NmMemberLoggedOut( s16NmHandImp1 );
   //s16NmHandImp1    = HANDLE_UNVALID;
   //s16NmHandTECU    = HANDLE_UNVALID;
   //s16NmHandOwnedVT = HANDLE_UNVALID;
}


//prototype!
extern void doStoreSA(void);

static void CbIsoNetwork( const ISONETEVENT_T* psNetEv )
{
   switch( psNetEv->eMemberRefer )
   {
      case thisMember:
         switch( psNetEv->eNetEvent )
         {
             case Isonet_MemberActive: 
                 /* Only called at first MemberNew or after MemberLoggedOut/MemberNew */
                 m_CfHndImp = psNetEv->s16Handle;
                 General_Settings.u8_SAImp = psNetEv->u8SAMember;  // SA should be stored and used in VT
                 doStoreSA();
                 #ifdef _NMEA_
                 //AppImpl2NMEA( );  // Sending and Receiving of FPP
                 #endif
                 // work with unlogged CFs
                 //AppPGNTestUnloggedCFs( );
			     //AppPGNPart12Demo( );
			     installPart12PGN(CFTypeIsInvalid, m_CfHndImp);
                 break;
             case Isonet_MemberNewAddress:  
                 m_CfHndImp = psNetEv->s16Handle;
                 General_Settings.u8_SAImp = psNetEv->u8SAMember;   /* After address conflict use/store new SA */
                 doStoreSA();
                 break;
             case Isonet_MemberAddressConflict:
             case Isonet_MemberInactive:
                 m_CfHndImp = HANDLE_UNVALID;  /* iso_NmMemberLoggedOut() -> Cb */
                 break;
             case Isonet_MemberAddressViolation:  
                 /* todo Part 5 - 4.4.4.3: set diagnostic trouble code with SPN=2000 + SA and FMI=31 */
                 break;
             default: break;
         }
         break;
      case network:
         switch( psNetEv->eNetEvent )
         {
            case Isonet_MemberActive: 
               {
                    iso_s16 s16NmHandTemp = HANDLE_UNVALID, s16Re;
                    ISOUSERAPP_T sUserInfo;
                    s16NmHandTemp = iso_NmGetMemHandleofSA(ISO_CAN_VT, psNetEv->u8SAMember);
                    s16Re = iso_NmGetDataOfMember( s16NmHandTemp, &sUserInfo );
                    if (s16Re == E_NO_ERR)
                    {
                       // Storing handle of Primary TECU
                       if (sUserInfo.eIsoUserFunct == tractor_ecu)
                       {
                    	   if(sUserInfo.psName->bFunctionInst == 0u)
                    	   {
                               s16NmHandTECU = s16NmHandTemp;
                               // Extern member are only reported after thisMember
                               AppImpPGNInstallofPrimaryTECU();
                    	   }
                    	   else if(s16NmHandTECU == HANDLE_UNVALID)
                    	   {
                               s16NmHandTECU = s16NmHandTemp;
                               // Extern member are only reported after thisMember
                               AppImpPGNInstallofPrimaryTECU();
                    	   }
                       }
                       if (sUserInfo.eIsoUserFunct == virtual_terminal
                          && sUserInfo.psName->bFunctionInst == 0u)
                       {
                          s16NmHandOwnedVT = s16NmHandTemp;
                       }
                       if (sUserInfo.eIsoUserFunct == tractor_ecu)
                       {
                          // Extern member are only reported after thisMember
                          AppImpPGNInstall();
                          if (s16HaC1TXRequFacil != HANDLE_UNVALID) iso_AlPgnTxSend(s16HaC1TXRequFacil);
                       }
                    }
                    AppNetworkMemberList( );
               }
               break;
            case Isonet_MemberNewAddress:
            case Isonet_MemberAddressConflict: break;
            case Isonet_MemberInactive:  break;
            default: break;
         }
         break;
      default: break;
   }
   OutputNetworkEvents( psNetEv );
}


#if defined(_LAY6_) || defined(_LAY10_)
/***************************************************************************/
/*!
   \brief       Application-Interface to the client-server control
   
   This function is called if an external server or client is started or lost on the network
   or if a server has sent a Language command.
   Alive control works with the cyclic messages ( status or maintenance messages ).

   \param[in]      \wp{psCfData, ISOCFEVENT_T*}  
                     Pointer to CF struct 

   \par
   Languages can be read with IsoClServ_ReadLCOfServer()
*/
static void CbCfClientEvents( const ISOCFEVENT_T* psCfData )
{

   #ifdef _LAY10_
   // New Taskcontroller found -> Start TC client  
   if (  ( psCfData->eIsoUserFunct == task_controller )
      && ( psCfData->eCFEvent == IsoCF_Active )  )
   {  
      AppTCClientSet(m_CfHndImp, psCfData->s16Handle );
   }
   // Datalogger found -> Start DL client  
   if (  ( psCfData->eIsoUserFunct == datalogger )
      && ( psCfData->eCFEvent == IsoCF_Active )  )
   {
      AppDLClientSet(m_CfHndImp, psCfData->s16Handle );
   }
   #endif
   if (psCfData->eCFEvent != IsoCF_LCReceived)
   {
#ifdef ISO_DEBUG_ENABLED
	  iso_s16 s16Handles[5], sNumb = 0;
      IsoClientsReadListofExtHandles(task_controller, 5, s16Handles, &sNumb);
      iso_DebugTrace("Number of TC server:     %3d \n", sNumb);
      IsoClientsReadListofExtHandles(virtual_terminal, 5, s16Handles, &sNumb);
      iso_DebugTrace("Number of VT server:     %3d \n", sNumb);
      IsoClientsReadListofExtHandles(auxiliary, 5, s16Handles, &sNumb);
      iso_DebugTrace("Number of AuxInputDevs:  %3d \n", sNumb);
      IsoClientsReadListofExtHandles(working_set_master, 5, s16Handles, &sNumb);
      iso_DebugTrace("Number of WorkSetMaster: %3d \n", sNumb);
#endif /* ISO_DEBUG_ENABLED */
      OutputCFEvents(psCfData);   // Show 
   }
}
#endif //(_LAY6_) || (_LAY10_)



static void CbIsoDataLink( ISO_TPREP_E eDataTransStatus, const ISO_TPINFO_T* psTpInfo )
{
   DataLinkOutput( eDataTransStatus, psTpInfo );
   switch ( eDataTransStatus )
   {
   case record_request :
      /* NACK/ACK of requests need to be handled manual like in this example
       -> following if() must be part of every application */
      if ( psTpInfo->s16HndIntern != HANDLE_GLOBAL )
      {
       switch ( psTpInfo->dwPGN )
       {
         case PGN_DIAGNOSTIC_DATA_CLEAR:
            // DM3: Application has to erase trouble codes here and then respond with
            iso_SpnDMResetDTC( au8DM2, DTC_ARRAYSIZE, &s16DM1NumbOfActDTCs );
            (void)iso_DlPgnAcknowledge( ACK, PGN_DIAGNOSTIC_DATA_CLEAR, 0xFFu, 0xFFu,
                                        psTpInfo->s16HndIntern, psTpInfo->s16HndPartner );
            break;
         default:
            if ( psTpInfo->s16HndPartner != HANDLE_UNVALID )
            {
               (void)iso_DlPgnAcknowledge(NACK, psTpInfo->dwPGN, 0xFFu, 0xFFu,
                                       psTpInfo->s16HndIntern, psTpInfo->s16HndPartner);
            }
            break;
        }
      }
      break; //ereq
   case record_finish :
      /* Receiving PGNs which are installed and dont use a own callback function */
      switch ( psTpInfo->dwPGN )
      {
      case PGN_GROUND_BASED_SPEED :  
       iso_CoreKeySwitchStateRead( ISO_CAN_VT );  // for breakpoint
       break;
      case 0xEF00 :          
       iso_CoreKeySwitchStateRead( ISO_CAN_VT );  // for breakpoint
       break;
      case PGN_GENPURPOSE_VALVE_ESTIMFLOW:
       iso_CoreKeySwitchStateRead( ISO_CAN_VT );  // for breakpoint
       break;
      default:
       break;
      }
      break;
   case send_finished :
      break;
   case datatrans_abort :
      /* Abort of data transport - possible actions like repeating request, ..   */
      break;
   default:
      break;
   }
}


static iso_s16 AppImpPGNInstallofPrimaryTECU( )
{
   ISO_USER_PARAM_T userParam = ISO_USER_PARAM_DEFAULT;

   // Wheel based speed must be sent of Primary TECU
   /* Example wheel based speed with own callback */
   s16HaC1WhSpeedDis  = iso_AlPgnRxNew( m_CfHndImp,
                                      PGN_WHEEL_BASED_SPEED,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveWheelbasedSpeed );
   /* Installing second PGN callback for wheel based speed */
                      iso_AlPgnRxNew( m_CfHndImp,
                                      PGN_WHEEL_BASED_SPEED,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, speedDistance_CbPGNReceiveWheelbasedSpeed );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINESPEED, 1u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINEDISTANCE, 3u, 1u, 32u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINEDIRECTION, 8u, 1u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_KEYSWITCHSTATE, 8u, 3u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_MAXI_TIMEOFTRACPOWER, 7u, 1u, 8u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_OPERATOR_DIREC_REVERSED, 8u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1WhSpeedDis, SPN_START_STOP_STATE, 8u, 5u, 8u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1WhSpeedDis );


   s16HaC1GdSpeedDis  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_GROUND_BASED_SPEED,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveGroundbasedSpeed );
   s16HaC1GdSpeedDis  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_GROUND_BASED_SPEED,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, speedDistance_CbPGNReceiveGroundbasedSpeed );

   iso_SpnDefineSpn( s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINESPEED, 1u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINEDISTANCE, 3u, 1u, 32u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINEDIRECT, 8u, 1u, 2u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1GdSpeedDis );


   s16HaC1RearPTO  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_REAR_PTO,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveRearPTO );

   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_OUTP_SHAFT_SPEED, 1u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_OUTP_SH_SPEED_SET, 3u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_ENGAGEMENT, 5u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_MODE, 5u, 5u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_ECONOMY_MODE , 5u, 3u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_ENGAGEMENT_REQ, 5u, 1u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_MODE_REQ_STATUS, 6u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_ECONOMY_MODE_REQ_STAT, 6u, 5u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_SH_SPEED_LIMIT_STAT, 6u, 2u, 3u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearPTO, SPN_REAR_PTO_EXIT_REASON_CODE, 7u, 1u, 8u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1RearPTO );


   s16HaC1FrontPTO  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_FRONT_PTO,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveFrontPTO );

   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_OUTP_SHAFT_SPEED, 1u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_OUTP_SH_SPEED_SET, 3u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_ENGAGEMENT, 5u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_MODE, 5u, 5u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_ECONOMY_MODE , 5u, 3u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_ENGAGEMENT_REQ, 5u, 1u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_MODE_REQ_STATUS, 6u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_ECONOMY_MODE_REQ_STAT, 6u, 5u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_SH_SPEED_LIMIT_STAT, 6u, 2u, 3u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontPTO, SPN_FRONT_PTO_EXIT_REASON_CODE, 7u, 1u, 8u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1FrontPTO );


   s16HaC1RearHitch  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_REAR_HITCH,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveRearHitch );

   iso_SpnDefineSpn( s16HaC1RearHitch, SPN_REAR_HITCH_POSITION, 1u, 1u, 8u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearHitch, SPN_REAR_HITCH_WORK_INDICATION, 2u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearHitch, SPN_REAR_HITCH_POS_LIMIT_STATUS, 2u, 4u, 3u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearHitch, SPN_REAR_HITCH_NOMINAL_LOWER_LINK_FORCE, 3u, 1u, 8u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearHitch, SPN_REAR_HITCH_DRAFT, 4u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1RearHitch, SPN_REAR_HITCH_EXIT_REASON_CODE, 6u, 1u, 8u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1RearHitch );


   s16HaC1FrontHitch  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_FRONT_HITCH,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveFrontHitch );

   iso_SpnDefineSpn( s16HaC1FrontHitch, SPN_FRONT_HITCH_POSITION, 1u, 1u, 8u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontHitch, SPN_FRONT_HITCH_WORK_INDICATION, 2u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontHitch, SPN_FRONT_HITCH_POS_LIMIT_STATUS, 2u, 4u, 3u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontHitch, SPN_FRONT_HITCH_NOMINAL_LOWER_LINK_FORCE, 3u, 1u, 8u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontHitch, SPN_FRONT_HITCH_DRAFT, 4u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1FrontHitch, SPN_FRONT_HITCH_EXIT_REASON_CODE, 6u, 1u, 8u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1FrontHitch );


   s16HaC1SelectedSpeed  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_FRONT_HITCH,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveSelectedSpeed );
   s16HaC1SelectedSpeed  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_FRONT_HITCH,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, speedDistance_CbPGNReceiveMachineSelectedSpeed );


   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINESPEED, 1u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINEDISTANCE, 3u, 1u, 32u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINE_EXIT_REASON_CODE, 7u, 1u, 8u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINEDIRECTION, 8u, 1u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINESOURCE, 8u, 3u, 3u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINELIMITSTATUS, 8u, 6u, 3u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1SelectedSpeed );


   s16HaC1NavigationSpeed  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_VEHICLE_DIRECTION_SPEED,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveNavigationSpeed );
   s16HaC1NavigationSpeed  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_VEHICLE_DIRECTION_SPEED,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, speedDistance_CbPGNReceiveGPSSpeed );


   iso_SpnDefineSpn( s16HaC1NavigationSpeed, SPN_COMPASSBEARING, 1u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1NavigationSpeed, SPN_NAVIGATIONBASEDVEHICLESPEED, 3u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1NavigationSpeed, SPN_ALTITUDE, 5u, 1u, 16u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1NavigationSpeed, SPN_PITCH, 7u, 1u, 16u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1NavigationSpeed );


   s16HaC1NMEA2000Speed  = iso_AlPgnRxNew( m_CfHndImp,
		   	   	   	   	   	   	   	  PGN_COG_SOG_RAPID_UPDATE,
                                      s16NmHandTECU,
                                      8u, 0, 3, 200, userParam, CbPGNReceiveNMEA2000Speed );

   iso_SpnDefineSpn( s16HaC1SelectedSpeed, SPN_SELECTEDMACHINESPEED, 5u, 1u, 16u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1SelectedSpeed );

   // Maintain power is sent to TECU in order to get 2sec more power
   s16HaC1MaintainPow = iso_AlPgnTxNew( m_CfHndImp,
                                      PGN_MAINTAIN_POWER,
                                      s16NmHandTECU,
                                      8u, 0, 3, REPRATE_INACTIVE, userParam, 0 );
   iso_SpnDefineSpn( s16HaC1MaintainPow, SPN_MAINTAIN_ECUPOWER, 1u, 7u, 2u, SpnValStandard );
   iso_SpnDefineSpn( s16HaC1MaintainPow, SPN_MAINTAIN_ACTUATORPOWER, 1u, 5u, 2u, SpnValStandard );
   iso_AlPgnActivate( s16HaC1MaintainPow );
   return 0;
}


static iso_s16 AppImpPGNInstall( )
{
   static iso_u8 au8ReqFaciltiy[8] = { 0x3F, 0xFC, 0x7E, 0, 0, 0, 0, 0 };
   ISO_USER_PARAM_T userParam = ISO_USER_PARAM_DEFAULT;

   s16HaC1PriHitchSta = iso_AlPgnRxNew( m_CfHndImp,
                                      PGN_REAR_HITCH,
                                      ISO_GLOBAL_ADDRESS,
                                      8u, 0, 3, 200, userParam, 0 );
   //iso_AlPgnRxRequestCyclic( s16HaC1PriHitchSta, 500 );       /* Activation included */
   iso_AlPgnActivate( s16HaC1PriHitchSta );

   // PDU1 sended message
   s16HaC1TxCapMsg   = iso_AlPgnTxNew( m_CfHndImp,
                                     PGN_CAB_MESSAGE1,
                                     ISO_GLOBAL_ADDRESS,
                                     8u, 0, 3, 500u, userParam, 0 );
   //iso_AlPgnActivate( s16HaC1TxCapMsg );
   iso_AlPgnTxSetLimitsReqForRepet( s16HaC1TxCapMsg, 100, 3000 );

   /* PDU 1 > 12 Bytes Test */
   s16HaC1RxGenValEst = iso_AlPgnRxNew( m_CfHndImp,
                                     PGN_GENPURPOSE_VALVE_ESTIMFLOW,
                                     ISO_GLOBAL_ADDRESS,
                                     12u, au8NameT, 3, 500, userParam, 0 );
   iso_AlPgnActivate( s16HaC1RxGenValEst );
   //iso_AlPgnRxRequestCyclic( s16HaC1RxGenValEst, 500 );       /* Activation included */

   s16HaC1TXRequFacil = iso_AlPgnTxNew( m_CfHndImp,
                                      PGN_REQUIRED_TRACTOR_FACILITIES,
                                      ISO_GLOBAL_ADDRESS,
                                      8u, au8ReqFaciltiy, 3u, REPRATE_INACTIVE, userParam, 0 );
   iso_AlPgnActivate( s16HaC1TXRequFacil );  

   /* Tractor facilities Msg - Receive */
   s16HaC1RXFacility  = iso_AlPgnRxNew( m_CfHndImp,
                                      PGN_TRACTOR_FACILITIES_RESPONSE,
                                      ISO_GLOBAL_ADDRESS,
                                      8u, 0, 3, REPRATE_INACTIVE, userParam, CbPGNReceiveFacilities );
   iso_AlPgnActivate( s16HaC1RXFacility );
   return 0;
}

void AppActivatePGNsForVT( )
{
   iso_s16 s16HaC1StopButton = HANDLE_UNVALID;
   iso_s16 s16HaC1TxFlex = HANDLE_UNVALID;
   ISO_USER_PARAM_T userParam = ISO_USER_PARAM_DEFAULT;

   s16HaC1RX_TDCmd  = iso_AlPgnRxNew( m_CfHndImp,
                                    PGN_TIME_DATA,
                                    s16NmHandOwnedVT,
                                    8u, 0, 6, 1000, userParam, CbPGNReceiveTimeDate );
   //iso_AlPgnRxRequestCyclic( s16HaC1RX_TDCmd, 1000 );  
   iso_AlPgnActivate( s16HaC1RX_TDCmd ); 

   /* Request of STOP Button */
   s16HaC1StopButton = iso_AlPgnRxNew( m_CfHndImp,
                                     PGN_STOP_BUTTON,
                                     ISO_GLOBAL_ADDRESS,
                                     8, 0, 3, 2000, userParam, CbPGNStopButton );
   iso_AlPgnActivate( s16HaC1StopButton );

   iso_u8 au8NameFlex[8] = { 0x02, 0xFD, 0, 0xE8, 0x03, 0xFF, 0xFF, 0xFF }; 
   s16HaC1TxFlex    = iso_AlPgnTxNew( m_CfHndImp,
                                     PGN_REQ_FOR_REP_RATE,
                                     ISO_GLOBAL_ADDRESS,
                                     8, au8NameFlex, 6, REPRATE_INACTIVE, userParam, 0 );
   iso_AlPgnActivate( s16HaC1TxFlex );
   iso_AlPgnTxSend(s16HaC1TxFlex);
}


static void CbPGNStopButton( const PGNDAT_T* psData )
{
   if ( psData->qTimedOut == ISO_FALSE && m_CfHndImp != HANDLE_UNVALID )
   {
      static iso_u8 u8Byte8 = 1u;
      if (u8Byte8 != psData->pau8Data[7])
      {
#ifdef ISO_DEBUG_ENABLED
         iso_DebugTrace( "StopButton data changed: %3d \n", psData->pau8Data[7] );
#endif
         u8Byte8 = psData->pau8Data[7];
      }
   }
}


/* Test of functions for network member data monitoring */
static void AppNetworkMemberList( )
{
    #define HANDARRSIZE1     10
    #define HANDARRSIZE2      5
    iso_s16 as16HandleList[HANDARRSIZE1], as16HandleListTractor[HANDARRSIZE2];
    iso_s16 s16NumberExternMembers = 0, s16NumberInternMembers = 0, s16I = 0;
    ISO_CF_INFO_T sNmData;

    /* Get a list of all extern network members */
    iso_NmReadListOfExternHandles(ISO_CAN_VT, HANDARRSIZE1, as16HandleList, &s16NumberExternMembers );
    for ( s16I = 0; s16I < s16NumberExternMembers; s16I++ )
    {
       iso_NmGetCfInfo( as16HandleList[s16I], &sNmData );
    }
    /* Get a list of all intern network members */
    iso_NmReadListOfInternHandles(ISO_CAN_VT, HANDARRSIZE1, as16HandleList, &s16NumberInternMembers );
    for ( s16I = 0; s16I < s16NumberInternMembers; s16I++ )
    {
       iso_NmGetCfInfo( as16HandleList[s16I], &sNmData );
    }
    /* Get a list of all TECUs */
    iso_NmReadListOfExtHandWithFunc(ISO_CAN_VT, F_TRACTOR_ECU, HANDARRSIZE2,
                                     as16HandleListTractor, &s16NumberExternMembers );
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
iso_u32 u32WheelBasedSpeed = 0uL;
iso_u32 u32WheelBasedDistance = 0uL;

/* Callbackfunction for Wheel-based speed and distance */
static void CbPGNReceiveWheelbasedSpeed( const PGNDAT_T* psData )
{
   if ( psData->qTimedOut == ISO_FALSE )
   {
	   iso_SpnDataRead( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINESPEED, &u32WheelBasedSpeed );
	   iso_SpnDataRead( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINEDISTANCE, &u32WheelBasedDistance );

#ifdef ISO_DEBUG_ENABLED
     iso_DebugPrint("Wheel based Distance: %d Speed: %d Time: %8.4d\n", u32WheelBasedSpeed, u32WheelBasedDistance, IsoClientsGetTimeMs());
#endif /* ISO_DEBUG_ENABLED */

     #ifdef _LAY10_
     IsoTC_SetDistance( u32WheelBasedDistance );  /* Giving distance to TC client */
     #endif
   }
   else
   {
     #ifdef _LAY10_
     IsoTC_SetDistance( ISO_NOT_AVAILABLE_DM );
     #ifdef ISO_DEBUG_ENABLED
     iso_DebugPrint("AL - Time out - Wheel based speed not received  Time: %8.4d\n", IsoClientsGetTimeMs());
     #endif /* ISO_DEBUG_ENABLED */
     #endif /*  _LAY10_ */
   }
}
iso_u32 u32GroundBasedSpeed = 0uL;
iso_u32 u32GroundBasedDistance = 0uL;
/* Callbackfunction for Ground-based speed and distance */
static void CbPGNReceiveGroundbasedSpeed(const PGNDAT_T *psData)
{
	if (psData->qTimedOut == ISO_FALSE)
	{
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINESPEED, &u32GroundBasedSpeed);
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINEDISTANCE, &u32GroundBasedDistance);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Ground based Distance: %d Speed: %d Time: %8.4d\n", u32GroundBasedSpeed, u32GroundBasedDistance, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32RearPTOSpeed = 0uL;
static void  CbPGNReceiveRearPTO( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{

		iso_SpnDataRead(s16HaC1RearPTO, SPN_REAR_PTO_OUTP_SHAFT_SPEED, &u32RearPTOSpeed);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Rear PTO Speed: %d Time: %8.4d\n", u32RearPTOSpeed, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32FrontPTOSpeed = 0uL;
static void  CbPGNReceiveFrontPTO( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{

		iso_SpnDataRead(s16HaC1FrontPTO, SPN_FRONT_PTO_OUTP_SHAFT_SPEED, &u32FrontPTOSpeed);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Front PTO Speed: %d Time: %8.4d\n", u32FrontPTOSpeed, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32RearHitchPosition = 0uL;
static void  CbPGNReceiveRearHitch( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{

		iso_SpnDataRead(s16HaC1RearHitch, SPN_REAR_HITCH_POSITION, &u32RearHitchPosition);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Rear Hitch Position: %d Time: %8.4d\n", u32RearHitchPosition, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32FrontHitchPosition = 0uL;
static void  CbPGNReceiveFrontHitch( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{

		iso_SpnDataRead(s16HaC1FrontHitch, SPN_FRONT_HITCH_POSITION, &u32FrontHitchPosition);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Front Hitch Position: %d Time: %8.4d\n", u32FrontHitchPosition, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32SelectedSpeed = 0uL;
iso_u32 u32SelectedDistance = 0uL;
static void  CbPGNReceiveSelectedSpeed( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_SELECTEDMACHINESPEED, &u32SelectedSpeed);

		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_SELECTEDMACHINEDISTANCE, &u32SelectedDistance);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Machine selected Distance: %d Speed: %d Time: %8.4d\n", u32SelectedSpeed, u32SelectedDistance, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32NavigationSpeed = 0uL;
static void  CbPGNReceiveNavigationSpeed( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{

		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_SELECTEDMACHINESPEED, &u32NavigationSpeed);

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("Navigation based Speed: %d Time: %8.4d\n", u32SelectedSpeed, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}
iso_u32 u32NMEA2000Speed = 0uL;
static void  CbPGNReceiveNMEA2000Speed( const PGNDAT_T* psData )
{
	if (psData->qTimedOut == ISO_FALSE)
	{

	     iso_SpnDataReadCom( 32,  16, psData->pau8Data, &u32NMEA2000Speed );

		#ifdef ISO_DEBUG_ENABLED
		iso_DebugPrint("NMEA 2000 Speed: %d Time: %8.4d\n", u32SelectedSpeed, IsoClientsGetTimeMs());
		#endif /* ISO_DEBUG_ENABLED */
	}
}


/* Callbackfunction for time date */
static void CbPGNReceiveTimeDate( const PGNDAT_T* psData )
{
   iso_u32 u32DatVal = 0uL;

   if ( psData->qTimedOut == ISO_FALSE )
   {
     iso_SpnDataReadCom( 16, 32, psData->pau8Data, &u32DatVal );
#ifdef ISO_DEBUG_ENABLED
     iso_DebugTrace( "PGN received: %5x  %2x\n", psData->u32PGN, psData->pau8Data[0] );
#endif
   }
}


/* Callbackfunction for Tractor facility message */
static void CbPGNReceiveFacilities( const PGNDAT_T* psData )
{
   // Facilities available of the TECUs of the network
   static iso_u8 au8TECUFacilities[8];
   iso_s8  s8L;
   for( s8L = 0; s8L < 8; s8L++ )
   {
      au8TECUFacilities[s8L] |= psData->pau8Data[s8L];
   }
}



#endif /* _LAY78_ is defined */
