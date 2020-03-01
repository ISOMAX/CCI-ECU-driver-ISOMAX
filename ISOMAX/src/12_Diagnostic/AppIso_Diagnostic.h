/* ************************************************************************ */
#ifndef DEF_APPISO_DIAGNOSTIC_H
#define DEF_APPISO_DIAGNOSTIC_H
/* ************************************************************************ */

#include "IsoCommonDef.h"

/* this enum is application specific and defines the different CF's being used. */
enum CFType
{
    CFTypeIsInvalid = 0,
    CFTypeIsAuxN = 1
};

#ifdef __cplusplus
extern "C" {
#endif

void installPart12PGN(enum CFType cfType, iso_s16 nmHandle);

/* ************************************************************************ */
#ifdef __cplusplus
} /* end of extern "C" */
#endif
/* ************************************************************************ */

#endif /* DEF_APPISO_DIAGNOSTIC_H */
/* ************************************************************************ */
