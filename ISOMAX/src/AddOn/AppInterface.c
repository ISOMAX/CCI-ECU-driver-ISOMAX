//#include "stdafx.h"
#ifndef DOXNOTSHOW

#ifndef __AFXWIN_H__
#define NO_MFC /* Do not use Windows functions .... */
#endif 

#include "IsoDef.h"



/* includes for GetSizeOfTypes() calculation - not needed for an application  */
#ifndef NO_MFC
#ifdef _NMEA_
    #include "Core/IsoDataM.h"
#endif /* _NMEA_ */
#include "Core/IsoDrv.h"
#include "IsoClientsApi.h"
#include "IsoVtcApi.h"
#include "IsoSccApi.h"
#include "IsoTccApi.h"

#include "CShared/UtST.h"    
#include "CShared/IsoAliveCtrl.h"
#include "VTC/IsoVtUt.h"
#include "CShared/IsoWoSe.h"
#include "CShared/IsoClServ.h"
#include "VTC/IsoVt.h"
#include "VTC/IsoVtcPoolService.h"
#include "SQCC/IsoSCLogin.h"
#include "SQCC/IsoSCWork.h"
#include "SQCC/IsoSCFund.h"
#include "SQCC/IsoSCCtrl.h"
#include "TCC/IsoTcConn.h"
#include "TCC/IsoTcPd.h"
#include "IsoMnApi.h"
#include "IsoPgnDef.h"
#include "IsoMnNiuApi.h"
#include "Base/IsoMnAl.h"
#ifdef _LAY4_
   #include "NIUS/IsoMnNld.h" /* for size of */
#endif
#include "IsoVtsApi.h"
#endif /* not defined NO_MFC */

#include "AppMain.h"
#include "AppIso_Implement.h"
#include "VIEngine.h"
#include "General_Settings.h"

#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <stdarg.h>

//#include <iostream>
//#include <sstream>

#ifdef ISO_DEBUG_ENABLED
static const  iso_char* NMUserFuncString(ISO_USERFUNC_e eIsoUserFunct);
#endif /*ISO_DEBUG_ENABLED */

const iso_char * VTSublistTextout(iso_u8 bByte1);

//extern "C"
//{
iso_u8  abPoolBuffer[];
iso_u16  NumberObjects_glw = 0;
//}

iso_u8  abPoolBuffer[2000000];

iso_bool qShowDataLinkMsg = ISO_FALSE; 


#endif // DOXNOTSHOW

/*  CbErrorOutp() moved to AppIso_Output.c*/

#ifndef DOXNOTSHOW

void DataLinkOutput( ISO_TPREP_E eDataTransStatus, const ISO_TPINFO_T* psTpInfo )
{
    if ( qShowDataLinkMsg )
    {
#ifdef ISO_DEBUG_ENABLED
       const iso_char* cpchTpStatus;
       iso_s32  s32Time;

       s32Time = IsoDrvGetTimeMs();

       switch (eDataTransStatus)
       {
       case record_start:         cpchTpStatus = "RX start"; break;            /* An extern ECU has started a TP */
       case record_firstdata:     cpchTpStatus = "RX first data"; break;       /* An extern ECU has send the first data message (7 bytes, TP ...)  */
       case record_finish:        cpchTpStatus = "RX finish"; break;           /* An extern ECU has finished sending data (8 bytes, TP ...)  */
       case record_request:       cpchTpStatus = "record request"; break;      /* An extern ECU has sent an request - only Multi node API */
       case send_buffed:          cpchTpStatus = "TX buffered"; break;         /* An intern ECU has called IsoOpenTP successfully, data buffered */
       case send_start_OK:        cpchTpStatus = "TX start"; break;            /* An intern ECU has started a transport successfully */
       case send_packet_request:  cpchTpStatus = "TX packet request"; break;   /* The transport module is ready to send the next data packet ( 7 Bytes ) - Direct transfer only  */
       case send_finished:        cpchTpStatus = "TX finished"; break;         /* An intern ECU has finished a transport successfully */
       case datatrans_abort:      cpchTpStatus = "ABORT"; break;               /* A transport is aborted */
       case tp_pause:             cpchTpStatus = "PAUSE"; break;               /* Transport stopped (by the extern ECU) */
       case pgn_missed:           cpchTpStatus = "PGN missed"; break;          /* Cyclic PGN is missed */
       case address_conflict:     cpchTpStatus = "address conflict"; break;    /* Address conflict occurred */
       default:                   cpchTpStatus = "n/a"; break;
       };

       iso_DebugPrint("PGN: %6.6x CF INT: %2.2x (%4.4x)  EXT: %2.2x (%4.4x)  %-14.17s Size: %8.1d  Time: %8.4d \n",
          psTpInfo->dwPGN, psTpInfo->u8SAIntern, psTpInfo->s16HndIntern,
          psTpInfo->u8SAPartner, psTpInfo->s16HndPartner, cpchTpStatus, psTpInfo->dwNumberofBytes, s32Time);
#endif /* ISO_DEBUG_ENABLED */
    }
}

void OutputNetworkEvents( const ISONETEVENT_T* psNmEvent)
{
#ifdef ISO_DEBUG_ENABLED
    const iso_char    *pchRev, *pchEv;
    iso_s32  s32Time;

    s32Time = iso_BaseGetTimeMs();

    switch (psNmEvent->eMemberRefer)
    {
    case intern:    pchRev = "Intern"; break;
    case network:   pchRev = "Extern"; break;
    case thisMember:pchRev = "This  "; break;
    default:        pchRev = "Notdef"; break;
    }

    switch (psNmEvent->eNetEvent)
    {
    case Isonet_MemberActive:           pchEv = "Active  "; break;
    case Isonet_MemberAddressConflict:  pchEv = "Conflict"; break;
    case Isonet_MemberInactive:         pchEv = "Inactive"; break;
    case Isonet_MemberNewAddress:       pchEv = "New Addr"; break;
    case Isonet_MemberAddressViolation: pchEv = "AddrViol"; break;
#if defined(ISO_NM_MSG_MONITOR)
    case Isonet_MemberTimeout:          pchEv = "Timeout "; break;
    case Isonet_MemberResume:           pchEv = "Resume  "; break;
#endif /* #if defined(ISO_NM_MSG_MONITOR) */
    default:                            pchEv = "Notdef  "; break;
    }

#if 0
    /* simple output */
    iso_DebugPrint("NE: %-6s %-8s CF %2.2x (%4.4x)  Time: %8.4d \n",
       pchRev, pchEv, psNmEvent->u8SAMember, psNmEvent->s16Handle, s32Time);
#else
    {  /* extended output */
       const  iso_char   *pchFunc;
       ISO_CF_INFO_T     sCfInfo;

       (void)iso_NmGetCfInfo(psNmEvent->s16Handle, &sCfInfo); /* only single threaded ... */
       pchFunc = NMUserFuncString(sCfInfo.eIsoUserFunct);

       iso_DebugPrint("NE CH %1u: %-6s %-8s CF %2.2x (%4.4x) Time: %8.4d \n         NAME: %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x  %-10s (UFnc:%2.2d)\n",
          sCfInfo.u8CanNode, pchRev, pchEv, psNmEvent->u8SAMember, psNmEvent->s16Handle, s32Time,
          sCfInfo.au8Name[0], sCfInfo.au8Name[1], sCfInfo.au8Name[2], sCfInfo.au8Name[3],
          sCfInfo.au8Name[4], sCfInfo.au8Name[5], sCfInfo.au8Name[6], sCfInfo.au8Name[7], pchFunc, sCfInfo.eIsoUserFunct);
    }
#endif

#endif /* ISO_DEBUG_ENABLED */
}

void OutputCFEvents(const ISOCFEVENT_T* psCfData )
{
#ifdef ISO_MODULE_CLIENTS /* VTC TCC FSC ... */
#ifdef ISO_DEBUG_ENABLED
   const  iso_char   *pchFunc, *pchEv ;
   ISOUSERAPP_T      sCFDat;
   iso_s32           s32Time;

   s32Time = IsoClientsGetTimeMs();
   pchFunc = NMUserFuncString(psCfData->eIsoUserFunct);

   switch (psCfData->eCFEvent)
   {
   case IsoCF_Active:          pchEv = "CF Active ";  break;
   case IsoCF_Lost:            pchEv = "CF Lost   ";  break;
   case IsoCF_LCReceived:      pchEv = "LC Received"; break;
   default:                    pchEv = "Unknown   ";  break;
   }
   
   (void)iso_NmGetDataOfMember(psCfData->s16Handle, &sCFDat);

   iso_DebugPrint("CL - CF: %-10s Event: %-10s SA: %2.2x (%4.4x)  Time: %8.4d \n",
      pchFunc, pchEv, sCFDat.u8IsoSourceAdd, psCfData->s16Handle, s32Time);

#endif /* ISO_DEBUG_ENABLED */
#endif /* ISO_MODULE_CLIENTS */
}

void OutputNetworkMemberList()
{  
#ifdef ISO_DEBUG_ENABLED
   iso_u16 u16CanNbr;

   #define HANDARRSIZE1     30
   
   iso_DebugPrint( " \n" );

   for( u16CanNbr = 0; u16CanNbr < ISO_CAN_NODES; u16CanNbr++)
   {
      iso_s16 as16HandleList[HANDARRSIZE1], s16NumberInternMembers = 0;

      /* intern CFs */
      iso_NmReadListOfInternHandles( u16CanNbr, HANDARRSIZE1, as16HandleList, &s16NumberInternMembers );
        
      for( int iI = 0; iI < s16NumberInternMembers; iI++)
      {
         ISOUSERAPP_T sUserInfo; 
         const iso_char *pchUserFunc, *pchStatus;

         iso_NmGetDataOfMember( as16HandleList[iI], &sUserInfo );
         pchUserFunc = NMUserFuncString(sUserInfo.eIsoUserFunct);

         if (sUserInfo.eIsoAnnState == isouser_OK)
            pchStatus = " OK";
         else
            pchStatus = " NOK";

         iso_DebugPrint("B%.2d: %-18.20s SA: %2.2x (%4.4x)  in  %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %3s \n",
            u16CanNbr, pchUserFunc, sUserInfo.u8IsoSourceAdd, as16HandleList[iI],
            (*sUserInfo.pacIsoUserName)[7], (*sUserInfo.pacIsoUserName)[6], (*sUserInfo.pacIsoUserName)[5], (*sUserInfo.pacIsoUserName)[4],
            (*sUserInfo.pacIsoUserName)[3], (*sUserInfo.pacIsoUserName)[2], (*sUserInfo.pacIsoUserName)[1], (*sUserInfo.pacIsoUserName)[0], pchStatus);
            
      }  

      /* extern CFs */
      iso_s16 s16NumberExternMembers = 0;
      iso_NmReadListOfExternHandles( u16CanNbr, HANDARRSIZE1, as16HandleList, &s16NumberExternMembers );

      for( int iI = 0; iI < s16NumberExternMembers; iI++)
      {
         ISOUSERAPP_T   sUserInfo;
         const iso_char *pchUserFunc, *pchStatus;

         iso_NmGetDataOfMember( as16HandleList[iI], &sUserInfo );
         pchUserFunc = NMUserFuncString(sUserInfo.eIsoUserFunct);
          
         if (sUserInfo.eIsoAnnState == isouser_OK)
            pchStatus = " OK";
         else
            pchStatus = " NOK";

         iso_DebugPrint("B%.2d: %-18.20s SA: %2.2x (%4.4x)  ex  %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %3s \n", 
            u16CanNbr, pchUserFunc, sUserInfo.u8IsoSourceAdd, as16HandleList[iI],
            (*sUserInfo.pacIsoUserName)[7], (*sUserInfo.pacIsoUserName)[6], (*sUserInfo.pacIsoUserName)[5], (*sUserInfo.pacIsoUserName)[4],
            (*sUserInfo.pacIsoUserName)[3], (*sUserInfo.pacIsoUserName)[2], (*sUserInfo.pacIsoUserName)[1], (*sUserInfo.pacIsoUserName)[0], pchStatus);

      } /*END extern for */
   } /* END CAN Node for */
    
   iso_DebugPrint( " \n" );
#endif /*ISO_DEBUG_ENABLED */
}

