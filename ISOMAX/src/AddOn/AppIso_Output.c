/*! \file
    \brief       Debug output functions for the driver callbacks. 
    \copyright   Wegscheider Hammerl Ingenieure Partnerschaft

*/

#include "IsoDef.h"
#include "IsoMnNiuApi.h"

#include "AppIso_Output.h"

static void ErrorOutput(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause, iso_s32 s32Time);


void CbErrorOutpCore(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause)
{
   ErrorOutput(bFun, bLocation, iCause, IsoDrvGetTimeMs());
}

void CbErrorOutpBase(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause)
{
   ErrorOutput(bFun, bLocation, iCause, iso_BaseGetTimeMs());
}

#ifdef ISO_MODULE_CLIENTS
void CbErrorOutpClient(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause)
{
   ErrorOutput(bFun, bLocation, iCause, IsoClientsGetTimeMs());
}
#endif /* ISO_MODULE_CLIENTS */

void CbErrorOutp(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause)
{  /* only for single threaded -> use CbErrorOutpCore(), CbErrorOutpBase() and CbErrorOutpClient() if multithreaded */
   ErrorOutput(bFun, bLocation, iCause, IsoDrvGetTimeMs());
}

/* ******************************************************************* */
/*!
    \brief        This function is called of the driver in order to give information to the application. \n
                  This information can have several reasons ( -> error classes ): 
                  - Configuration errors ( driver informs that an array, buffer is to small -> IsoConfig.h )
                  - Network errors ( an error code is received of the bus e. g. neg. response of a VT command )
                  - Application software errors ( wrong use of API-function ... )

                  With the help of the parameters its possible to find the affected function and source line. \n
                  An extra help page is in preparation.
                 
    \param[out]      \wp{bFun, iso_u8}         
                     Concerned function see IsoDef.h
    \param[out]      \wp{bLocation, iso_u8}     
                     Concerned module see #M_DATA ... in IsoDef.h
    \param[out]      \wp{iCause, iso_s16}
                     Error cause see #E_NO_ERR ... in IsoDef.h     
    \param[out]      \wp{s32Time, iso_s32}
                     Time stamp in milliseconds 
 */
