/*
 * General_Settings.h
 *
 *  Created on: 30.12.2016
 *      Author: hoepffr
 */

#ifndef GENERAL_SETTINGS_H_
#define GENERAL_SETTINGS_H_

#include "stdint.h"


#define IMP_DEFAULT_SA     0x8cu

//														_123456789012345 //MAXIMUM 15 Chars
#define key_u8_SAImp									"u8SAImp"

#define key_iNumberOfAssigns							"iNOfAssigns"


#define key_asAuxAss_0_wManuCode						"aux_00_Manu"
#define key_asAuxAss_0_wModelIdentCode					"aux_00_Model"
#define key_asAuxAss_0_bFuncAttribute					"aux_00_Func"
#define key_asAuxAss_0_wObjID_Fun						"aux_00_Fun"
#define key_asAuxAss_0_wObjID_Input						"aux_00_Inpu"
#define key_asAuxAss_0_baAuxName						"aux_00_N0"

#define key_asAuxAss_1_wManuCode						"aux_01_Manu"
#define key_asAuxAss_1_wModelIdentCode					"aux_01_Model"
#define key_asAuxAss_1_bFuncAttribute					"aux_01_Func"
#define key_asAuxAss_1_wObjID_Fun						"aux_01_Fun"
#define key_asAuxAss_1_wObjID_Input						"aux_01_Inpu"
#define key_asAuxAss_1_baAuxName						"aux_01_Name"

#define key_asAuxAss_2_wManuCode						"aux_02_Manu"
#define key_asAuxAss_2_wModelIdentCode					"aux_02_Model"
#define key_asAuxAss_2_bFuncAttribute					"aux_02_Func"
#define key_asAuxAss_2_wObjID_Fun						"aux_02_Fun"
#define key_asAuxAss_2_wObjID_Input						"aux_02_Inpu"
#define key_asAuxAss_2_baAuxName						"aux_02_Name"

#define key_asAuxAss_3_wManuCode						"aux_03_Manu"
#define key_asAuxAss_3_wModelIdentCode					"aux_03_Model"
#define key_asAuxAss_3_bFuncAttribute					"aux_03_Func"
#define key_asAuxAss_3_wObjID_Fun						"aux_03_Fun"
#define key_asAuxAss_3_wObjID_Input						"aux_03_Inpu"
#define key_asAuxAss_3_baAuxName						"aux_03_Name"

typedef struct typ_General_Settings
{
	uint8_t  u8_SAImp; // Preferred SA for restarts
	//more can be added !
}str_General_Settings;



extern str_General_Settings General_Settings;

#endif /* GENERAL_SETTINGS_H_ */