#ifdef _LAY6_

void OutputVtMessages( const ISOVT_MSG_STA_T* pIsoMsgSta )
{ 
#ifdef ISO_DEBUG_ENABLED
    iso_s32 s32Time;
    const iso_char *pchStatus;

    s32Time = IsoClientsGetTimeMs();

    if (pIsoMsgSta->iErrorCode == E_NO_ERR)
       pchStatus = "OK   ";
    else if (pIsoMsgSta->iErrorCode == E_CANMSG_MISSED)
       pchStatus = "TIMEOUT";
    else
       pchStatus = "ERROR";

    switch ( pIsoMsgSta->iVtFunction )
    {
        case softkey_activation:
           iso_DebugPrint("SOFTKEY ACTIVATION: 0x%4.4x   %5d   %10d   Time: %8.4d\n", pIsoMsgSta->wObjectID, pIsoMsgSta->bPara, pIsoMsgSta->lValue, s32Time);
           break;
        case auxiliary_assign_type_1:
           iso_DebugPrint("AUX TYP 1 ASSIGN:   0x%4.4x   %5d   Time: %8.4d\n", pIsoMsgSta->wObjectID, pIsoMsgSta->wPara1, s32Time);
           break;
        case auxiliary_assign_type_2:
           {
              iso_u8 u8I;
              iso_DebugPrint("AUX TYP 2 ASSIGN:   0x%4.4x   %5d   Time: %8.4d Aux Unit: ", pIsoMsgSta->wObjectID, pIsoMsgSta->wPara1, s32Time);
              for (u8I = 0u; u8I < 8u; u8I++) {
                 iso_DebugPrint("%2.2X", pIsoMsgSta->pabVtData[u8I]);
              }
              iso_DebugPrint("\n");
           }
           break;
        case aux_input_status_type_1:
           iso_DebugPrint("AUX TYP 1 INPUT/FKT: 0x%4.4x   %10d   %s   Time: %8.4d\n", pIsoMsgSta->wObjectID, pIsoMsgSta->lValue, pchStatus, s32Time);
           break;
        case auxiliary_input_status_type_2:
           iso_DebugPrint("AUX TYP 2 INPUT/FKT: 0x%4.4x   %10d   %s   Time: %8.4d\n", pIsoMsgSta->wObjectID, pIsoMsgSta->lValue, pchStatus, s32Time);
           break;
        case vtc_pool_aux_input_type_2:
           iso_DebugPrint("PoolScan AUX INPUT TYP 2: 0x%4.4x Index: %4d Type: %4d   %s   Time: %8.4d\n", pIsoMsgSta->wObjectID, pIsoMsgSta->wPara1, pIsoMsgSta->wPara2, pchStatus, s32Time);
           break;
        case aux_pre_input_status_enable:
           iso_DebugPrint("AUX INPUT STATUS pre enable : 0x%4.4x  Enable: %4d   %s   Time: %8.4d\n", pIsoMsgSta->wObjectID, pIsoMsgSta->bPara, pchStatus, s32Time);
           break;
        case aux_learn_mode_changed:
           iso_DebugPrint("VT Status AUX Learn mode changed : bPara: %4d   Time: %8.4d\n", pIsoMsgSta->bPara, s32Time);
           break;
        default:
            {
               const iso_char *pchCmd;
               pchCmd = VTSublistTextout((iso_u8)pIsoMsgSta->iVtFunction);
               if (pIsoMsgSta->iErrorCode != E_NO_ERR)
               {
                  iso_DebugPrint("ERROR CommandResp:  %s   %10d   %s   Time: %8.4d\n", pchCmd, pIsoMsgSta->wPara1, pchStatus, s32Time);
               }
            }
            break;
    }
#endif /* Debug end */
}


uint16_t IsoAuxReadAssignOfFile(VT_AUXAPP_T asAuxAss[]) {

	nvs_handle my_handle;
	size_t mlenght = 8;
	size_t* length = &mlenght;
	printf("\n");
	printf("Opening Non-Volatile Storage (NVS) handle... IsoAuxReadAssignOfFile ");
	//TODO !!!!
	esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
	printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
	uint16_t iCnt = 0;

	err = nvs_get_u16(my_handle, key_iNumberOfAssigns, &iCnt);
	printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
	switch (iCnt) {

	case 4:
		err = nvs_get_u16(my_handle, key_asAuxAss_3_wManuCode,				&asAuxAss[3].wManuCode);
		err = nvs_get_u16(my_handle, key_asAuxAss_3_wModelIdentCode,		&asAuxAss[3].wModelIdentCode);
		err = nvs_get_u8 (my_handle, key_asAuxAss_3_bFuncAttribute,			&asAuxAss[3].bFuncAttribute);
																		     asAuxAss[3].eAuxType = (VTAUXTYP_e) (asAuxAss[3].bFuncAttribute & 0x1Fu);
		err = nvs_get_u16(my_handle, key_asAuxAss_3_wObjID_Fun,				&asAuxAss[3].wObjID_Fun);
		err = nvs_get_u16(my_handle, key_asAuxAss_3_wObjID_Input,			&asAuxAss[3].wObjID_Input);
		err = nvs_get_blob(my_handle, key_asAuxAss_3_baAuxName,				&asAuxAss[3].baAuxName, length);
		/* fall through */
		/* no break */
	case 3:
		err = nvs_get_u16(my_handle, key_asAuxAss_2_wManuCode,				&asAuxAss[2].wManuCode);
		err = nvs_get_u16(my_handle, key_asAuxAss_2_wModelIdentCode,		&asAuxAss[2].wModelIdentCode);
		err = nvs_get_u8 (my_handle, key_asAuxAss_2_bFuncAttribute,			&asAuxAss[2].bFuncAttribute);
																		     asAuxAss[2].eAuxType = (VTAUXTYP_e) (asAuxAss[2].bFuncAttribute & 0x1Fu);
		err = nvs_get_u16(my_handle, key_asAuxAss_2_wObjID_Fun,				&asAuxAss[2].wObjID_Fun);
		err = nvs_get_u16(my_handle, key_asAuxAss_2_wObjID_Input,			&asAuxAss[2].wObjID_Input);
		err = nvs_get_blob(my_handle, key_asAuxAss_2_baAuxName,				&asAuxAss[2].baAuxName, length);
		/* fall through */
		/* no break */
	case 2:
		err = nvs_get_u16(my_handle, key_asAuxAss_1_wManuCode,				&asAuxAss[1].wManuCode);
		err = nvs_get_u16(my_handle, key_asAuxAss_1_wModelIdentCode,		&asAuxAss[1].wModelIdentCode);
		err = nvs_get_u8 (my_handle, key_asAuxAss_1_bFuncAttribute,			&asAuxAss[1].bFuncAttribute);
																		     asAuxAss[1].eAuxType = (VTAUXTYP_e) (asAuxAss[1].bFuncAttribute & 0x1Fu);
		err = nvs_get_u16(my_handle, key_asAuxAss_1_wObjID_Fun,				&asAuxAss[1].wObjID_Fun);
		err = nvs_get_u16(my_handle, key_asAuxAss_1_wObjID_Input,			&asAuxAss[1].wObjID_Input);
		err = nvs_get_blob(my_handle, key_asAuxAss_1_baAuxName,				&asAuxAss[1].baAuxName, length);
		/* fall through */
		/* no break */
	case 1:
		err = nvs_get_u16(my_handle, key_asAuxAss_0_wManuCode,				&asAuxAss[0].wManuCode);
		err = nvs_get_u16(my_handle, key_asAuxAss_0_wModelIdentCode,		&asAuxAss[0].wModelIdentCode);
		err = nvs_get_u8 (my_handle, key_asAuxAss_0_bFuncAttribute,			&asAuxAss[0].bFuncAttribute);
																		     asAuxAss[0].eAuxType = (VTAUXTYP_e) (asAuxAss[0].bFuncAttribute & 0x1Fu);
		err = nvs_get_u16(my_handle, key_asAuxAss_0_wObjID_Fun,				&asAuxAss[0].wObjID_Fun);
		err = nvs_get_u16(my_handle, key_asAuxAss_0_wObjID_Input,			&asAuxAss[0].wObjID_Input);
		err = nvs_get_blob(my_handle, key_asAuxAss_0_baAuxName,				&asAuxAss[0].baAuxName, length);
		/* fall through */
		/* no break */
	case 0:
	default:
		break;
	}
	// Close
	nvs_close(my_handle);
	return (iCnt);
}

int IsoAuxWriteAssignToFile(VT_AUXAPP_T asAuxAss[], int iNumberOfAssigns)
{

	esp_err_t err;
	// Open
	printf("\n");
	printf("Opening Non-Volatile Storage (NVS) handle... IsoAuxWriteAssignToFile ");
	nvs_handle my_handle;
	err = nvs_open("storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");
		err = nvs_set_u16(my_handle, key_iNumberOfAssigns, iNumberOfAssigns);
		switch (iNumberOfAssigns) {

		case 4:
			err = nvs_set_u16(my_handle, key_asAuxAss_3_wManuCode, 		 asAuxAss[3].wManuCode);
			err = nvs_set_u16(my_handle, key_asAuxAss_3_wModelIdentCode, asAuxAss[3].wModelIdentCode);
			err = nvs_set_u8 (my_handle, key_asAuxAss_3_bFuncAttribute,  asAuxAss[3].bFuncAttribute);
			err = nvs_set_u16(my_handle, key_asAuxAss_3_wObjID_Fun, 	 asAuxAss[3].wObjID_Fun);
			err = nvs_set_u16(my_handle, key_asAuxAss_3_wObjID_Input, 	 asAuxAss[3].wObjID_Input);
			err = nvs_set_blob(my_handle, key_asAuxAss_3_baAuxName, 	 asAuxAss[3].baAuxName,8);
			/* fall through */
			/* no break */
		case 3:
			err = nvs_set_u16(my_handle, key_asAuxAss_2_wManuCode, 		 asAuxAss[2].wManuCode);
			err = nvs_set_u16(my_handle, key_asAuxAss_2_wModelIdentCode, asAuxAss[2].wModelIdentCode);
			err = nvs_set_u8 (my_handle, key_asAuxAss_2_bFuncAttribute,  asAuxAss[2].bFuncAttribute);
			err = nvs_set_u16(my_handle, key_asAuxAss_2_wObjID_Fun, 	 asAuxAss[2].wObjID_Fun);
			err = nvs_set_u16(my_handle, key_asAuxAss_2_wObjID_Input, 	 asAuxAss[2].wObjID_Input);
			err = nvs_set_blob(my_handle, key_asAuxAss_2_baAuxName, 	 asAuxAss[2].baAuxName,8);
			/* fall through */
			/* no break */
		case 2:
			err = nvs_set_u16(my_handle, key_asAuxAss_1_wManuCode, 		 asAuxAss[1].wManuCode);
			err = nvs_set_u16(my_handle, key_asAuxAss_1_wModelIdentCode, asAuxAss[1].wModelIdentCode);
			err = nvs_set_u8 (my_handle, key_asAuxAss_1_bFuncAttribute,  asAuxAss[1].bFuncAttribute);
			err = nvs_set_u16(my_handle, key_asAuxAss_1_wObjID_Fun, 	 asAuxAss[1].wObjID_Fun);
			err = nvs_set_u16(my_handle, key_asAuxAss_1_wObjID_Input, 	 asAuxAss[1].wObjID_Input);
			err = nvs_set_blob(my_handle, key_asAuxAss_1_baAuxName, 	 asAuxAss[1].baAuxName,8);
			/* fall through */
			/* no break */
		case 1:
			err = nvs_set_u16(my_handle, key_asAuxAss_0_wManuCode, 		 asAuxAss[0].wManuCode);
			err = nvs_set_u16(my_handle, key_asAuxAss_0_wModelIdentCode, asAuxAss[0].wModelIdentCode);
			err = nvs_set_u8 (my_handle, key_asAuxAss_0_bFuncAttribute,  asAuxAss[0].bFuncAttribute);
			err = nvs_set_u16(my_handle, key_asAuxAss_0_wObjID_Fun, 	 asAuxAss[0].wObjID_Fun);
			err = nvs_set_u16(my_handle, key_asAuxAss_0_wObjID_Input, 	 asAuxAss[0].wObjID_Input);
			err = nvs_set_blob(my_handle, key_asAuxAss_0_baAuxName, 	 asAuxAss[0].baAuxName,8);
			/* fall through */
			/* no break */
		case 0:
		default:
			break;
		}

		// Commit written value.
		// After setting any values, nvs_commit() must be called to ensure changes are written
		// to flash storage. Implementations may write to storage at other times,
		// but this is not guaranteed.
		printf("Committing updates in NVS ... ");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
	}
	// Close
	nvs_close(my_handle);

    return (0);
}


