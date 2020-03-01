#include "IsoDef.h"
#include "IsoStdTypes.h"
#include "MyProject1.iop.h"
#include "VIEngine.h"
#include "AppIso_Impl_VTApp.h"
#include "IsoCommandECHelper.h"
#include "General_Settings.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "AppIso_Impl_TCApp.h"
#include "DDI.h"
#include "GeometryHandling.h"
#include "GeometryHandling_initNVS.h"
#include "SpeedDistanceAreaHelper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

//not usable outside.
static nvs_handle my_geometry_nvs;



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////  u8_SAImp Getter/Setter /////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

esp_err_t Set_value_u8_SAImp(void) {
	/* Store the value of key 'u8_SAImp' to NVS */
	return nvs_set_u8(my_geometry_nvs, key_u8_SAImp, General_Settings.u8_SAImp);
}
esp_err_t Get_value_u8_SAImp(void) {
	return nvs_get_u8(my_geometry_nvs, key_u8_SAImp, &General_Settings.u8_SAImp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////  Getter/Setter  END        /////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void initDefault(void) {

	esp_err_t err;
	// Open
	printf("\n");
	printf("##########################################################################\n");
	printf("######################## INIT DEFAULT ####################################\n");
	printf("##########################################################################\n");
	printf("Opening Non-Volatile Storage (NVS) handle... ");
	printf("Set Defaults !... ");

	/* NVS partition was truncated
	 * and needs to be erased */
	printf("-----------------------------------Erase NVS");
	err = nvs_flash_erase();

	/* Retry nvs_flash_init */
	err |= nvs_flash_init();

	if (err != ESP_OK) {
		//TODO
		//ESP_LOGE(TAG, "Failed to init NVS");
		printf("Failed to init NVS");
		return;
	}
	err = nvs_open("storage", NVS_READWRITE, &my_geometry_nvs);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");

		General_Settings.u8_SAImp = IMP_DEFAULT_SA;


		/* Store the value of key 'ConnectorReferencePoint_X' to NVS */
		err = Set_value_u8_SAImp();
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");


		// Commit written value.
		// After setting any values, nvs_commit() must be called to ensure changes are written
		// to flash storage. Implementations may write to storage at other times,
		// but this is not guaranteed.
		printf("Committing updates in NVS ... ");
		err = nvs_commit(my_geometry_nvs);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

		// Close
		nvs_close(my_geometry_nvs);
		esp_restart(); // restart with Default Settings !
	}

}

void ReportErr(esp_err_t err, const char *s) {
	switch (err) {
	case ESP_OK:
		printf("Done\n");
		break;
	case ESP_ERR_NVS_NOT_FOUND:
		printf("The value is not initialized yet! %s \n", s);
		initDefault();
		break;
	default:
		printf("Error (%s) reading!\n", esp_err_to_name(err));
	}
}



void read_nvs_and_print() {
	// Read
	esp_err_t err;
	err = nvs_open("storage", NVS_READONLY, &my_geometry_nvs);
	err = Get_value_u8_SAImp();
	ReportErr(err, "General_Settings.Get_value_u8_SAImp");

	printf("read values:\n");
	printf("Preferred SA for restarts: %i\n", General_Settings.u8_SAImp);


	// Close
	nvs_close(my_geometry_nvs);
}

void initNVS(void) {

	//app_driver_init();

	/* Initialize NVS partition */
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		initDefault();
	}


	esp_err_t err = nvs_open("storage", NVS_READONLY, &my_geometry_nvs);
	// Close
	nvs_close(my_geometry_nvs);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
		printf("Error (%i) opening NVS handle!\n", err);
		if (err == ESP_ERR_NVS_NOT_FOUND) {

			initDefault();

		}
	} else {
		printf("Done. no need to init Defaults !\n");

		// Read
		read_nvs_and_print(my_geometry_nvs);

	}

}


void commit() {
	esp_err_t err;
	// Commit written value.
	// After setting any values, nvs_commit() must be called to ensure changes are written
	// to flash storage. Implementations may write to storage at other times,
	// but this is not guaranteed.
	printf("Committing updates in NVS ... ");
	err = nvs_commit(my_geometry_nvs);
	printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
	// Close
	nvs_close(my_geometry_nvs);

	// Read
	read_nvs_and_print(my_geometry_nvs);
}


#define DC_FEEDERS_MIXERS					26u
#define DC_WEEDERS							27u

/*******************************************************************
 *       public  Function IN_InputSignalCallback_GeometryHandling
 ********************************************************************
 *
 *    FUNCTION:   IN_InputSignalCallback_GeometryHandling
 *	FULL NAME:	 IN_InputSignalCallback_GeometryHandling
 *
 * DESCRIPTION:	 IN_InputSignalCallback_GeometryHandling
 *
 *      ACCESS:   public
 *
 *  PARAMETERS:	 struct InputNumber_S * pInputNumberData
 *
 *     RETURNS:   iso_u8
 *
 * 	   AUTHOR:	 Franz_Hoepfinger
 *		 DATE:   2018/04/06
 *
 *******************************************************************/
iso_u8 IN_InputSignalCallback_GeometryHandling(struct InputNumber_S *pInputNumberData) {

	switch (pInputNumberData->objectIdOfInputNumber) {
	case 1: //some obj
		switch (pInputNumberData->newValue)
		{
		case 4711: //Reset to Default !!
			nvs_flash_erase();
			esp_restart(); // restart with Default Settings !
			return (ISO_TRUE);
			break;
		default:
			break;
		}
		break;
		case 2://some obj

			break;
		default:
			break;
		}
	return (ISO_TRUE);
}

void doStoreSA(void) {

	esp_err_t err;
	// Open
	printf("\n");
	printf("Opening Non-Volatile Storage (NVS) handle... ");
	err = nvs_open("storage", NVS_READWRITE, &my_geometry_nvs);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} else {
		printf("Done\n");

		err = Set_value_u8_SAImp();
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
		commit();
	}
}