static void ErrorOutput( iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause, iso_s32 s32Time)
{
#if defined(ISO_DEBUG_ENABLED) || defined(DOXYGEN)
    const iso_char *strMod, *strFun, *strCau;

    switch ( bLocation )
    {                               
       case M_DRV   :  
         strMod = "IsoDrvN  ";
         switch (bFun)
         {
             case ISODRVINIT:         strFun = "IsoDrvInit()      "; break;
             case ISOGETPTRCANNODE:   strFun = "IsoGetPtrCanNode()"; break;
             case ISODRVCYCLE:        strFun = "iso_drvCyclicRun()"; break;
             default:                 strFun = "Unknown           "; break;
         }   
         break;   
       case M_DD  :  
         strMod = "IsoDD    "; 
         switch (bFun)
         {
             case ISODDPROCMSG:       strFun = "iso_ddDistributeMsg()"; break;
             default:                 strFun = "Unknown           "; break;
         }           
         break;
       case M_DATA  :  
         strMod = "IsoData  "; 
         switch (bFun)
         {
             case ISODLPGNSEND:       strFun = "iso_dlPgnSend() "; break;
             /* case ISOBROADCASTSEND:   strFun = "IsoBroadcastSend()"; break; */
             case ISOFPPRECEIVE:      strFun = "Iso_TlNmeaFptRec()"; break;
             case ISOOPENTP:          strFun = "Iso_Open_TP()     "; break;
             case ISORELEASETP:       strFun = "iso_tlReleaseChannel()"; break;
             case ISOTLDOSEND:        strFun = "iso_tlDoSend()    "; break;
             case ISOTLDIRECTSEND:    strFun = "TL_DoTpSingleProcess()"; break;
             default:                 strFun = "Unknown           "; break;
         }           
         break;
       case M_USER  :  
         strMod = "IsoUser  ";
         switch (bFun)
         {  
             case ISOGETUSERINFOVIASA:strFun = "IsoGetUserInfoSA()"; break;
             default:                 strFun = "Unknown           "; break;
         }   
         break;
       case M_NET   :
         strMod = "IsoNet   "; 
         switch (bFun)
         {  
             case ISOUSERANNOUNCE:     strFun = "IsoUserAnnounce() "; break;
             case ISOUSERLOGOUT:       strFun = "IsoUserLogOut()   "; break;
             default:                  strFun = "Unknown           "; break;
         }     
         break;
       case M_WOSE  :  
         strMod = "IsoWoSe  "; 
         switch (bFun)
         {
             case ISOADDWORKINGSETMEMBER:strFun ="IsoWorkingSetMembe"; break;
             default:                    strFun ="Unknown           "; break;
         }  
         break;
       case M_ALARM :  
         strMod = "IsoAlarm "; 
         switch (bFun)
         {
             case ISOALARMOBJINIT       :strFun = "IsoAlarmObjInit()"; break;
             case ISOALARMCMD           :strFun = "IsoAlarmCmd()    "; break;
             case ISOALARMGETNUM        :strFun = "IsoAlarmGetNum() "; break;
             case ISOALARMSETRETURNOBJID:strFun = "IsoAlarmSetReturn"; break;
             default:                    strFun = "Unknown          "; break;
         }     
         break;
       case M_AUX   :  
         strMod = "IsoAux   "; 
         switch (bFun)
         {
             case ISOAUXIMPCYCLIC:  strFun ="IsoAuxImpCyclic() "; break;
             case ISOAUXASSIGNEXT:  strFun ="IsoVTAuxAssignExt "; break;
             case ISOAUXFUNCTOAPP:  strFun ="IsoAuxFunctiontoAp"; break;
             case ISOAUXPREFASSIGN: strFun ="IsoAuxPrefAssSend "; break;
             default:               strFun ="Unknown           "; break;
         }  
         break;
       case M_VTCPOOL : /* old M_POOL: */
         strMod = "IsoPool  "; 
         switch (bFun)
         {
             case ISOPOOLSETIDRANGEMODE:strFun ="IsoPoolSetIDRange "; break;
             case ISOPOOLMANIPULATE    :strFun ="IsoPoolManipulate "; break;
             case ISOPOOLRUN           :strFun ="IsoPoolRun()      "; break;
             case ISOPOOLSCAN          :strFun ="IsoPoolScan()     "; break;
             case ISOPOOLTESTIFLOAD    :strFun ="PoolObjTestIfLoad "; break;
             default:                   strFun ="Unknown           "; break;
         }              
         break;
       case M_INTERN:  
         strMod = "Intern fu"; 
         strFun = "                  "; 
         break;
       case M_APP   :  
         strMod = "Applicati"; 
         strFun = "                  "; 
         break;    
       case M_TCCTRL :  
         strMod = "IsoTcCtrl "; 
         switch (bFun)
         {
            case ISOTC_STATUSREC:       strFun ="IsoTcStatusRec()  "; break;
            default:                    strFun ="Unknown           "; break;
         }  
         break;
       case M_TCCONN :  
         strMod = "IsoTcConn "; 
         switch (bFun)
         {
             case ISOTC_INSTCMD               :   strFun = "IsoTC_InstCmd     "; break;
             case ISOTC_CONDOPROCESS          :   strFun = "IsoTcConnDoProcess"; break;
             case ISOTC_SUBCOMMANDREC         :   strFun = "IsoTC_SubCommandRe"; break;
             case ISOTC_SETPREFDL             :   strFun = "IsoTC_SetPrefDL   "; break;
             case ISOTC_SETPREFTC             :   strFun = "IsoTC_SetPrefTC   "; break;
             case ISOTC_MOVETOANOTHERTC       :   strFun = "IsoTC_MoveToAnothe"; break;
             case ISOTC_DEVICESETVERSION      :   strFun = "IsoTC_SetVersionMs"; break;
             case ISOTC_DDLOADOFBINARY        :   strFun = "IsoTC_DDLoadOfBina"; break;
             default:                             strFun = "Unknown           "; break;
         }
         break;
       case M_TCPD :  
         strMod = "IsoTcPd  "; 
         switch (bFun)
         {
             case ISOTC_PROCESSDATADEFTRIGGER_SET:strFun = "IsoTC_ProcessDataD"; break;
             default:                             strFun = "Unknown           "; break;
         }           
         break;
       case M_TCPOOL :  
         strMod = "IsoTcPool"; 
         switch (bFun)
         {
             case ISOTC_SUBCMDPOOL:               strFun = "IsoTcClSubCmdToPoo"; break;
             case ISOTC_DEVICE_SET            :   strFun = "IsoTC_Device_Set  "; break;
             case ISOTC_DEVICEELEMENT_SET     :   strFun = "IsoTC_DeviceElemen"; break;
             case ISOTC_DEVICEPROCESSDATA_SET :   strFun = "IsoTC_DeviceProces"; break;
             case ISOTC_DEVICEPROPERTY_SET    :   strFun = "IsoTC_DeviceProper"; break;
             case ISOTC_DEVICEVALUEPRESENT_SET:   strFun = "IsoTC_DeviceValueP"; break;
             case ISOTC_ADDDPDOBJECT:             strFun = "IsoTC_AddDPDObject"; break;
             case ISOTC_ADDDPTOBJECT:             strFun = "IsoTC_AddDPTObject"; break;
             case ISOTC_CHANGE_DESIGNATOR:        strFun = "IsoTC_ChangeDesign"; break;
             case ISOTC_RELOADDDOBJECTS:          strFun = "IsoTC_ReloadDDObje"; break;   
             default:                             strFun = "Unknown           "; break;
         }           
         break;
       case M_UTPICT:  
         strMod = "UtPoPict "; 
         switch (bFun)
         {
             case POOLOBJPICTCONVERT          :strFun ="PoolObjPictureConve"; break;
             default:                          strFun ="Unknown            "; break;
         }  
         break;
       case M_VT : 
          strMod = "IsoVt.c  ";
          switch (bFun)
          {
             case ISOVTMULTIPLENEXTVT:strFun = "IsoVTMultipleNext "; break;
             case ISOVTRUN           :strFun = "IsoVtcCyclic()    "; break;
             case ISOVTINIT          :strFun = "IsoVTInit()       "; break;
             case ISOWORKINGSET      :strFun = "IsoWorkingSet()   "; break;
             default                 :strFun = "Unknown "          ; break;
          }
          break;
      case M_VTCMD:
          strMod = "IsoVtMsg ";
#if defined(_LAY6_)
          switch (bFun)
          {
            case (iso_u8)preferred_assignment    :strFun = "IsoCmd_PrefAssign "; break;
            case (iso_u8)esc                     :strFun = "IsoCmd_Esc()      "; break;
            case (iso_u8)hide_show_object        :strFun = "IsoCmd_ShowHide() "; break;
            case (iso_u8)enable_disable_object   :strFun = "IsoCmd_DisEnable  "; break;
            case (iso_u8)select_input_object     :strFun = "IsoCmd_InputSelect"; break;
            case (iso_u8)control_audio_device    :strFun = "IsoCmd_ControlAudi"; break;
            case (iso_u8)set_audio_volume        :strFun = "IsoCmd_SetAudioVol"; break;
            case (iso_u8)change_child_location   :strFun = "IsoCmd_ChildLocati"; break;
            case (iso_u8)change_size             :strFun = "IsoCmd_Size()     "; break;
            case (iso_u8)change_background_colour:strFun = "IsoCmd_BackgroundC"; break;
            case (iso_u8)change_numeric_value    :strFun = "IsoCmd_NumericValu"; break;
            case (iso_u8)change_end_point        :strFun = "IsoCmd_EndPoint() "; break;
            case (iso_u8)change_font_attributes  :strFun = "IsoCmd_FontAttribu"; break;
            case (iso_u8)change_line_attributes  :strFun = "IsoCmd_LineAttribu"; break;
            case (iso_u8)change_fill_attributes  :strFun = "IsoCmd_FillAttribu"; break;
            case (iso_u8)change_activ_mask       :strFun = "IsoCmd_ActiveMask "; break;
            case (iso_u8)change_softkey_mask     :strFun = "IsoCmd_SoftKeyMask"; break;
            case (iso_u8)change_attribute        :strFun = "IsoCmd_Attribute()"; break;
            case (iso_u8)change_priority         :strFun = "IsoCmd_Priority() "; break;
            case (iso_u8)change_list_item        :strFun = "IsoCmd_ListItem() "; break;
            case (iso_u8)delete_object_pool      :strFun = "IsoCmd_DeleteObPoo"; break;
            case (iso_u8)change_string_value     :strFun = "IsoCmd_String()   "; break;
            case (iso_u8)change_child_position   :strFun = "IsoCmd_ChildPositi"; break;
            case (iso_u8)change_object_label     :strFun = "IsoCmd_ObjectLabel"; break;
            case (iso_u8)change_polygon_point    :strFun = "IsoCmd_PolyPoint()"; break;
            case (iso_u8)change_polygon_scale    :strFun = "IsoCmd_PolyScale()"; break;
            case (iso_u8)graphics_context        :strFun = "IsoCmd_GrapContext"; break;
            case (iso_u8)get_attribute_value     :strFun = "IsoCmd_GetAttribut"; break;
            case (iso_u8)select_colour_map       :strFun = "IsoCmd_SelColourMa"; break;
            case (iso_u8)identify_VT             :strFun = "IsoCmd_Identify_VT"; break;
            case (iso_u8)execute_ExtMacro        :strFun = "IsoCmd_ExeExtMacro"; break;
            case (iso_u8)lockunlock_mask         :strFun = "IsoCmd_Un_LockMask"; break;
            case (iso_u8)execute_Macro           :strFun = "IsoCmd_ExeMacro() "; break;
            case (iso_u8)get_supported_widechar  :strFun = "IsoGetSuppWidechar"; break;
            case (iso_u8)get_WM_data             :strFun = "IsoGetWMData()    "; break;
            case (iso_u8)get_supported_objects   :strFun = "IsoGetSuppObjects "; break;
            case (iso_u8)store_version           :strFun = "IsoStoreVersion() "; break;
            case (iso_u8)delete_version          :strFun = "IsoDeleteVersion()"; break;
            case (iso_u8)extended_store_version  :strFun = "IsoExtStoreVersion"; break;
            case (iso_u8)extended_delete_version :strFun = "IsoExtDeleteVersio"; break;
            case (iso_u8)end_of_pool_transfer    :strFun = "IsoEndOfPoolTransf"; break;
            default                              :strFun = "Unknown "          ; break;
          }           
#else  /* defined(_LAY6_) */
          strFun = "Unknown ";
#endif /* defined(_LAY6_) */
          break;
    case M_VTXCR :
         strMod = "IsoVtXcr "; 
         switch (bFun)
         {
             case ISOCOMMANDSC:  strFun ="IsoCommandSC()    "; break;
             case ISOCOMMANDEC:  strFun ="IsoCommandEC()    "; break;
             case ISOCOMMANDTC:  strFun ="IsoCommandTC()    "; break;
             default:            strFun ="Unknown           "; break;
         }  
         break;
       case M_SC:
         strMod = "IsoSeqC  "; 
         switch (bFun)
         {
             case ISOSCCtoSCMSEND             :strFun = "IsoSccToScmSend() "; break;    
             case ISOSC_INSTCMD               :strFun = "IsoSC_InstCmd()   "; break;
             case ISOSC_FUNCTIONOBJSET        :strFun = "IsoSC_FunctionObjS"; break;
             case ISOSC_FUNCTVALUESET         :strFun = "IsoSC_FunctValueSe"; break;
             case ISOSC_STATEOBJSET           :strFun = "IsoSC_StateObjSet "; break;
             case ISOSC_EXECUTIONINDICATION   :strFun = "IsoSC_ExecutIndica"; break;
             case ISOSC_EXECUTIONSTATUS       :strFun = "IsoSC_ExecutStatus"; break;
             case ISOSC_CLIENTABORT           :strFun = "IsoSC_ClientAbort "; break;
             case ISOSC_FUNERRORSTATUSSET     :strFun = "IsoSC_FunErrStaSet"; break;
             case ISOSC_FUNTRIGGERSET         :strFun = "IsoSC_FunTriggSet "; break;
             case ISOSC_LOGINREC              :strFun = "IsoSC_LoginRec    "; break;
             case ISOSCDPOOLPREPARE           :strFun = "IsoScdPoolPrepare "; break;
             case ISOSC_EXTOBJDEFSET          :strFun = "IsoSC_ExtObjDefSet"; break;
             case ISOSC_SETVERSION            :strFun = "IsoSC_SetVersion  "; break;
             default:                          strFun = "Unknown           "; break;
         }           
         break;
       case M_APLAY:
         strMod = "IsoMnAl  "; 
         switch (bFun)
         {
             case ISOAL_PGNTXNEW              :strFun = "iso_AlPgnTxNew()  "; break;    
             case ISOAL_PGNRXNEW              :strFun = "iso_AlPgnRxNew()  "; break;
             case ISOAL_PGNACTIVATE           :strFun = "iso_AlPgnActivate "; break;
             case ISOAL_PGNDEACTIVATE         :strFun = "iso_AlPgnDeactivat"; break;
             case ISOAL_PGNDISABLE            :strFun = "iso_AlPgnDisable()"; break;
             case ISOAL_PGNCHANGEPRIORITY     :strFun = "iso_AlPgnTxChaPrio"; break;
             case ISOAL_PGNCHANGESRCDEST      :strFun = "iso_AlPgnChaSrcDes"; break;
             case ISOAL_PGNSETREPETITIONRATE  :strFun = "iso_AlPgnSetReRaMs"; break;
             case ISOAL_PGNGETREPETITIONRATE  :strFun = "iso_AlPgnGetReRaMs"; break;
             case ISOAL_PGNRXREQUEST          :strFun = "iso_AlPgnRxRequest"; break;
             case ISOAL_PGNRXREQUESTCYC       :strFun = "iso_AlPgnRxReqCycl"; break;
             case ISOAL_PGNTXSETLIMITS        :strFun = "iso_AlPgnTxSetLiRe"; break;
             case ISOAL_PGNTXSEND             :strFun = "iso_AlPgnTxSend   "; break;
             case ISOAL_PGNSEND               :strFun = "IsoAlPGNSend()    "; break;
             default:                          strFun = "Unknown           "; break;
         }           
         break;
        case M_NMNET :  
         strMod = "IsoMnNm  "; 
         switch (bFun)
         {
             case ISONM_LISTOFEXTERNHANDLES   :strFun = "iso_NmReLiOfExHa  "; break;
             case ISONM_LISTOFEXHANDFUNC      :strFun = "iso_NmReLiOfExHaFu"; break;
             case ISONM_GETDATAOFMEMBER       :strFun = "iso_NmGetDataOfMem"; break;
             default:                          strFun = "Unknown           "; break;
         }  
         break;
        case M_SPN :  
         strMod = "IsoMnSpn "; 
         switch (bFun)
         {
             case ISOSPNDEFINE     :strFun ="iso_SpnDefineSpn()"; break;
             case ISOALPGNDATASET  :strFun ="iso_SpnDataSet()  "; break;
             case ISOALPGNDATAREAD :strFun ="iso_SpnDataRead() "; break;
             case ISOALSPNADDDTC   :strFun ="iso_SpnDMAddDTC() "; break;
             default:               strFun ="Unknown           "; break;
         }  
         break;
       case M_FSC:
        strMod = "IsoFsc   ";
        switch (bFun)
         {
            case ISOFSC_FUNCTION            : strFun ="FSC_Function()    "; break;
            case ISOFSC_INIT                : strFun ="iso_fscInit()     "; break;
            case ISOFSC_PROCESSMSG          : strFun ="iso_fscProcessMsg "; break;
            case ISOFSC_RUN                 : strFun ="iso_fscRun()      "; break;
            case ISOFSC_SEND                : strFun ="FSC_Send()        "; break;
            case ISOFSC_DOCALLBACK          : strFun ="FSC_Do Callback() "; break;
            case ISOFSC_CLOSE_CONNECTION    : strFun ="FSC_CloseConnection"; break;
            case ISOFSC_CLOSE_IO            : strFun ="FSC_CloseIO()     "; break;
            case ISOFSC_IOMODE_CALLBACK     : strFun ="FSC_IOMode_Cb()   "; break;
            case ISOFSC_FUNCTION_CALL       : strFun ="iso_Fsc...()      "; break;
            default                         : strFun = "Unknown "         ; break;
         }
        break;
       case M_NIU:
        strMod = "IsoNIU   ";
        switch (bFun)
         {
            case ISONIU_FUNCTION            : strFun ="NIU_Function()    "; break;
            case ISONIU_FN_INIT             : strFun ="iso_NiuInit()     "; break;
            case ISONIU_FN_PROCESS_REQUEST  : strFun ="iso_niuProcessMsg "; break;
            case ISONIU_FN_CYCLIC_RUN       : strFun ="iso_niuCyclicRun()"; break;
            default                         : strFun = "Unknown "         ; break;
         }
        break;
       case M_NL:
        strMod = "IsoNL    ";
        switch (bFun)
         {
            case ISONL_FUNCTION             : strFun ="NL_Function()     "; break;
            case ISONL_FN_INIT              : strFun ="iso_nlInit()      "; break;
            case ISONL_FN_PROCESS_CAN_MSG   : strFun ="iso_nlProcessMsg()"; break;
            case ISONL_FN_CYCLIC_RUN        : strFun ="iso_nlCyclicRun   "; break;
            case ISONL_FN_DO_BRIDGING       : strFun ="NL_DoBridging()   "; break;
            case ISONL_FN_DO_ROUTING        : strFun ="NL_DoRouting()    "; break;
            case ISONL_FN_FILTER_MSG        : strFun ="NL_FilterMsg()    "; break;
            case ISONL_FN_TRANSMIT_MSG      : strFun ="NL_TransmitMsg()  "; break;
            case ISONL_FN_TR_TABLE          : strFun ="NL_SetTREntry()   "; break;
            default                         : strFun = "Unknown "         ; break;
         }
        break;
       case M_CLSH:
        strMod = "IsoClShCt";
        switch (bFun)
         {
            case ISOCLSHREADLISTEXTHAND     : strFun ="CLSHReadListExtHan"; break;
            case ISOCLSHMSGDISTRIBUTOR      : strFun ="CLSHDataDistributo"; break;
            default                         : strFun ="Unknown "          ; break;
         }
        break;
       default      :  strMod = "???????  ";  strFun = "???????  ";         break;
    }
    switch ( iCause )   
    {                       
       case E_NO_ERR        : strCau = "E_NO_ERR       "; break;  
       case E_UNKNOWN_ERR   : strCau = "E_UNKNOWN_ERR  "; break;  
       case E_WARN          : strCau = "E_WARN         "; break;  
       case E_DEFAULT       : strCau = "E_DEFAULT      "; break;  
       case E_BUSY          : strCau = "E_BUSY         "; break;  
       case E_RANGE         : strCau = "E_RANGE        "; break;  
       case E_OVERFLOW      : strCau = "E_OVERFLOW     "; break;  
       case E_RD_WR         : strCau = "E_RD_WR        "; break;  
       case E_NOACT         : strCau = "E_NOACT        "; break;  
       case E_COM           : strCau = "E_COM          "; break;  
       case E_CONFIG        : strCau = "E_CONFIG       "; break;  
       case E_CHECKSUM      : strCau = "E_CHECKSUM     "; break;  
       case E_ERROR_INDI    : strCau = "E_ERROR_INDI   "; break;  
       case E_NOT_AVAILABLE : strCau = "E_NOT_AVAILABLE"; break;  
       case E_PGN_UNKNOWN   : strCau = "E_PGN_UNKNOWN  "; break;  
       case E_USER_ERROR    : strCau = "E_USER_ERROR   "; break;  
       case E_DATA_LINK     : strCau = "E_DATA_LINK    "; break;  
       case E_NO_INSTANCE   : strCau = "E_NO_INSTANCE  "; break; 
       case E_CANMSG_MISSED : strCau = "E_CANMSG_MISSED"; break; 
       case E_VALUE_WRONG   : strCau = "E_VALUE_WRONG  "; break; 
       case E_OUT_OF_MEMORY : strCau = "E_OUT_OF_MEMORY"; break; 
       case E_NACK          : strCau = "E_NACK         "; break; 
       case E_RESPTOCMDERROR: strCau = "E_RESPTOCMDERRO"; break;
       default              : strCau = "     "          ; break;
    }
    
    iso_DebugPrint("ERROR %3.2d: %-12.17s M0x%2.2X: %-10.12s, F0x%2.2X: %-18.22s Time: %8.4d \n", 
         iCause, strCau, bLocation, strMod, bFun, strFun, s32Time);

#endif /* defined(ISO_DEBUG_ENABLED) || defined(DOXYGEN) */
} /* EndDoxyErrorOutput */

