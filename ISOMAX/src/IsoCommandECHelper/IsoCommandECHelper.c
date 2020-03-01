



#include "IsoDef.h"
#include "IsoCommandECHelper.h"
#include "AppIso_Impl_VTApp.h"


iso_s16 IsoCommandECHelper(ISOVT_FUNC_e eVtFunc, iso_u16 wObjId, iso_u32 dwVal)
{
	if (q_ProOnScreen)
	{
		return IsoCommandEC(eVtFunc, wObjId, dwVal);
	}
	return E_NOACT; //else
}


iso_s16 IsoCmd_AttributeHelper(iso_u16 u16Id, iso_u8 u8IdAttribute, iso_u32 u32ValueAttribute)
{
	if (q_ProOnScreen)
	{
		return IsoCmd_Attribute(u16Id, u8IdAttribute, u32ValueAttribute);
	}
	else
	{
		return E_NOACT;
	}
}