int ImpObjectPoolLoad( int iNr )
{
    int iCheck = 0;
#ifndef NO_MFC
    CFile abAuxObjPool; 
    CFileException fileException;
    if ( iNr == 1 )
    {
       if ( !abAuxObjPool.Open("pools/pool.iop", CFile::modeRead, &fileException ) )
       {   
            std::cout << "Error " << fileException.m_cause << " opening the pool" << std::endl;
       }
       else
       {   
           ULONGLONG ulPoolSize = abAuxObjPool.GetLength( );
           iCheck = abAuxObjPool.Read( abPoolBuffer, (UINT)ulPoolSize );
           NumberObjects_glw = IsoGetNumofPoolObjs( abPoolBuffer, (UINT)ulPoolSize );
           iso_DebugTrace( "Number of objects: %d   Poolsize: %d\n", NumberObjects_glw, ulPoolSize  );
           abAuxObjPool.Close();
       }
    }
    else
    {
       if ( !abAuxObjPool.Open("pools/pool_aux.iop", CFile::modeRead, &fileException ) )
       {   
           std::cout << "Error " << fileException.m_cause << " opening the pool" << std::endl;
       }
       else
       {   
           ULONGLONG ulPoolSize = abAuxObjPool.GetLength( );
           iCheck = abAuxObjPool.Read( abPoolBuffer, (UINT)ulPoolSize );
           NumberObjects_glw = IsoGetNumofPoolObjs( abPoolBuffer, (UINT)ulPoolSize );
           abAuxObjPool.Close();
       }
    }
#endif /* not defined NO_MFC */
    return iCheck;
}

#endif /* _LAY6_ */

const iso_char * VTSublistTextout( iso_u8 bByte1 )
{
   const iso_char *strSubl;
    switch( bByte1 )
    {
        case get_memory                    : strSubl = "VtGetMemory "; break;
        case get_number_softkey            : strSubl = "VtGetNumbSK "; break;
        case get_textfonddata              : strSubl = "VtGetTexFont"; break;
        case get_WM_data                   : strSubl = "VtGetWMData "; break;
        case get_supported_objects         : strSubl = "VtGetSuppObj"; break;
        case get_hardware                  : strSubl = "VtGetHardwar"; break;
        case store_version                 : strSubl = "VtStoreVers "; break;
        case load_version                  : strSubl = "VtLoadVersi "; break;
        case get_version                   : strSubl = "VtGetVersion"; break;
        case delete_version                : strSubl = "VtDeleteVers"; break;
        case end_of_pool_transfer          : strSubl = "VtEndOfPoTra"; break;
        case working_set_maintenance_msg   : strSubl = "WsS         "; break;
        case VT_status_message             : strSubl = "VtS         "; break;
                                                        
        case auxiliary_assign_type_2       : strSubl = "VtAuxAssiRes"; break;
        case aux_input_maintenance         : strSubl = "VtAuxInpMain"; break;
        case auxiliary_input_status_type_2 : strSubl = "VtAuxInpStat"; break;
        case aux_input_status_enable       : strSubl = "VtAuxInpEnab"; break;
        case preferred_assignment          : strSubl = "VtAuxPrefAss"; break;
                                                        
        case esc                           : strSubl = "VtEsc       "; break;
        case hide_show_object              : strSubl = "VtChaHideSho"; break;
        case enable_disable_object         : strSubl = "VtObjEnDisab"; break;
        case select_input_object           : strSubl = "VtSelInpObj "; break;
        case control_audio_device          : strSubl = "VtControlAud"; break;
        case set_audio_volume              : strSubl = "VtSetAudioVo"; break;
        case change_child_location         : strSubl = "VtChaChildLo"; break;
        case change_size                   : strSubl = "VtChaSize   "; break;
        case change_background_colour      : strSubl = "VtChaBagrCol"; break;
        case change_numeric_value          : strSubl = "VtChaNumeric"; break;
        case change_end_point              : strSubl = "VtChaEndPoin"; break;
        case change_font_attributes        : strSubl = "VtChaFontAtt"; break;
        case change_line_attributes        : strSubl = "VtChaLineAtt"; break;
        case change_fill_attributes        : strSubl = "VtChaFillAtt"; break;
        case change_activ_mask             : strSubl = "VtChaActMask"; break;
        case change_softkey_mask           : strSubl = "VtChaSKMask "; break;
        case change_attribute              : strSubl = "VtChaAttri  "; break;
        case change_priority               : strSubl = "VtChaPriorit"; break;
        case change_list_item              : strSubl = "VtChaListIte"; break;
        case delete_object_pool            : strSubl = "VtChaDelObjP"; break;
        case change_string_value           : strSubl = "VtChaStringV"; break;
        case change_child_position         : strSubl = "VtChaChildPo"; break;
        case get_attribute_value           : strSubl = "VtGetAttri  "; break;
		  case select_colour_map             : strSubl = "VtSelColMap "; break;

        default                            : strSubl = "            "; break;
    }
    return strSubl;
}



void OutputVTCoupleState( iso_u16 u16CoupleState )
{
#ifdef ISO_DEBUG_ENABLED
   /* Debug - Announcement finished */
   switch (u16CoupleState)
   {
   case Ws_MaskNoFunc:
      iso_DebugPrint("VT - Event: Connection established - WS is MaskNoFunc\n");
      break;
   case Ws_Aux:
      iso_DebugPrint("VT - Event: Connection established - WS is Aux\n");
      break;
   case Ws_Mask2Inst:
      iso_DebugPrint("VT - Event: Connection established - WS is Mask2Inst\n");
      break;
   case Ws_MaskAux:
      iso_DebugPrint("VT - Event: Connection established - WS is MaskAux\n");
      break;
   case Ws_Mask:
      iso_DebugPrint("VT - Event: Connection established - WS is Mask\n");
      break;
   case Ws_VtWaiter:
   case Ws_Last:
   default:
      break;
   }
#endif /* Debug end   */
}


const iso_char * SCSublistTextout( iso_u8 bByte1 )
{
    const iso_char *strSubl;
#ifndef NO_MFC
    switch( bByte1 )
    {
        case SCD_VERSION_REQUEST_MSG              : strSubl = "ScVersion   "; break;
        case SCD_CONF_LABEL_REQUEST_MSG           : strSubl = "ScConfLabel "; break;
        case SCD_TRANSFER_MSG                     : strSubl = "ScTransfer  "; break;
        case SCD_ACTIVATION_COMMAND_MSG           : strSubl = "ScActivCmd  "; break;
        case SCD_DELETE_COMMAND_MSG               : strSubl = "ScDeleteCmd "; break;
        case SCD_STORE_COMMAND_MSG                : strSubl = "ScStoreCmd  "; break;
        case SCD_LOAD_COMMAND_MSG                 : strSubl = "ScLoadCmd   "; break;
        case SC_MASTER_CAPABILITY_REQ_MSG         : strSubl = "ScMaCapab   "; break;
        case SC_MASTER_MEMORY_REQUEST_MSG         : strSubl = "ScMaMemory  "; break;
        case SC_STATE_COMMAND_MSG                 : strSubl = "ScStateCmd  "; break;
        case SC_CLIENT_ABORT_MSG                  : strSubl = "ScCliAbort  "; break;
        case SC_EXECUTION_INDICATION_MSG          : strSubl = "ScExeIndicat"; break;
        case SC_MASTER_EXECUTION_COMMAND_MSG      : strSubl = "ScMExeCmd   "; break;
        case SC_CLIENT_EXECUTION_STATUS_MSG       : strSubl = "ScCExeSta   "; break;
        case SC_CLIENT_FUNC_ERROR_REQ_MSG         : strSubl = "ScCFuncError"; break;
        case SC_MASTER_STATUS_MSG                 : strSubl = "ScM         "; break;
        case SC_CLIENT_STATUS_MSG                 : strSubl = "ScC         "; break;
        default                                   : strSubl = "            "; break;
    }
#else  /* defined NO_MFC */
    strSubl = "            ";
#endif /* not defined NO_MFC */ 
    return strSubl;
}

