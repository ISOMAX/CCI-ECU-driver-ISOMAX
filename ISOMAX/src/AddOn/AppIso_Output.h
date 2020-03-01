/* ************************************************************************ */
/*!
   \file
   \brief       Debug output functions for the driver callbacks. 
   \copyright   Wegscheider Hammerl Ingenieure Partnerschaft

*/
/* ************************************************************************ */
#ifndef DEF_APPISO_OUTPUT_H
    #define DEF_APPISO_OUTPUT_H

#include "IsoCommonDef.h"

#ifdef __cplusplus
extern "C" {
#endif
/* ************************************************************************ */

/* **************************  defines  *********************************** */

/* **************************  typedef  *********************************** */

/* **************************  function declarations  ********************* */

   /* Multi threaded */
   void CbErrorOutpCore(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause);
   void CbErrorOutpBase(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause);
   void CbErrorOutpClient(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause);

   /* Single threaded */
   void CbErrorOutp(iso_u8 bFun, iso_u8 bLocation, iso_s16 iCause);

/* ************************************************************************ */

#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif /* DEF_APPISO_OUTPUT_H */
/* ************************************************************************ */
