/* ************************************************************************ */
/*!
   \file        AppIso_Implement.h               
   \brief       Header for implement application ISO 11783 
   \author      Hammerl Erwin

   \copyright   Wegscheider Hammerl Ingenieure Partnerschaft
                                
   HISTORY:
   ----------
   \date 17.09.2007 \author E. Hammerl
   - created

*/
/* ************************************************************************ */
#ifndef __APPISO_IMPLEMENT_H
#define __APPISO_IMPLEMENT_H
/* ************************************************************************ */
#ifdef __cplusplus
extern "C" {
#endif
/* ************************************************************************ */

/* *******************************function prototypes  ******************* */

#define MINIMUM_CF              0

extern  iso_s16      m_CfHndImp;

   // Impl1 - First test implement
   void     AppImpl_Ignition(iso_bool qIgnition);
   iso_s16  AppImpl_doProcess(void);

   // Impl2 - Second test implement
   void    AppImpl2_Ignition(iso_bool qIgnition);
   int     AppImpl2_VT_doProcess(void);

    void     DataLinkOutput          ( ISO_TPREP_E , const ISO_TPINFO_T*  );
#ifdef _LAY6_
    void     OutputVtMessages        ( const ISOVT_MSG_STA_T * );
#endif 
    void     OutputNetworkEvents     ( const ISONETEVENT_T * psNmEvent );
#if defined(ISO_MODULE_CLIENTS)
    void     OutputCFEvents          ( const ISOCFEVENT_T * psCfData );
#endif
    int      ImpObjectPoolLoad       ( int );
    void     OutputVTCoupleState     ( iso_u16 );
    iso_s16  NextVTButtonPressed     (iso_u8 Buttonnumber);
    iso_s16  PoolReload              ( iso_u8 newPoolLanguage );
    iso_s16  PoolReloadSeg           ( void );   // Impl 2
    void     AppTriggerPreferredAssignment( void );
    void     AppAlarmRelease         ( int ); 
    void     AppAlarmDelete          ( void );
    void     AppActivatePGNsForVT    ( void );
    // Aux
    int      AppAux_KeyPress         ( int, iso_bool );
    int      AppAux_SwitchState      ( int, int );
    int	    AppAux_AnalogValueSet   ( int, unsigned int );
#ifdef _LAY6_
    uint16_t      IsoAuxReadAssignOfFile  ( VT_AUXAPP_T[] );
    int      IsoAuxWriteAssignToFile ( VT_AUXAPP_T[], int );
#endif 
    /* VT */
    void     AppVTClientSet          ( iso_s16 s16CfHandle );
    void     AppVTClientLogOut       ( void );
    /* TC */
    void     AppTCClientSet          ( iso_s16 s16CfHandle, iso_s16 s16CfSrvHandle );
    void     AppTCClientLogOut       ( iso_s16 s16CfHandle );
    void     AppTCC_MoveToAnotherTC  ( void );
    iso_s16  AppTCClientSet2         ( iso_s16 s16CfHandle );
    /* DL */
    void     AppDLClientSet          ( iso_s16 s16CfHandle, iso_s16 s16CfSrvHandle );
    void     AppDLCheckIfNewLanguage ( iso_u8 *, iso_s16 );
    void     AppDLClientLogOut       ( iso_s16 s16CfHandle );
    void     AppDLC_MoveToAnotherDL  ( void );
    /* SC */
    void    AppSCClientSet          ( iso_s16 s16CfHandle );
    void    AppSCClientLogOut       ( iso_s16 s16CfHandle );
    void    AppSCOPManipulation     ( void );
#ifdef _LAY14_
    ISO_SCLINK_T * ScLinkFIFORead          ( void );     /* Testfunction */
#endif

/* ************************************************************************ */
#ifdef __cplusplus
} /* end of extern "C" */
#endif
/* ************************************************************************ */

#endif /* __APPISO_IMPLEMENT_H */
/* ************************************************************************ */