#ifdef _LAY10_
void AppTCC_LoginStateOutput( iso_u16 wTCCl )
{
   static iso_u32 dwStateOld = 0xFFFFuL;
   iso_u32 dwState; 
   
   dwState = IsoTC_StatusInfoGet( wTCCl, TCC_STATEOFANNOUNCING );

   if( dwState != dwStateOld )
   {



#ifdef ISO_DEBUG_ENABLED
	  iso_s32 s32Time;

	  s32Time = IsoClientsGetTimeMs();
      switch( dwState )
      {
       case InstanceUnused                :
          iso_DebugPrint("TC - InstanceUnused          Time: %8.4d\n", s32Time);
          break; 
       case ClientRestart                 :
          iso_DebugPrint("TC - ClientRestart           Time: %8.4d\n", s32Time);
          break; 
       case Address_claim                 :
          iso_DebugPrint("TC - Address_claim           Time: %8.4d\n", s32Time);
          break; 
       case Waiting_6_seconds             :
          iso_DebugPrint("TC - Waiting_6_seconds       Time: %8.4d\n", s32Time);
          break; 
       case WaitingTaskControllerStatus   :
          iso_DebugPrint("TC - WaitingTCStatusMsg      Time: %8.4d\n", s32Time);
          break; 
       case WSMasterSend                  :
          iso_DebugPrint("TC - WSMasterSend            Time: %8.4d\n", s32Time);
          break; 
       case WSMemberSend                  :
          iso_DebugPrint("TC - WSMemberSend            Time: %8.4d\n", s32Time);
          break; 
       case WSTaskMessageSend             :
          iso_DebugPrint("TC - WSTaskMessageSend       Time: %8.4d\n", s32Time);
          break; 
       case RequestVersionMessageSend     :
          iso_DebugPrint("TC - RequestVersionMsgSend   Time: %8.4d\n", s32Time);
          break; 
       case VersionMessageWaiting         :
          iso_DebugPrint("TC - VersionMessageWaiting   Time: %8.4d\n", s32Time);
          break; 
       case TCVersionRequestWaiting       :
          iso_DebugPrint("TC - TCVersionRequestWaiting Time: %8.4d\n", s32Time);
          break; 
       case RequestLanguageCommandSend    :
          iso_DebugPrint("TC - RequestLCSend           Time: %8.4d\n", s32Time);
          break; 
       case LanguageCommandWaiting        :
          iso_DebugPrint("TC - LanguageCommandWaiting  Time: %8.4d\n", s32Time);
          break; 
       case RequestStructureLabelSend     :
          iso_DebugPrint("TC - RequestStructLabelSend  Time: %8.4d\n", s32Time);
          break; 
       case StructureLabelResponseWaiting :
          iso_DebugPrint("TC - StructLabelRespWaiting  Time: %8.4d\n", s32Time);
          break; 
       case RequestLocalizationLabelSend  :
          iso_DebugPrint("TC - RequestLocLabelSend     Time: %8.4d\n", s32Time);
          break; 
       case LocalizationLabelWaiting      :
          iso_DebugPrint("TC - LocLabelWaiting         Time: %8.4d\n", s32Time);
          break; 
       case RequestObjectPoolTransferSend :
          iso_DebugPrint("TC - RequestObjPoolTransSend Time: %8.4d\n", s32Time);
          break; 
       case ObjPoolTransferWaiting        :
          iso_DebugPrint("TC - ObjPoolTransferWaiting  Time: %8.4d\n", s32Time);
          break; 
       case ObjPoolDeleteSend             :
          iso_DebugPrint("TC - ObjPoolDeleteSend       Time: %8.4d\n", s32Time);
          break; 
       case ObjPoolDeleteRespWaiting      :
          iso_DebugPrint("TC - ObjPoolDeleteRespWaiting Time: %8.4d\n", s32Time);
          break; 
       case ObjPoolActivateRespWaiting    :
          iso_DebugPrint("TC - ObjPoolActRespWaiting   Time: %8.4d\n", s32Time);
          break; 
       case TcRunning                     :
          iso_DebugPrint("TC - TcRunning               Time: %8.4d\n", s32Time);
          break; 
       default:
          break;
      }
#endif /* ISO_DEBUG_ENABLED */
      dwStateOld = dwState;
   }
}
#endif


const iso_char * TCSublistTextout( iso_u8 bByte1 )
{
   const iso_char *strSubl;
    switch( bByte1 & 0xF )                                
    {                                                     
        case 0:
        case 1:
           switch ( bByte1 )
           {
              case     0: strSubl = "TcVersReq   "; break;   
              case     1: strSubl = "TcReqStruLab"; break;
              case  0x10: strSubl = "TcVersion   "; break;
              case  0x11: strSubl = "TcStruLab   "; break;
              case  0x21: strSubl = "TcReqLocaLab"; break;
              case  0x31: strSubl = "TcLocaLab   "; break;
              case  0x41: strSubl = "TcReqObjPoTr"; break;
              case  0x51: strSubl = "TcReqObjPoTR"; break;
              case  0x61: strSubl = "TcObjPoTra  "; break;
              case  0x71: strSubl = "TcObjPoTraRe"; break;
              case  0x81: strSubl = "TcObjPoAct  "; break;
              case  0x91: strSubl = "TcObjPoActRe"; break;
              default:    strSubl = "            "; break;               
           }   
           break;
        case   2: strSubl = "TcReqValCmd "; break;
        case   3: strSubl = "TcValCmd    "; break;
        case   4: strSubl = "TcMPdTimInt "; break;
        case   5: strSubl = "TcMPdDistInt"; break;
        case   6: strSubl = "TcMPdMinThre"; break;
        case   7: strSubl = "TcMPdMaxThre"; break;
        case   8: strSubl = "TcMPdChaThre"; break;
        case 0xA: strSubl = "TcMPdTimInt "; break;
        case 0xD: strSubl = "TcPDNACK    "; break;
        case 0xE: strSubl = "TcS         "; break;
        case 0xF: strSubl = "WsT         "; break;
        default:  strSubl = "            "; break;                                     
    }                                                     
    return strSubl;     
}  


const iso_char * TPSublistTextout( iso_u8 bByte1, iso_u8 bByte2 )
{
   const iso_char *strSubl;
    if ( bByte1 == 0x1 )
    {
        switch( bByte2 )
        {
            case get_version_response      : strSubl = "VtGetVersRes"; break;
            case objectpool_transfer_msg   : strSubl = "VtPoolData  "; break;
            case change_string_value       : strSubl = "VtChaStringV"; break;
            case auxiliary_assign_type_2   : strSubl = "AuxAssignmen"; break;
            case preferred_assignment      : strSubl = "AuxPrefAssig"; break;
            case 1                         : strSubl = "ScDVersRes  "; break;
            case 3                         : strSubl = "ScDTransfer "; break;
            default                        : strSubl = "            "; break;
        }
    }
    else
    {
       strSubl = "            ";
    }
    return strSubl;
}

#ifdef ISO_DEBUG_ENABLED
static const  iso_char* NMUserFuncString(ISO_USERFUNC_e eIsoUserFunct)
{
   const  iso_char   *pchFunc;

   switch (eIsoUserFunct)
   {  /* internal used CF/user function */
      case working_set_master:      pchFunc = "Working set master"; break;
      case task_controller:         pchFunc = "Task controller"; break;
      case virtual_terminal:        pchFunc = "Virtual Terminal"; break;
      case tractor_ecu:             pchFunc = "Tractor ECU"; break;
      case auxiliary:               pchFunc = "Auxiliary"; break;
      case sequenz_control_master:  pchFunc = "Sequenz control master"; break;

      case working_set_member:      pchFunc = "Working set member"; break;
      case steering_control:        pchFunc = "Steering control"; break;
      case off_vehicle_gateway:     pchFunc = "Off vehicle gateway"; break;
      case network_interconnection_unit:  pchFunc = "Network interconnection unit"; break;
      case unknown:                 pchFunc = " "; break;
      case file_server:             pchFunc = "File server"; break;

      case virtual_cf:              pchFunc = "Virtual CF"; break;
      case bridged_cf:              pchFunc = "Bridged CF"; break;

      case datalogger:              pchFunc = "Data logger"; break;
      case tim_client:              pchFunc = "TIM client"; break;
      case tim_server:              pchFunc = "TIM server"; break;
      case all_clients:             pchFunc = "all_clients"; break;
      case all_server:              pchFunc = "all_server"; break;
   #if defined(ISO_NM_CF_PASSIVE)
      case cf_passive:              pchFunc = "Passive CF"; break;   /* Feature request ID 2919: Use NAME and SA without "active" network management */
   #endif /* ISO_NM_CF_PASSIVE */
      case undefined:               pchFunc = "undefined"; break;
      default:                      pchFunc = " "; break;
   }

   return pchFunc;
}
#endif /*ISO_DEBUG_ENABLED */

const iso_char * FSCSublistTextout  ( iso_u32 u32PGN, iso_u8 paMsg[])
{
#ifndef NO_MFC
    static std::string strSubl;

    if( u32PGN == PGN_FSC_TO_FS )
        strSubl = "Fsc ";
    else if( u32PGN == PGN_FS_TO_FSC )
        strSubl = "Fs  ";
   
    //switch( (ISO_FSC_CMDFUNCTION_e) paMsg[0] )
    switch(  paMsg[0] ) 
    {
        case  0     : strSubl.append("Alive   "); break;
        case  1     : strSubl.append("Property"); break;
        case  2     : strSubl.append("Volume  "); break;
        case 16     : strSubl.append("Curr Dir"); break;
        case 17     : strSubl.append("Set Dir "); break;
        case 32     : strSubl.append("Open  Fl"); break;
        case 33     : strSubl.append("Seek  Fl"); break;
        case 34     : strSubl.append("Read  Fl"); break;
        case 35     : strSubl.append("Write Fl"); break;
        case 36     : strSubl.append("Close Fl"); break;
        case 48     : strSubl.append("MoveFile"); break;
        case 49     : strSubl.append("Del File"); break;
        case 50     : strSubl.append("GetFAttr"); break;
        case 51     : strSubl.append("SetFAttr"); break;
        case 52     : strSubl.append("File DT "); break;
        case 64     : strSubl.append("Init Vol"); break;
        default     : strSubl.append("???? "); break;
    }

    return strSubl.c_str();
#else  /* defined NO_MFC */
    return "";
#endif /* not defined NO_MFC */
}

const iso_char* ACLSublistTextout( iso_u32 u32CanID, const ISO_CF_NAME_T* au8Name )
{
   static iso_char strSubl[100];
#if 0
   ISONAMEFIELD_T sNameField;

   iso_NmSetNameField(au8Name, &sNameField);
   sprintf(strSubl, "ACL %.2x: S/G=%.1x/%.1x DC/DI=%.3d/%.2d F/I/E=%.3d/%.2d/%.1d MC=%.4d SNr=%.7d", 
      (u32CanID & 0x000000FFUL),
      sNameField.bSelfConf, sNameField.bIndGroup, 
      sNameField.bDevClass, sNameField.bDevClassInst,
      sNameField.bFunction, sNameField.bFunctionInst, sNameField.bEcuInstance,
      sNameField.wManufCode, sNameField.dwIdentNumb );
#endif
   return strSubl;
}

const iso_char* TPCMSublistTextOut(iso_u32 u32CanID, iso_u8 au8Data[])
{
   static iso_char strSubl[200];
#if 0
   const iso_char *pstrFormat;
   iso_u8  u8SA, u8Err=0u;
   iso_u32 u32PGNofDT, u32P1 = 0UL, u32P2 = 0UL, u32P3 = 0UL;

   u8SA = (iso_u8)(u32CanID & 0x000000FFuL);
   u32PGNofDT = (((iso_u32)au8Data[5]) | ((iso_u32)au8Data[6] << 8) | ((iso_u32)au8Data[7] << 16));

   switch (au8Data[0])
   {
   case 0x10u: // TP_CM_RTS
   case 0x20u: // TP_CM_BAM
      pstrFormat = (au8Data[0] == 0x10u) 
                 ? "TP_CM_RTS SA=%.2x PGN=%.6x size=%.4d packets total=%.3d CTS_Max=%.3d" 
                 : "TP_CM_BAM SA=%.2x PGN=%.6x size=%.4d packets total=%.3d";
      u32P1 = (iso_u32)au8Data[1] | ((iso_u32)au8Data[2] << 8u); // number of data bytes
      u32P2 = au8Data[3]; //total number packets
      u32P3 = au8Data[4]; //max number packets to one CTS
      if ((u32P2 < 2u)
         || (u32P1 >(iso_u32)(u32P2 * 7UL))
         || (u32P1 <= ((iso_u32)((iso_u32)u32P2 - 1UL) * 7UL)))
      {  /* parameter error (packets != size)*/
         u8Err = (iso_u8)IsoTpOtherError;
      }
      break;
   case 0x11u: // TP_CM_CTS
      pstrFormat = "TP_CM_CTS SA=%.2x PGN=%.6x packets send=%.3d next=%.3d";
      u32P1 = au8Data[1]; //number of packets that can be send
      u32P2 = au8Data[2]; //next packet number
      if (u32P1 == 0UL) { u8Err = 0xEEu; /* WAIT */ }
      break;
   case 0x13u: // TP_CM_EOMA
      u32P1 = (iso_u32)au8Data[1] | ((iso_u32)au8Data[2] << 8u); // number of data bytes received
      u32P2 = au8Data[3]; //total number packets
      pstrFormat = "TP_CM_EOMA SA=%.2x PGN=%.6x received=%.4d packets=%.3d";
      break;
   case 0x14u: // ETP_CM_RTS
      u32P1 = ((iso_u32)au8Data[1]) | ((iso_u32)au8Data[2] << 8u) 
            | ((iso_u32)au8Data[3] << 16u) | ((iso_u32)au8Data[4] << 24u); // number of data bytes
      pstrFormat = "ETP_CM_RTS SA=%.2x PGN=%.6x size=%.4d";
      break;
   case 0x15u: // ETP_CM_CTS
      u32P1 = au8Data[1]; //number of packets to send
      u32P2 = (((iso_u32)au8Data[2]) | ((iso_u32)au8Data[3] << 8) | ((iso_u32)au8Data[4] << 16)); // next data packet
      if (u32P1 == 0UL) { u8Err = 0xEEu; /* WAIT */ }
      pstrFormat = "ETP_CM_CTS %.2x PGN=%.6x packets send=%.3d next=%.4d";
      break;
   case 0x16u: // ETP_CM_DPO
      u32P1 = au8Data[1]; //number of packets to which the to apply the offset
      u32P2 = (((iso_u32)au8Data[2]) | ((iso_u32)au8Data[3] << 8) | ((iso_u32)au8Data[4] << 16)); // data packet offset
      pstrFormat = "ETP_CM_DPO SA=%.2x PGN=%.6x packets send=%.3d offset=%.4d";
      break;
   case 0x17u: // ETP_CM_EOMA
      u32P1 = ((iso_u32)au8Data[1]) | ((iso_u32)au8Data[2] << 8u)
         | ((iso_u32)au8Data[3] << 16u) | ((iso_u32)au8Data[4] << 24u); // number of data bytes received
      pstrFormat = "ETP_CM_EOMA SA=%.2x PGN=%.6x  received=%.4d";
      break;

   case 0xFFu: // (E)TP Abort
      u8Err = au8Data[1]; // Abort reason
      if ((u32CanID & 0x00FF0000uL) == 0x00EC0000uL)
      {
         pstrFormat = "TP_CM_ABORT SA=%.2x PGN=%.6x";
      }
      else
      {
         pstrFormat = "ETP_CM_ABORT SA=%.2x PGN=%.6x";
      }
      break;
   default:
      u32P1 = au8Data[0];
      pstrFormat = "(E)TP ERROR SA=%.2x PGN=%.6x unknown command byte 0x%.2x";
      break;
   }
   
   {
      int fnRet;
      fnRet = sprintf(strSubl, pstrFormat, u8SA, u32PGNofDT, u32P1, u32P2, u32P3);

      switch (u8Err)
      {
      case 0u:
         pstrFormat = " ";
         break;
      case 1u:
         pstrFormat = " - ERROR (0x%.2x): cannot support another connection";
         break;
      case 2u:
         pstrFormat = " - ERROR (0x%.2x): currently no free system resources";
         break;
      case 3u:
         pstrFormat = " - ERROR (0x%.2x): timeout occurred";
         break;
      case 4u:
         pstrFormat = " - ERROR (0x%.2x): CTS messages received when data transfer is in progress";
         break;
      case 5u:
         pstrFormat = " - ERROR (0x%.2x): Maximum retransmit request limit reached";
         break;
      case 6u:
         pstrFormat = " - ERROR (0x%.2x): Unexpected data transfer packet";
         break;
      case 7u:
         pstrFormat = " - ERROR (0x%.2x): Bad sequence number";
         break;
      case 8u:
         pstrFormat = " - ERROR (0x%.2x): Duplicate sequence number";
         break;
      case 9u:
         pstrFormat = " - ERROR (0x%.2x): Unexpected EDPO packet";
         break;
      case 10u:
         pstrFormat = " - ERROR (0x%.2x): Unexpected EDPO PGN";
         break;
      case 11u:
         pstrFormat = " - ERROR (0x%.2x): EDPO number of packets is greater than CTS";
         break;
      case 12u:
         pstrFormat = " - ERROR (0x%.2x): Bad EDPO offset ";
         break;
      case 13u:
         pstrFormat = " - ERROR (0x%.2x): Any other error";
         break;
      case  0xEEu: /* (E)CTS WAIT */
         pstrFormat = " - do WAIT ";
         break;
      default:
         pstrFormat = " - ERROR (0x%.2x)";
         break;
      }

      fnRet = sprintf(&strSubl[fnRet], pstrFormat, u8Err);
   }
#endif
   return strSubl;
}

const iso_char* ACKSublistTextOut(iso_u32 u32CanID, iso_u8 au8Data[])
{
   static iso_char strSubl[100];
#if 0
   const iso_char *pstrCmd, *pstrTxt = " ";
   iso_u8  u8SA, u8DA, u8Err = 0u, u8P1, u8P2;
   iso_u32 u32PGNofDT;

   u8SA = (iso_u8)(u32CanID & 0x000000FFuL);
   u8DA = (iso_u8)(u32CanID >> 8);
   u32PGNofDT = (((iso_u32)au8Data[5]) | ((iso_u32)au8Data[6] << 8) | ((iso_u32)au8Data[7] << 16));
   u8P1 = au8Data[1]; // Group function 
   u8P2 = au8Data[4]; // SA of requester

   switch (au8Data[0])
   {
   case 0x0u: // ACK
      pstrCmd = "ACK";
      break;
   case 0x1u: // NACK
      pstrCmd = "NACK";
      break;
   case 0x2u: // Access Denied
      pstrCmd = "Access Denied";
      break;
   case 0x3u: // Cannot Respond
      pstrCmd = "Cannot Respond";
      break;
   default:
      pstrCmd = "ACK ERROR";
      pstrTxt = "unknown control byte";
      break;
   }

   if ((au8Data[0] > 0u) && (u8DA >= 0xFEu))
   {
      pstrTxt = " - ERROR global response"; // or to FE is not allowed NACKs
   }

   
   sprintf(strSubl, "%s PGN=%.6x GFV=%.2x SA=%.2x Requester=%.2x %s", pstrCmd, u32PGNofDT, u8P1, u8SA, u8P2, pstrTxt);
#endif
   return strSubl;
}

/* **************************************************************************** */
#ifdef ISO_DEBUG_USE_EXTERN_FUNCTIONS

void iso_DebugPrint( const iso_char pacFormat[], ... )
{
    va_list args;
#ifdef __cplusplus
    iso_char sOut[250];
    va_start(args, pacFormat);
    vsprintf( sOut, pacFormat, args );
    std::cout << sOut; 
#else
    va_start(args, pacFormat);
    vprintf( pacFormat, args );
#endif 
    va_end (args);
}

void iso_DebugTrace( const iso_char pacFormat[], ... )
{
#if 0
  iso_char strOut[2000];
  va_list args;
  va_start (args, pacFormat);
  vsprintf(strOut, pacFormat, args);
  //ATL::CTrace::s_trace.TraceV(NULL, -1, atlTraceGeneral, 0, pacFormat, args);
  va_end (args);

  OutputDebugString(strOut);
#endif
}

#endif

#ifdef ISO_DEBUG_USE_CALLBACK_FUNCTION

/* void isoapp_debug_out( const iso_char pacFormat[], ... ) */
void isoapp_debug_out( const iso_char pacFormat[], va_list args )
{
#if 0
    //va_list args;
#ifdef __cplusplus
    iso_char sOut[500];
    //va_start(args, pacFormat);
    vsprintf( sOut, pacFormat, args );
    std::cout << sOut; 
#else
    //va_start(args, pacFormat);
    vprintf( pacFormat, args );
#endif 
    //va_end (args);
#endif
}

#endif


/* **************************************************************************** */

#ifndef NO_MFC

/* avoid RAM size compiler errors */
#ifndef ISO_HBC_MAX
#define ISO_HBC_MAX  (0u)
#endif
#ifndef ISO_HBS_MAX
#define ISO_HBS_MAX  (0u)
#endif

CString GetSizeOfTypes( void )
{
    CString strRet = "";
    size_t size1, size2, size3, sizeCore = 0, sizeBase = 0, sizeClients = 0;
    size_t sizeTmp1;

    strRet.Append( "\n" );
    strRet.Append( "\\par RAM size calculation:\n" );
    strRet.Append( "\n" );
    strRet.Append( " - Core Driver module: \\n\n" );
    strRet.AppendFormat( "Used defines:           \t #ISO_CAN_NODES = %d ,#ISO_CANMSG_QUEUE_SIZE = %d \\n\n", ISO_CAN_NODES, ISO_CANMSG_QUEUE_SIZE );
    size1 = sizeof( ISO_NODE_T );
    size2 = size1 * ISO_CAN_NODES ;
    size3 = size2;
    strRet.AppendFormat( "Size of ISO_NODE_T:     \t %d bytes \t\\n RAM = #ISO_CAN_NODES * ISO_NODE_T = %d bytes \\n\n", size1, size2 );
    #if defined(ISO_MODULE_NIU) || defined(ISO_EXTERNAL_NIU) || defined(DRV_USE_CAN_MSG_QUEUE)
    size1 = sizeof( ISO_CAN_MSG_T );
    size2 = size1 * ISO_CANMSG_QUEUE_SIZE ;
    size3+= size2;
    strRet.AppendFormat( "Size of ISO_CAN_MSG_T:  \t %d bytes \t\\n RAM = #ISO_CANMSG_QUEUE_SIZE * ISO_CAN_MSG_T = %d bytes (only needed with NIU service ) \\n\n", size1, size2 );
    strRet.Append(       "SUM = (#ISO_CAN_NODES * ISO_NODE_T) + (#ISO_CANMSG_QUEUE_SIZE * ISO_CAN_MSG_T) \\n\n" );
    #else
    strRet.Append(       "SUM = (#ISO_CAN_NODES * ISO_NODE_T)  \\n\n" );
    #endif
    strRet.AppendFormat( "\\b SUM = %d bytes  \\n\n", size3 );
    sizeCore += size3;

    strRet.Append( "\\n\n" );
    strRet.Append( " - Network management module: \\n\n" );
    strRet.AppendFormat( "Used defines:           \t ISO_NM_CAN_NODES = #ISO_CAN_NODES = %d ,#ISO_USER_MAX = %d \\n\n", ISO_NM_CAN_NODES, ISO_USER_MAX );
    strRet.AppendFormat( "Used defines:           \t #ISO_USER_MAX_NODE1 = %d ,#ISO_USER_MAX_NODE2 = %d, #ISO_USER_MAX_NODE3 = %d \\n\n", ISO_USER_MAX_NODE1, ISO_USER_MAX_NODE2, ISO_USER_MAX_NODE3 );
    strRet.AppendFormat( "Used defines:           \t #ISO_USER_MAX_NODE4 = %d ,#ISO_USER_MAX_NODE5 = %d, #ISO_USER_MAX_NODE6 = %d \\n\n", ISO_USER_MAX_NODE4, ISO_USER_MAX_NODE5, ISO_USER_MAX_NODE6 );
    strRet.AppendFormat( "Calculated:             \t ISO_NM_USER_MAX = %d + %d + %d + %d + %d + %d + %d = %d  \\n\n", 
            ISO_USER_MAX_NODE1, ISO_USER_MAX_NODE2, ISO_USER_MAX_NODE3, ISO_USER_MAX_NODE4,
            ISO_USER_MAX_NODE5, ISO_USER_MAX_NODE6, NM_TMP_UGT6_COUNT, ISO_NM_USER_MAX );
    size1 = sizeof( ISOUSER_T );
    size2 = size1 * ISO_NM_USER_MAX ;
    strRet.AppendFormat( "Size of ISOUSER_T:      \t %d bytes \t\\n RAM = ISOUSER_T * ISO_NM_USER_MAX = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( ISOUSER_T* );
    size2 = size1 * ISO_NM_USER_MAX ;
    strRet.AppendFormat( "Size of ISOUSER_T*:     \t %d bytes \t\\n RAM = (ISOUSER_T*) * ISO_NM_USER_MAX = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( ISO_NM_NODE_T );
    size2 = size1 * ISO_NM_CAN_NODES ;
    strRet.AppendFormat( "Size of ISO_NM_NODE_T:  \t %d bytes \t\\n RAM = ISO_NM_NODE_T * ISO_NM_CAN_NODES = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( ISO_NM_DATA_T );
    strRet.Append(       "SUM = ISO_NM_DATA_T = 12 + ((ISOUSER_T*) * ISO_NM_USER_MAX ) + (ISOUSER_T * ISO_NM_USER_MAX) + (ISO_NM_NODE_T * ISO_NM_CAN_NODES) \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n", size1 );
    sizeCore += size1;

    strRet.Append( "\\n\n" );
    strRet.Append( " - Transport protocol module: \\n\n" );
    strRet.AppendFormat( "Used defines:\t\t #TP_PARALLEL_MAX = %d ,#ISO_TPREDUCED_MAX = %d, #BLOCK_SIZE_J1939 = %d \\n\n", TP_PARALLEL_MAX, ISO_TPREDUCED_MAX, BLOCK_SIZE_J1939 );
    size1 = sizeof( ISO_TL_DATA_T );
    size3 = size1;
    strRet.AppendFormat( "Size of ISO_TL_DATA_T:   \t %d bytes \\n\n", size1 );
    size1 = sizeof( ISO_TP_T );
    size2 = size1 * TP_PARALLEL_MAX ;
    size3+= size2;
    strRet.AppendFormat( "Size of ISO_TP_T:        \t %d bytes \t\\n RAM = ISO_TP_T * #TP_PARALLEL_MAX = %d bytes \\n\n", size1, size2 );
    size1 = TP_SIZE_MAX;
    size2 = size1 * ((TP_PARALLEL_MAX) - (ISO_TPREDUCED_MAX)  ) ;
    size3+= size2;
    strRet.AppendFormat( "Size of Buffer full size: \t %d bytes \t\\n RAM = 1785 * ((#TP_PARALLEL_MAX) - (#ISO_TPREDUCED_MAX) = %d bytes \\n\n", size1, size2 );
    size1 = BLOCK_SIZE_J1939;
    size2 = size1 * (ISO_TPREDUCED_MAX);
    size3+= size2;
    strRet.AppendFormat( "Size of Buffer reduced size:\t %d bytes \t\\n RAM = #BLOCK_SIZE_J1939 * #ISO_TPREDUCED_MAX = %d bytes \\n\n", size1, size2 );
    size1 = size3;
    strRet.Append(       "SUM = ISO_TL_DATA_T + (ISO_TP_T * #TP_PARALLEL_MAX) + (1785 * (#TP_PARALLEL_MAX - #ISO_TPREDUCED_MAX)) + (#BLOCK_SIZE_J1939 * #ISO_TPREDUCED_MAX)  \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n", size1 );
    sizeCore += size1;

#ifndef ISO_NM_USER_INTERN_MAX
    /* maximum number of internal CFs for all CAN nodes (array size) */
    #define ISO_NM_USER_INTERN_MAX ((iso_s16)((iso_s16)(ISO_NM_USER_MAX) / (iso_s16)2))
#endif /* !ISO_NM_USER_INTERN_MAX*/
    strRet.Append("\\n\n");
    strRet.Append(" - Base Driver module: \\n\n");
    strRet.AppendFormat("Used defines:\t\t ISO_NM_USER_MAX = %d, ISO_NM_USER_INTERN_MAX = %d \\n\n", ISO_NM_USER_MAX, ISO_NM_USER_INTERN_MAX);
    size1 = sizeof(iso_bitfield) + sizeof(iso_s16);
    size2 = size1 * ISO_NM_USER_MAX;
    sizeBase += size2;
    strRet.AppendFormat("Size of ISO_BASE_CF_T:      \t %d bytes \t\\n RAM = ISO_BASE_CF_T * ISO_NM_USER_MAX = %d bytes \\n\n", size1, size2);
    size1 = sizeof(void*) * 4; // 3 pointers and a 16 bit var...
    size2 = size1 * ISO_NM_USER_INTERN_MAX;
    sizeBase += size2;
    strRet.AppendFormat("Size of ISO_BASE_CF_PARAM_T:     \t %d bytes \t\\n RAM = ISO_BASE_CF_PARAM_T * ISO_NM_USER_INTERN_MAX = %d bytes \\n\n", size1, size2);
    size1 = sizeof(void*); // Function pointers
    size2 = size1 * (4+11); // base + wrapper   
    sizeBase += size2 + sizeof(iso_s16);
    strRet.AppendFormat("Size of function pointers:     \t %d bytes \t\\n RAM = (sizeof(pointer) * 15) + sizeof(iso_s16) = %d bytes \\n\n", size1, size2);
    strRet.Append("SUM = (ISO_BASE_CF_T * ISO_NM_USER_MAX) + (SO_BASE_CF_PARAM_T * ISO_NM_USER_INTERN_MAX) + Size of function pointers \\n\n");
    strRet.AppendFormat("\\b SUM = %d bytes \\n\n", sizeBase);


    strRet.Append( "\\n\n" );
    strRet.Append( " - Multi node application layer module: \\n\n" );
    strRet.AppendFormat( "Used defines:           \t #ISO_CAN_NODES = %d ,#NUMBERPGN_MAX = %d \\n\n", ISO_CAN_NODES, NUMBERPGN_MAX );
    size1 = 3 + (12 * ISO_CAN_NODES) + (4 * NUMBERPGN_MAX);
    size3 = size1  ;
    strRet.AppendFormat( "Size of PGNLL_T:        \t %d bytes \t =  3 + (12 * #ISO_CAN_NODES) + (4 * #NUMBERPGN_MAX)  \\n\n", size1 );
    size1 = sizeof( PGNCo_T );
    size2 = size1 * NUMBERPGN_MAX ;
    strRet.AppendFormat( "Size of PGNCo_T:        \t %d bytes \t\\n RAM = #NUMBERPGN_MAX * PGNCo_T = %d bytes \\n\n", size1, size2 );
    size1 = size2 + size3;
    strRet.Append(       "SUM = PGNLL_T + (#NUMBERPGN_MAX * PGNCo_T) \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes  \\n\n", size1 );
    sizeBase += size1;

    strRet.Append("\\n\n");
    strRet.Append(" - Heartbeat client/server module: \\n\n");
    strRet.AppendFormat("Used defines:           \t #ISO_HBC_MAX = %d ,#ISO_HBS_MAX = %d \\n\n", ISO_HBC_MAX, ISO_HBS_MAX);
    size1 = 23;   /* ... with 32-Bit CB function pointers and 32-Bit user parameter */
    size2 = size1 * ISO_HBC_MAX;
    strRet.AppendFormat("Size of HBClient_T:        \t %d bytes \t\\n RAM = #ISO_HBC_MAX * HBClient_T = %d bytes   \\n\n", size1, size2);
    size1 = 25;   /* ... with 32-Bit CB function pointers and 32-Bit user parameter */
    size3 = size1 * ISO_HBS_MAX;
    strRet.AppendFormat("Size of HBServer_T:        \t %d bytes \t\\n RAM = #ISO_HBS_MAX * HBServer_T = %d bytes \\n\n", size1, size3);
    size1 = size2 + size3;
    if (size1 > 0) size1 += 2; /* only if module is enabled we need 2 bytes for the counters */
    strRet.Append("SUM = HBData_T + (#ISO_HBC_MAX * HBClient_T) + (#ISO_HBS_MAX * HBServer_T) \\n\n");
    strRet.AppendFormat("\\b SUM = %d bytes  \\n\n", size1);
    sizeBase += size1;


    #ifndef SERVER_MAX
      #define SERVER_MAX                 (15u)  /* Max number of controlled server CFs  */
    #endif /* SERVER_MAX */   
    strRet.Append( "\\n\n" );
    strRet.Append(" - Client shared module: \\n\n");
    strRet.AppendFormat( "Used defines:           \t #WORKING_SETS = %d ,#WS_EXTERN = %d, SERVER_MAX = %d \\n\n", WORKING_SETS, WS_EXTERN, SERVER_MAX );
    /* 3 * function pointer = 3 * 4 IsoClients.c */
    /* iso_s16 sNumbWs = 2  IsoWoSe.c */
    /* iso_u8   abArray[9] UtST.c */
    size1 = sizeof( WORK_SET_T );
    size2 = size1 * (WORKING_SETS + WS_EXTERN);
    strRet.AppendFormat( "Size of WORK_SET_T:\t %d bytes \t\\n RAM = (#WORKING_SETS + #WS_EXTERN) * WORK_SET_T = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( ISO_SERV_T );
    size2 += (size1 * SERVER_MAX);
    strRet.AppendFormat( "Size of ISO_SERV_T:\t %d bytes \t\\n RAM = SERVER_MAX * ISO_SERV_T = %d bytes \\n\n", size1, (size1 * SERVER_MAX) );
    /* u16NumbServ + au8PrimTECULC[6] IsoClServ.c */
    strRet.Append(       "SUM = (#WORKING_SETS + #WS_EXTERN) * WORK_SET_T + SERVER_MAX * ISO_SERV_T \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n",  size2 );
    sizeClients += size2;


#if defined(ISO_VTC_POOLBUFFERSIZE)  
   #define POOL_BUFFER_SIZE   (ISO_VTC_POOLBUFFERSIZE)
#else 
#if defined(RAMSIZEPOOL)
   #define POOL_BUFFER_SIZE   (RAMSIZEPOOL)
#endif
#endif /* !defined(ISO_VTC_POOLBUFFERSIZE) */
    strRet.Append("\\n\n");
    strRet.Append(" - Client data buffers: \\n\n");
    strRet.AppendFormat("Used defines:           \t #RAMSIZEPOOL = %d , #DEVICEDESCRIPTION_SIZE = %d \\n\n",
                         POOL_BUFFER_SIZE, DEVICEDESCRIPTION_SIZE);
    size2 = 0;
    size1 = POOL_BUFFER_SIZE;
    size2 += size1;
    strRet.AppendFormat("Size of VTC Poolbuffer:     \t %d bytes \t\\n RAM = #RAMSIZEPOOL = %d bytes \\n\n", size1, size1);
    size1 = DEVICEDESCRIPTION_SIZE;
    size2 += size1;
    strRet.AppendFormat("Size of TCC device description:\t %d bytes \t = #DEVICEDESCRIPTION_SIZE * 1Byte = %d bytes \\n\n", size1, size1);
    strRet.Append("SUM = #RAMSIZEPOOL + #DEVICEDESCRIPTION_SIZE \\n\n");
    strRet.AppendFormat("\\b SUM = %d bytes \\n\n", size2);
    sizeClients += size2;

    strRet.Append( "\\n\n" );
    strRet.Append(" - VT client module: \\n\n");
    strRet.AppendFormat( "Used defines:           \t #VTMSG_BUFSIZE = %d ,#STRBUFFSIZE = %d \\n\n", VTMSG_BUFSIZE, STRBUFFSIZE );
    strRet.AppendFormat( "Used defines:           \t #AUXINPUTMAX = %d ,#POOLVERSIONS = %d \\n\n", AUXINPUTMAX, POOLVERSIONS );
    strRet.Append(" WS_ADMIN_T subsections: \\n\n"); 
    size1 = sizeof( iso_char );
    #if defined(ISO_VTC_UT3)  
    sizeTmp1 = LENVERSIONSTRV5; 
    #else 
    sizeTmp1 = LENVERSIONSTRV4;
    #endif
    size2 = size1 * POOLVERSIONS * sizeTmp1;
    strRet.AppendFormat( "Size of Poolvers Buffer:\t %d bytes \t = #POOLVERSIONS * %d  \\n\n", size2, sizeTmp1);

    size1 = sizeof(VTCPOOL_INFO_Ts);
    strRet.AppendFormat( "Size of VTCPOOL_INFO_Ts:\t %d bytes \t   \\n\n", size1 );  // uses POOBBUFF
    size1 = sizeof( VT_DATA_T );
    strRet.AppendFormat( "Size of VT_DATA_T:      \t %d bytes \t   \\n\n", size1 );  // uses BITMASKOBJTYPES    
    size1 = sizeof(ISOVT_CMD_STA_T);
    size2 = size1 * VTMSG_BUFSIZE ;
    strRet.AppendFormat( "Size of ISOVT_CMD_STA_T:\t %d bytes \t\\n RAM = #VTMSG_BUFSIZE * ISOVT_CMD_STA_T = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( ISOSTR_T );
    size2 = size1 * STRBUFFSIZE ;
    strRet.AppendFormat( "Size of ISOSTR_T:       \t %d bytes \t\\n RAM = #STRBUFFSIZE * ISOSTR_T = %d bytes \\n\n", size1, size2 );
    #if defined(ISO_VTC_AUX_O)
    size1 = sizeof( VT_AUX_T );
    #else
    size1 = 0;
    #endif /* defined(ISO_VTC_AUX_O) */
    size2 = size1 * AUXINPUTMAX ;
    strRet.AppendFormat( "Size of VT_AUX_T:       \t %d bytes \t\\n RAM AuxAssOld = #AUXINPUTMAX * VT_AUX_T = %d bytes \\n\n", size1, size2 );
    #if defined(ISO_VTC_AUX_N) || defined(ISO_VTC_AUX_INP_N)
    size1 = sizeof( VT_AUX_T );
    #else
    size1 = 0;
    #endif    
    size2 = size1 * AUXINPUTMAX ;
    strRet.AppendFormat( "Size of VT_AUX_T:       \t %d bytes \t\\n RAM AuxAssign = #AUXINPUTMAX * VT_AUX_T = %d bytes \\n\n", size1, size2 );
    strRet.Append(" WS_ADMIN_T: \\n\n"); 
    size1 = sizeof( WS_ADMIN_T );
    size2 = size1 * WORKING_SETS ;
    strRet.AppendFormat( "Size of WS_ADMIN_T:     \t %d bytes \t\\n RAM = #WORKING_SETS * WS_ADMIN_T = %d bytes \\n\n", size1, size2 );
    strRet.Append("SUM = #WORKING_SETS * WS_ADMIN_T \\n\n");
    strRet.AppendFormat("\\b SUM = %d bytes \\n\n", size2);
    sizeClients += size2;
/* 
    #define ISOALARM_CNTMAX              (20u)
    static ISOALARM_t      aps_Alarmfld[ISOALARM_CNTMAX];
    static iso_u16            w_AlarmfldCnt = 0u;   
    static ISOALARM_t      ps_ActiveAlarm = 0;    
    static iso_u16 ObjIdToReturnFromAlarm_u16 = UNKNOWN_ENTRY;
*/    
/*
   static POOLADMIN_T  sPoAd;          Administration of pool
   #if defined(ISO_POOL_BUFFERED_LOAD)
   #if defined(ISO_EXTERNAL_POOLBUFFER) && (!defined(DOXYGEN))
   static iso_u8 HUGE_C  *abPoolRAMBuff = 0;         Pointer to the array for temporary storage of manipulated objects
   #else
   static iso_u8 HUGE_C  abPoolRAMBuff[RAMSIZEPOOL];    Array for temporary storage of manipulated objects
   #endif
   #endif

   #ifdef POOLPROP_
   static iso_s16(*pf_ProprietaryAdaptation)(OBJTYP_e, iso_u16, iso_u32, const iso_u8 HUGE_C*, iso_u8 HUGE_C*, iso_u32*);
   #endif
*/

/*  IsoVtXcr.c
   static EcrList_T sEcrLi;
*/

    strRet.Append(" \\n\n");
    strRet.Append("Optional graphical aux client module: \\n\n");
    strRet.AppendFormat("Used defines:           \t #ISO_VTC_GAUX_CLIENTS_MAX = %d ,#ISO_VTC_GAUX_SERVERS_MAX = %d, #ISO_VTC_GAUX_CONNECTIONS_MAX = %d\\n\n", 
         ISO_VTC_GAUX_CLIENTS_MAX, ISO_VTC_GAUX_SERVERS_MAX, ISO_VTC_GAUX_CONNECTIONS_MAX);
    size1 = sizeof(VTCPOOL_INFO_Ts);
    strRet.AppendFormat("Size of VTCPOOL_INFO_Ts:    \t %d bytes \t   \\n\n", size1);  // uses POOBBUFF
    size1 = /*sizeof(ISO_VTClient_Ts)*/ 16;
    size2 = size1 * ISO_VTC_GAUX_CLIENTS_MAX;
    size3 = size2;
    strRet.AppendFormat("Size of ISO_VTClient_Ts:    \t %d bytes \t\\n RAM GAux clients = #ISO_VTC_GAUX_CLIENTS_MAX * ISO_VTClient_Ts = %d bytes \\n\n", size1, size2);
    size1 = sizeof(VT_DATA_T) + 6;
    size2 = size1 * ISO_VTC_GAUX_SERVERS_MAX;
    size3 += size2;
    strRet.AppendFormat("Size of ISO_VTServer_Ts:    \t %d bytes \t\\n RAM GAux servers = #ISO_VTC_GAUX_SERVERS_MAX * ISO_VTServer_Ts = %d bytes \\n\n", size1, size2);
    size1 = sizeof(VTCPOOL_INFO_Ts) + 28;
    size2 = size1 * ISO_VTC_GAUX_CONNECTIONS_MAX;
    size3 += size2;
    strRet.AppendFormat("Size of ISO_VTConnection_Ts: \t %d bytes \t\\n RAM GAux connections = #ISO_VTC_GAUX_CONNECTIONS_MAX * ISO_VTConnection_Ts = %d bytes \\n\n", size1, size2);
    #ifndef ISO_VTCAUX_MSG_FIFO_SIZE
      #define ISO_VTCAUX_MSG_FIFO_SIZE (32u)  /*!< module internal CAN message send FIFO size */
    #endif /* !ISO_VTCAUX_MSG_FIFO_SIZE */
    size1 = 32;
    size2 = size1 * ISO_VTCAUX_MSG_FIFO_SIZE;
    size3 += size2;
    strRet.AppendFormat("Size of FifoMsgEntry_Ts: \t %d bytes \t\\n RAM GAux msg fifo = ISO_VTCAUX_MSG_FIFO_SIZE * FifoMsgEntry_Ts = %d bytes \\n\n", size1, size2);

    /* POOL_SERVICE_T ISO_VTC_GRAPHIC_AUX FifoMsgEntry_Ts */
#if defined(ISO_VTC_GRAPHIC_AUX)
    size2 = size3;
#else
    size2 = 0;
#endif 
    strRet.AppendFormat("Size of GAux:                \t %d bytes \t\\n RAM GAux used = %d bytes \\n\n", size3, size2);
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n", size2 );
    sizeClients += size2;



    strRet.Append( "\\n\n" );
    strRet.Append(" - SQC client module: \\n\n");
    strRet.AppendFormat( "Used defines:           \t #SCSTATES_MAX = %d ,#SCFUNCTION_MAX = %d \\n\n",  SCSTATES_MAX, SCFUNCTION_MAX );
    //strRet.AppendFormat( "Used defines:           \t #SCSTATES_MAX = %d ,#SCFUNCTION_MAX = %d, #SC_LENOFLISTMAX = %d \\n\n",  SCSTATES_MAX, SCFUNCTION_MAX, SC_LENOFLISTMAX);
    // IN ScFunc_Ts ->    iso_u16            awStateObj[SC_LENOFLISTMAX];  // List of following state objects 
    size1 = sizeof( ScFunc_Ts );
    size2 = size1 * SCFUNCTION_MAX ;
    strRet.AppendFormat( "Size of ScFunc_Ts:      \t %d bytes \t\\n RAM = #SCFUNCTION_MAX * ScFunc_Ts = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( ScState_Ts );
    size2 = size1 * SCSTATES_MAX ;
    strRet.AppendFormat( "Size of ScState_Ts:     \t %d bytes \t\\n RAM = #SCSTATES_MAX * ScState_Ts = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( Scc_Ts );
    size2 = size1 * WORKING_SETS ;
    strRet.AppendFormat( "Size of Scc_Ts:         \t %d bytes \t\\n RAM = #WORKING_SETS * Scc_Ts = %d bytes \\n\n", size1, size2 );
    strRet.Append(       "SUM = #WORKING_SETS * Scc_Ts \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n", size2 );
    sizeClients += size2;

    strRet.Append( "\\n\n" );
    strRet.Append(" - TC client module: \\n\n");
    strRet.AppendFormat( "Used defines:           \t #TC_NUMB_TCCLIENTS = %d ,#PROCESSDATA_MAX = %d, #DEVICEDESCRIPTION_SIZE = %d \\n\n", 
                TC_NUMB_TCCLIENTS, PROCESSDATA_MAX, DEVICEDESCRIPTION_SIZE );
    // TASKCON_T -> TMCL_T
    strRet.Append(" TMCL_T: \\n\n"); // TC instances
    size1 = sizeof(TMCL_T);
    strRet.AppendFormat("Size of TMCL_T:     \t %d bytes   \\n\n", size1);
    size2 = size1 * TC_NUMB_TCCLIENTS;
    strRet.AppendFormat("RAM = %d bytes \t = #TC_NUMB_TCCLIENTS * TMCL_T  \\n\n", size2);
    size3 = size2;

    strRet.Append(" ISO_TCPD_T: \\n\n"); // List of PDs
    size1 = sizeof(ISO_TCPD_T) ;
    strRet.AppendFormat("Size of ISO_TCPD_T:     \t %d bytes   \\n\n", size1);
    size2 = size1 * PROCESSDATA_MAX;
    strRet.AppendFormat("RAM = %d bytes \t = #PROCESSDATA_MAX * ISO_TCPD_T  \\n\n", size2);
    size3 += size2;

    //static ISO_TCDDOP_T  sTcPool;            //    Pool service control
    // 9 + following subs
         //iso_u8  HUGE_C  *abDevDesc; -> 4
         //iso_u32         au32PosStack[TC_NUMNESTED_DE];
    strRet.Append(" ISO_TCDDOP_T subsections: \\n\n");
    size1 = sizeof(iso_u32);
    size2 = size1 * TC_NUMNESTED_DE;
    strRet.AppendFormat( "Size of stack:   \t %d bytes \t\\n RAM = #TC_NUMNESTED_DE * sizeof(iso_u32) = %d bytes \\n\n", size1, size2 );
    strRet.Append(" ISO_TCDDOP_T: \\n\n"); 
    size1 = 9 + 4 + (sizeof(iso_u32) * TC_NUMNESTED_DE) ;
    strRet.AppendFormat( "Size of ISO_TCDDOP_T:     \t %d bytes \t = 13 + (sizeof(iso_u32) * TC_NUMNESTED_DE)     \\n\n", size1 );
    size2 = size1;
    strRet.AppendFormat( "RAM = %d bytes \t = ISO_TCDDOP_T  \\n\n", size2 );
    size3+= size2;
    // 4 Bytes static ISO_TMCTRL_T  sTcCtrl;   // Superiorly TC control
    // 6 Bytes static ISO_PDCTRL_T  sTcPdCtrl; //    Control struct of PD management
    size3 += 10;
    strRet.Append(       "SUM = (#TC_NUMB_TCCLIENTS * TMCL_T) + (#PROCESSDATA_MAX * ISO_TCPD_T) + ISO_TCDDOP_T + 10 Bytes \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n", size3 );
    sizeClients += size3;

    strRet.Append( "\\n\n" );
    strRet.Append(" - FS client module: \\n\n");
    strRet.AppendFormat( "Used defines:           \t #FSC_MAX_CONNECTIONS = %d ,#FSC_MAX_IO_HANDLES = %d \\n\n", FSC_MAX_CONNECTIONS, FSC_MAX_IO_HANDLES );
    strRet.Append("SUM = FSC_DATA_T + (#FSC_MAX_CONNECTIONS * FSC_CONNECTION_T) + (#FSC_MAX_IO_HANDLES * FSC_IO_DATA_T) \\n\n");
    strRet.Append("SUM = 20 + (#FSC_MAX_CONNECTIONS * 28) + (#FSC_MAX_IO_HANDLES * 54) \\n\n");
    size1 = 20 + (FSC_MAX_CONNECTIONS * 28 ) + (FSC_MAX_IO_HANDLES * 54 ); // 52 + 2 fuellbytes
    strRet.AppendFormat("\\b SUM = %d bytes  \\n\n", size1);
    sizeClients += size1;

// Gaux input VT service
    #define ISO_VTSV_SERVERS_MAX  1u
    #if !defined(ISO_VTSV_CONNECTIONS_MAX)
      #define ISO_VTSV_CONNECTIONS_MAX  32u
    #endif /* !defined(ISO_VTSV_CONNECTIONS_MAX) */
    strRet.Append("\\n\n");
    strRet.Append(" - Optional VT service module (for graphical aux input): \\n\n");
    strRet.AppendFormat("Used defines:           \t ISO_VTSV_SERVERS_MAX = %d, ISO_VTSV_CONNECTIONS_MAX = %d \\n\n", ISO_VTSV_SERVERS_MAX, ISO_VTSV_CONNECTIONS_MAX);
    size1 = sizeof(IsoVtSv_Config_Ts) + 38;   
    size2 = size1 * ISO_VTSV_SERVERS_MAX;
    strRet.AppendFormat("Size of ISO_VTService_Ts:        \t %d bytes \t\\n RAM = ISO_VTSV_SERVERS_MAX * ISO_VTService_Ts = %d bytes   \\n\n", size1, size2);
    size1 = 16;   /* ISO_VtSvConnection_Ts */
    size3 = size1 * ISO_VTSV_CONNECTIONS_MAX;
    strRet.AppendFormat("Size of ISO_VtSvConnection_Ts:        \t %d bytes \t\\n RAM = ISO_VTSV_CONNECTIONS_MAX * ISO_VtSvConnection_Ts = %d bytes \\n\n", size1, size3);
    size1 = 1 + size2 + size3;
#if defined(ISO_VT_SERVICE)
    size3 = size1;
#else
    size3 = 0;
#endif
    strRet.Append("SUM = 1 + (ISO_VTSV_SERVERS_MAX * ISO_VTService_Ts) + (ISO_VTSV_CONNECTIONS_MAX * ISO_VtSvConnection_Ts) \\n\n");
    strRet.AppendFormat("\\b SUM = %d bytes (used %d) \\n\n", size1, size3);
    sizeClients += size3;


//NIU
#ifdef _LAY4_
    strRet.Append( "\\n\n" );
    strRet.Append( " - NIU Service module: \\n\n" );
    strRet.AppendFormat( "Used defines:           \t #NIU_MAX_CAN_PORTS = %d ,NIU_MAX_PORT_BINDINGS = %d \\n\n", NIU_MAX_CAN_PORTS, NIU_MAX_PORT_BINDINGS );
    strRet.AppendFormat( "Used defines:           \t #NIU_MAX_FILTER_PGN_ENTRIES = %d, #NIU_MAX_FILTER_PGN_NAME_ENTRIES = %d \\n\n", NIU_MAX_FILTER_PGN_ENTRIES, NIU_MAX_FILTER_PGN_NAME_ENTRIES );
    strRet.AppendFormat( "Used defines:           \t #NIU_MAX_FILTER_DBS = %d ,#NIU_MAX_ROUTER_CONNECTIONS = %d \\n\n", NIU_MAX_FILTER_DBS, NIU_MAX_ROUTER_CONNECTIONS );
    strRet.AppendFormat( "Used defines:           \t #NIU_MAX_MSG_TRANSFERS = %d, #NIU_MAX_RESPONSE_BUFFFER = %d \\n\n", NIU_MAX_MSG_TRANSFERS, NIU_MAX_RESPONSE_BUFFFER );
    size1 = sizeof( NL_PP_STAT_T );
    strRet.AppendFormat( "Size of NL_PP_STAT_T:   \t %d bytes \\n\n", size1 );
    size1 = sizeof( NL_STATISTIC_T );
    strRet.AppendFormat( "Size of NL_STATISTIC_T: \t %d bytes \\n\n", size1 );

    size1 = sizeof( NL_PORT_T );
    size2 = size1 * NIU_MAX_CAN_PORTS;
    strRet.AppendFormat( "Size of NL_PORT_T:      \t %d bytes \t\\n RAM = NL_PORT_T * #NIU_MAX_CAN_PORTS = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( NL_BINDING_T );
    size2 = size1 * NIU_MAX_PORT_BINDINGS;
    strRet.AppendFormat( "Size of NL_BINDING_T:   \t %d bytes \t\\n RAM = NL_BINDING_T * #NIU_MAX_PORT_BINDINGS = %d bytes \\n\n", size1, size2 );

    size1 = sizeof( NL_PORT_BINDING_ST_T );
    size2 = sizeof( NL_PORT_BINDING_ST_T );
    strRet.AppendFormat( "Size of NL_PORT_BINDING_ST_T:\t %d bytes \t\\n RAM = NL_PORT_BINDING_ST_T = 8 + (NL_PORT_T * #NIU_MAX_CAN_PORTS) + (NL_BINDING_T * #NIU_MAX_PORT_BINDINGS) = %d bytes \\n\n", size1, size2 );

    size1 = sizeof( NL_PGN_ENTRY_T );
    size2 = size1 * NIU_MAX_FILTER_PGN_ENTRIES;
    strRet.AppendFormat( "Size of NL_PGN_ENTRY_T: \t %d bytes \t\\n RAM = NL_PGN_ENTRY_T * #NIU_MAX_FILTER_PGN_ENTRIES = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( NL_NAME_ENTRY_T );
    size2 = size1 * NIU_MAX_FILTER_PGN_NAME_ENTRIES;
    strRet.AppendFormat( "Size of NL_NAME_ENTRY_T:\t %d bytes \t\\n RAM = NL_NAME_ENTRY_T * #NIU_MAX_FILTER_PGN_NAME_ENTRIES = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( NL_PGN_ST_T );
    strRet.AppendFormat( "Size of NL_PGN_ST_T:    \t %d bytes \t\\n RAM = NL_PGN_ST_T = 8 + (NL_PGN_ENTRY_T * #NIU_MAX_FILTER_PGN_ENTRIES) + (NL_NAME_ENTRY_T * #NIU_MAX_FILTER_PGN_NAME_ENTRIES) = %d bytes \\n\n", size1, size1 );

    size1 = sizeof( NL_FILTER_DB_T );
    size2 = size1 * NIU_MAX_FILTER_DBS;
    strRet.AppendFormat( "Size of NL_FILTER_DB_T: \t %d bytes \t\\n RAM = NL_FILTER_DB_T * #NIU_MAX_FILTER_DBS = (8 + NL_PP_STAT_T) * #NIU_MAX_FILTER_DBS = %d bytes \\n\n", size1, size2 );
    size1 = sizeof( NL_FILTER_DB_ST_T );
    strRet.AppendFormat( "Size of NL_FILTER_DB_ST_T:\t %d bytes \t\\n RAM = NL_FILTER_DB_ST_T = 4 + NL_PGN_ST_T + (NL_FILTER_DB_T * #NIU_MAX_FILTER_DBS) = %d bytes \\n\n", size1, size1 );

    size1 = sizeof( NL_CONNECTION_T );
    size2 = sizeof( NL_CONNECTIONS_ST_T );
    strRet.AppendFormat( "Size of NL_CONNECTION_T:\t %d bytes \t\\n RAM = NL_CONNECTIONS_ST_T = 4 + (NL_CONNECTION_T * #NIU_MAX_ROUTER_CONNECTIONS)  = %d bytes \\n\n", size1, size2 );

    size1 = sizeof( NL_TR_ENTRY_T );
    size2 = sizeof( NL_TR_ST_T );
    strRet.AppendFormat( "Size of NL_TR_ENTRY_T:  \t %d bytes \t\\n RAM = NL_TR_ST_T = 4 + (NL_TR_ENTRY_T * #NIU_MAX_MSG_TRANSFERS)  = %d bytes \\n\n", size1, size2 );

    size1 = sizeof( NL_DATA_T );
    strRet.AppendFormat( "Size of NL_DATA_T:      \t %d bytes \t = NL_DATA_T = 12 + ( 2 * NL_CYCLICRUN_STATE_T ) + NL_PORT_BINDING_ST_T + NL_FILTER_DB_ST_T + NL_CONNECTIONS_ST_T + NL_TR_ST_T + NL_STATISTIC_T \\n\n", size1 );

    size2 = sizeof( iso_u8 ) * NIU_MAX_RESPONSE_BUFFFER;
    strRet.AppendFormat( "Size NIU TP Buffer:     \t %d bytes \\n\n", size2 );
    strRet.Append(       "SUM = NL_DATA_T + NIU TP Buffer \\n\n" );
    strRet.AppendFormat( "\\b SUM = %d bytes \\n\n", size1 + size2 );
#else 
    size1 = 0; size2 = 0;
#endif
    strRet.Append( "\n\n\\par Total RAM size: \n" );
    strRet.AppendFormat( "SUM Core Driver = %d bytes  \\n\n", sizeCore);
    strRet.AppendFormat( "SUM Base Driver = %d bytes  \\n\n", sizeBase );
    strRet.AppendFormat( "SUM Clients     = %d bytes  \\n\n", sizeClients );
    strRet.AppendFormat( "SUM NIU         = %d bytes  \\n\n", size1 + size2 );
    strRet.AppendFormat( "<b>Total        = %d bytes</b>  \\n\n", sizeBase + sizeClients + size1 + size2 );
    strRet.Append( "\\n\n" );

    return strRet;
}
#endif /* not defined NO_MFC */

#endif // DOXNOTSHOW
