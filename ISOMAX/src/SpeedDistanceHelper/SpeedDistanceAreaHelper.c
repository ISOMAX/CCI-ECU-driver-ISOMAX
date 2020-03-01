//-IMPLEMENTATION : SpeedDistanceAreaHelper.c for SpeedDistanceAreaHelper.h
/******************************************************************/
//~IMPLEMENTATION



//#define DEBUG_SPEED_DISTANCE_HELPER 1
//#define DEBUG_SPEED_DISTANCE_HELPER_1 1
//#define DEBUG_SPEED_DISTANCE_HELPER_2 1

/** Include Files **************************************************/
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "IsoStdTypes.h"
#include "IsoDef.h"
#include "IsoMnApi.h"
#include "AppIso_Impl_NetDl.h"
#include "General_Settings.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "VIEngine.h"
#include "MyProject1.iop.h"
#include "SpeedDistanceAreaHelper.h"
#include "sdkconfig.h"

/** Private Types **************************************************/

/** Defines ********************************************************/

/** Constants ******************************************************/

/** Forward declaration of functions *******************************/
static void updateValuesFromVMVs(void);
static void updateDistancesAndAreasAndTime(void);



/** Variable declaration and definition*****************************/
static s_SpeedDistance speedDistance;
SYSTEMVARIABLE_T groundSpeed;
SYSTEMVARIABLE_T wheelSpeed;
SYSTEMVARIABLE_T present_Operator_Vehicle_Speed_Command;
SYSTEMVARIABLE_T machineSelectedSpeed;
SYSTEMVARIABLE_T navigationBasedVehicleSpeed;
SYSTEMVARIABLE_T speedOverGround;
SYSTEMVARIABLE_T groundDistance;
SYSTEMVARIABLE_T wheelDistance;
SYSTEMVARIABLE_T machineSelectedDistance;

static nvs_handle my_speed_distance_nvs;

/** Function Implementation ****************************************/





/*******************************************************************
*       public  Function speedDistanceAreaHelper_Task
********************************************************************
*
*    FUNCTION:   speedDistanceAreaHelper_Task
*	FULL NAME:	 speedDistanceAreaHelper_Task
*
* DESCRIPTION:	 The main task of this helper module. Must be
*				 called periodically or as a task in JDOS.
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   void
*
*		 DATE:   2014/12/17
*
*******************************************************************/
void speedDistanceAreaHelper_Task(void *arg) {

	{
		esp_err_t err;
		printf("\n");
		printf("Opening Non-Volatile Storage (NVS) handle... speedDistanceAreaHelper_Task ");
		//TODO !!!!
		err = nvs_open("storage", NVS_READONLY, &my_speed_distance_nvs);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");


		printf("speedDistance.pd271LifetimeTotalArea_mm2: %llu\n ", 				speedDistance.pd271LifetimeTotalArea_mm2 );
		printf("speedDistance.pd272LifetimeEffectiveTotalDistance_mm: %llu\n ", 	speedDistance.pd272LifetimeEffectiveTotalDistance_mm );
		printf("speedDistance.pd273LifetimeIneffectiveTotalDistance_mm: %llu\n ", 	speedDistance.pd273LifetimeIneffectiveTotalDistance_mm );
		printf("speedDistance.pd274LifetimeEffectiveTotalTime_us: %llu\n ", 		speedDistance.pd274LifetimeEffectiveTotalTime_us );
		printf("speedDistance.pd275LifetimeIneffectiveTotalTime_us: %llu\n ", 		speedDistance.pd275LifetimeIneffectiveTotalTime_us );
		// Close
		nvs_close(my_speed_distance_nvs);
	}




	while (1) {
#ifdef DEBUG_SPEED_DISTANCE_HELPER
     printf("speedDistanceAreaHelper_Task\n");
#endif
		// updating data
		updateValuesFromVMVs();

		// calculating
		updateDistancesAndAreasAndTime();
		vTaskDelay(pdMS_TO_TICKS(100)); //100ms
	}
	vTaskDelete(NULL);
}


//Every 1 Minute Store all Things.
void speedDistanceAreaHelper_Task_StorageLifetime(void *arg) {
	esp_err_t err;
	while (1) {

#ifdef DEBUG_SPEED_DISTANCE_HELPER_1
		printf("**********************************speedDistanceAreaHelper_Task_StorageLifetime\n");
#endif


		printf("\n");
		printf("Opening Non-Volatile Storage (NVS) handle... speedDistanceAreaHelper_Task_StorageLifetime ");
		//TODO !!!!
		err = nvs_open("storage", NVS_READWRITE, &my_speed_distance_nvs);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

		// Commit written value.
		// After setting any values, nvs_commit() must be called to ensure changes are written
		// to flash storage. Implementations may write to storage at other times,
		// but this is not guaranteed.
		printf("Committing updates in NVS ... ");
		err = nvs_commit(my_speed_distance_nvs);
		printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
		// Close
		nvs_close(my_speed_distance_nvs);

		vTaskDelay(pdMS_TO_TICKS(60000)); //1 Minute
	}
	vTaskDelete(NULL);
}

/*******************************************************************
*       public static  Function updateValuesFromVMVs
********************************************************************
*
*    FUNCTION:   updateValuesFromVMVs
*	FULL NAME:	 updateValuesFromVMVs
*
* DESCRIPTION:	 Validates the VMVs and stores their values to a
*			   local structure. Updates the status bits to flag
*			   which type of speed / distance is available.
*
*      ACCESS:   public static
*
*  PARAMETERS:	 void
*
*     RETURNS:   void
*

*		 DATE:   2014/12/17
*
*******************************************************************/
void updateValuesFromVMVs(void) {

	// Check data and fill local variables if possible.
	// Speed values
	// #### Ground Speed ####
	if (groundSpeed.status == DATA_GOODDATA) {
		speedDistance.groundBasedSpeed.speed_mm_s = (UInt16_T)groundSpeed.value; // m / s
		speedDistance.status.groundSpeedAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.groundSpeedAvailable = ISO_FALSE;
	}
	// #### Wheel Speed ####
	if (wheelSpeed.status == DATA_GOODDATA) {
		speedDistance.wheelBasedSpeed.speed_mm_s = (UInt16_T)wheelSpeed.value; // mm / s
		speedDistance.status.wheelSpeedAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.wheelSpeedAvailable = ISO_FALSE;
	}
	// #### Operator Vehicle Speed Command ####
	if (present_Operator_Vehicle_Speed_Command.status == DATA_GOODDATA) {
		speedDistance.operator_Vehicle_Speed.speed_mm_s = (UInt16_T)present_Operator_Vehicle_Speed_Command.value; // mm / s
	}
	else {
		// NEVER 0;
	}
	// #### Machine Selected Speed ####
	if (machineSelectedSpeed.status == DATA_GOODDATA) {
		speedDistance.machineSelectedSpeed.speed_mm_s = (UInt16_T)machineSelectedSpeed.value; // mm / s
		speedDistance.status.machineSelectedSpeedAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.machineSelectedSpeedAvailable = ISO_FALSE;
	}
	// #### navigation Based Vehicle Speed ####
	if (navigationBasedVehicleSpeed.status == DATA_GOODDATA) {
		speedDistance.navigationBasedVehicleSpeed.speed_mm_s = (UInt16_T)(1.085069444444444 * navigationBasedVehicleSpeed.value); // mm / s
		speedDistance.status.navigationBasedVehicleSpeedAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.navigationBasedVehicleSpeedAvailable = ISO_FALSE;
	}
	// ####  Speed Over Ground ####
	if (speedOverGround.status == DATA_GOODDATA) {
		//speedOverGround.value --> cm/s
		speedDistance.speedOverGround.speed_mm_s = (UInt16_T)(10 * speedOverGround.value); // mm / s
		speedDistance.status.speedOverGroundAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.speedOverGroundAvailable = ISO_FALSE;
	}


	// Distance Values
	// #### Ground Distance ####
	if (groundDistance.status == DATA_GOODDATA) {
		speedDistance.groundBasedDistance.distance_mm.i = groundDistance.value; // mm/s
		speedDistance.status.groundDistanceAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.groundDistanceAvailable = ISO_FALSE;
	}
	// #### Wheel Distance ####
	if (wheelDistance.status == DATA_GOODDATA ) {
		speedDistance.wheelBasedDistance.distance_mm.i = wheelDistance.value; // mm/s
		speedDistance.status.wheelDistanceAvailable = ISO_TRUE;
	}
	else {
		speedDistance.status.wheelDistanceAvailable = ISO_FALSE;
	}
	// #### Machine Selected  Distance ####
	if (machineSelectedDistance.status == DATA_GOODDATA) {
		speedDistance.machineSelectedDistance.distance_mm.i = machineSelectedDistance.value; // mm/s
		if(speedDistance.machineSelectedDistance.distance_mm.i==0) //check here if distance is zero because a lot of tractor send machine selected distance but this value is always zero only machine selected speed is valid
		{
			speedDistance.status.machineSelectedDistanceAvailable = ISO_FALSE;
		}
		else
		{
			speedDistance.status.machineSelectedDistanceAvailable = ISO_TRUE;
		}

	}
	else {
		speedDistance.status.machineSelectedDistanceAvailable = ISO_FALSE;
	}
}

/*******************************************************************
*       public static  Function updateDistances
********************************************************************
*
*    FUNCTION:   updateDistances
*	FULL NAME:	 updateDistances
*
* DESCRIPTION:	 Decides which type of speed and distance source
*				 is being used. Updates the local variables with
*				 proper values.
*
*      ACCESS:   public static
*
*  PARAMETERS:	 void
*
*     RETURNS:   void
*

*		 DATE:   2014/12/17
*
*******************************************************************/
void updateDistancesAndAreasAndTime(void) {

	UInt32_T incommingDistance_mm_machineDistance = 0;
	UInt32_T incommingDistance_mm_groundDistance = 0;
	UInt32_T incommingDistance_mm_wheelDistance = 0;
	UInt32_T difDistance_mm = 0;
	UInt32_T difDistance_mm_machineDistance = 0;
	UInt32_T difDistance_mm_groundDistance = 0;
	UInt32_T difDistance_mm_wheelDistance = 0;
	static UInt32_T lastDistance_mm_machineDistance = 0;
	static UInt32_T lastDistance_mm_groundDistance = 0;
	static UInt32_T lastDistance_mm_wheelDistance = 0;
	static int64_t lasttime = 0;

	// Decide which distance should be used whilst machineSelectedDistance is preferred over Ground, is preferred over Wheel
	if (speedDistance.status.machineSelectedDistanceAvailable == ISO_TRUE) {
		incommingDistance_mm_machineDistance = speedDistance.machineSelectedDistance.distance_mm.i;
	}
	if (speedDistance.status.groundDistanceAvailable == ISO_TRUE) {
		incommingDistance_mm_groundDistance = speedDistance.groundBasedDistance.distance_mm.i;
	}
	if (speedDistance.status.wheelDistanceAvailable == ISO_TRUE) {
		incommingDistance_mm_wheelDistance = speedDistance.wheelBasedDistance.distance_mm.i;
	}

	//init at first Run !!
	// must be HERE, not more above !
	if (lastDistance_mm_machineDistance == 0) {
		lastDistance_mm_machineDistance = incommingDistance_mm_machineDistance;
	}
	if (lastDistance_mm_groundDistance == 0) {
		lastDistance_mm_groundDistance = incommingDistance_mm_groundDistance;
	}
	if (lastDistance_mm_wheelDistance == 0) {
		lastDistance_mm_wheelDistance = incommingDistance_mm_wheelDistance;
	}
	if (lasttime == 0) {
		lasttime = esp_timer_get_time();
	}

	//check if last distance is bigger than incoming distance; this could happen if the display is shut down  and started again while the FFC was always on the bus
	//in this case we have to reset the last distance
	if (lastDistance_mm_machineDistance
			> incommingDistance_mm_machineDistance) {
		lastDistance_mm_machineDistance = incommingDistance_mm_machineDistance; // 0 is not correct here, as nobody guarantee that TECU start at 0.
	}
	if (lastDistance_mm_groundDistance > incommingDistance_mm_groundDistance) {
		lastDistance_mm_groundDistance = incommingDistance_mm_groundDistance; // 0 is not correct here, as nobody guarantee that TECU start at 0.
	}
	if (lastDistance_mm_wheelDistance > incommingDistance_mm_wheelDistance) {
		lastDistance_mm_wheelDistance = incommingDistance_mm_wheelDistance; // 0 is not correct here, as nobody guarantee that TECU start at 0.
	}

	difDistance_mm_machineDistance = (incommingDistance_mm_machineDistance			- lastDistance_mm_machineDistance);
	difDistance_mm_groundDistance = (incommingDistance_mm_groundDistance			- lastDistance_mm_groundDistance);
	difDistance_mm_wheelDistance = (incommingDistance_mm_wheelDistance				- lastDistance_mm_wheelDistance);

	// Decide which distance should be used whilst machineSelectedDistance is preferred over Ground, is preferred over Wheel
	if (speedDistance.status.machineSelectedDistanceAvailable == ISO_TRUE && difDistance_mm_machineDistance > 0)
	{
		difDistance_mm = difDistance_mm_machineDistance;
	} else if (speedDistance.status.groundDistanceAvailable == ISO_TRUE	&& difDistance_mm_groundDistance > 0)
	{
		difDistance_mm = difDistance_mm_groundDistance;
	} else if (speedDistance.status.wheelDistanceAvailable == ISO_TRUE	&& difDistance_mm_wheelDistance > 0)
	{
		difDistance_mm = difDistance_mm_wheelDistance;
	} else {
		//no Distance at all
		difDistance_mm = 0;
		//TODO Display on Screen
	}

	UInt32_T difArea_mm2 = (difDistance_mm  * 3000); //
	int64_t time = esp_timer_get_time();
	int64_t difTime_us =  time - lasttime;
#ifdef DEBUG_SPEED_DISTANCE_HELPER_1
		printf("difTime_us: %lli\n",	difTime_us);
#endif

#ifdef DEBUG_SPEED_DISTANCE_HELPER_2
		printf("General_Settings.value_u32_DeviceElement_Width: %u\n",	General_Settings.value_u32_DeviceElement_Width);
		printf("difDistance_mm: %lu\n",	difDistance_mm);
		printf("difArea_mm2: %lu\n",	difArea_mm2);
#endif
	switch (1)// Workstate e.g. qTotalsActive
	{
	case 1: // Workstate ON
		speedDistance.pd272LifetimeEffectiveTotalDistance_mm += difDistance_mm;
		speedDistance.pd117EffectiveTotalDistance_mm += difDistance_mm;
		speedDistance.pd271LifetimeTotalArea_mm2 += difArea_mm2;
		speedDistance.pd116TotalArea_mm2 += difArea_mm2;
		speedDistance.pd274LifetimeEffectiveTotalTime_us += difTime_us;
		speedDistance.pd119EffectiveTotalTime_us += difTime_us;
		break;
	default: // Workstate OFF
		speedDistance.pd273LifetimeIneffectiveTotalDistance_mm += difDistance_mm;
		speedDistance.pd118IneffectiveTotalDistance_mm += difDistance_mm;
		speedDistance.pd275LifetimeIneffectiveTotalTime_us += difTime_us;
		speedDistance.pd120IneffectiveTotalTime_us += difTime_us;
		break;
	}


#ifdef DEBUG_SPEED_DISTANCE_HELPER_1
	if (difDistance_mm > 0) {
		printf("speedDistance.lifetimeDistance_mm: %llu\n",	speedDistance.pd272LifetimeEffectiveTotalDistance_mm);
	}
#endif

	lastDistance_mm_machineDistance = incommingDistance_mm_machineDistance;
	lastDistance_mm_groundDistance 	= incommingDistance_mm_groundDistance;
	lastDistance_mm_wheelDistance 	= incommingDistance_mm_wheelDistance;
	lasttime = time;


}










/*******************************************************************
*       public  Function speedDistance_getSpeed
********************************************************************
*
*    FUNCTION:   speedDistance_getSpeed
*	FULL NAME:	 speedDistance_getSpeed
*
* DESCRIPTION:
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt16_T
*
* 	   AUTHOR:	 Franz H�pfinger
*		 DATE:   2015/04/09
*
*******************************************************************/

UInt16_T speedDistance_getSpeed(void) {
	// Decide which speed should be used whilest machineSelected is preferred.
	// if machineSelected not available, 
	// we use GroundBased when Speed <20km/h
	// we use WheelBased when Speed >20km/h

	if (speedDistance.status.navigationBasedVehicleSpeedAvailable == ISO_TRUE)
	{
		return speedDistance.navigationBasedVehicleSpeed.speed_mm_s;
	}

	else if (speedDistance.status.machineSelectedSpeedAvailable == ISO_TRUE
		&& speedDistance.machineSelectedSpeed.speed_mm_s > 0)
	{
		return speedDistance.machineSelectedSpeed.speed_mm_s;
	}
	else if (speedDistance.status.groundSpeedAvailable == ISO_TRUE
		&& speedDistance.groundBasedSpeed.speed_mm_s > 0
		&& speedDistance.status.wheelSpeedAvailable == ISO_TRUE
		&& speedDistance.wheelBasedSpeed.speed_mm_s > 0)
	{
		if (speedDistance.wheelBasedSpeed.speed_mm_s > 5555)
		{
			return speedDistance.wheelBasedSpeed.speed_mm_s;
		}
		else
		{
			return speedDistance.groundBasedSpeed.speed_mm_s;
		}

	}
	else if (speedDistance.status.groundSpeedAvailable == ISO_TRUE
		&& speedDistance.groundBasedSpeed.speed_mm_s > 0)
	{
		return speedDistance.groundBasedSpeed.speed_mm_s;
	}
	else if (speedDistance.status.wheelSpeedAvailable == ISO_TRUE)
	{
		return speedDistance.wheelBasedSpeed.speed_mm_s;
	}
	else if (speedDistance.status.navigationBasedVehicleSpeedAvailable == ISO_TRUE)
	{
		return speedDistance.navigationBasedVehicleSpeed.speed_mm_s;
	}
	else if (speedDistance.status.speedOverGroundAvailable == ISO_TRUE)
	{
		return speedDistance.speedOverGround.speed_mm_s;
	}
	else
	{
		return 0;
	}

}


/*******************************************************************
*       public  Function speedDistance_getOperatorSpeedCommand
********************************************************************
*
*    FUNCTION:   speedDistance_getOperatorSpeedCommand
*	FULL NAME:	 speedDisspeedDistance_getOperatorSpeedCommandtance_getSpeed
*
* DESCRIPTION:	 Returns the distance that was summarized while
*				 tracking flag was set to 1.
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt32_T
*
* 	   AUTHOR:	 Franz H�pfinger
*		 DATE:   2015/04/09
*
*******************************************************************/

UInt16_T speedDistance_getOperatorSpeedCommand(void){
		return speedDistance.operator_Vehicle_Speed.speed_mm_s;
}



/*******************************************************************
*       public  Function speedDistance_getMachineSelectedSpeed
********************************************************************
*
*    FUNCTION:   speedDistance_getMachineSelectedSpeed
*	FULL NAME:	 speedDistance_getMachineSelectedSpeed
*
* DESCRIPTION:	 machineSelected
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt32_T
*

*		 DATE:   2016/10/17
*
*******************************************************************/
UInt16_T speedDistance_getMachineSelectedSpeed(void) {
	return speedDistance.machineSelectedSpeed.speed_mm_s;
}


/*******************************************************************
*       public  Function speedDistance_WBSD_available
********************************************************************
*
*    FUNCTION:   speedDistance_WBSD_available
*	FULL NAME:	 speedDistance_WBSD_available
*
* DESCRIPTION:	 UInt8_T
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt8_T
*

*		 DATE:   2016/08/09
*
*******************************************************************/
UInt8_T speedDistance_WBSD_available(void) {
	return (speedDistance.status.wheelSpeedAvailable == ISO_TRUE);
}


/*******************************************************************
*       public  Function speedDistance_GBSD_available
********************************************************************
*
*    FUNCTION:   speedDistance_GBSD_available
*	FULL NAME:	 speedDistance_GBSD_available
*
* DESCRIPTION:	 UInt8_T
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt8_T
*

*		 DATE:   2016/08/09
*
*******************************************************************/
UInt8_T speedDistance_GBSD_available(void) {
	return (speedDistance.status.groundSpeedAvailable == ISO_TRUE);
}




/*******************************************************************
*       public  Function speedDistance_MSSD_available
********************************************************************
*
*    FUNCTION:   speedDistance_MSSD_available
*	FULL NAME:	 speedDistance_MSSD_available
*
* DESCRIPTION:	 UInt8_T speedDistance_GBSD_available(void) {
	return (speedDistance.status.groundSpeedAvailable == TRUE);
}
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt8_T
*

*		 DATE:   2016/10/17
*
*******************************************************************/
UInt8_T speedDistance_MSSD_available(void) {
	return (speedDistance.status.machineSelectedSpeedAvailable == ISO_TRUE);
}


/*******************************************************************
*       public  Function speedDistance_NBS_available
********************************************************************
*
*    FUNCTION:   speedDistance_NBS_available
*	FULL NAME:	 speedDistance_NBS_available
*
* DESCRIPTION:	 UInt8_T speedDistance_MSSD_available(void) {
	return (speedDistance.status.machineSelectedSpeedAvailable == TRUE);
}
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt8_T
*

*		 DATE:   2016/10/18
*
*******************************************************************/
UInt8_T speedDistance_NBS_available(void) {
	return (speedDistance.status.navigationBasedVehicleSpeedAvailable == ISO_TRUE);
}


/*******************************************************************
*       public  Function speedDistance_SOG_available
********************************************************************
*
*    FUNCTION:   speedDistance_SOG_available
*	FULL NAME:	 speedDistance_SOG_available
*
* DESCRIPTION:	 UInt8_T speedDistance_MSSD_available(void) {
	return (speedDistance.status.machineSelectedSpeedAvailable == TRUE);
}
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt8_T
*

*		 DATE:   2016/10/18
*
*******************************************************************/
UInt8_T speedDistance_SOG_available(void) {
	return (speedDistance.status.speedOverGroundAvailable == ISO_TRUE);
}



/*******************************************************************
*       public  Function speedDistance_getTrackedArea_m2
********************************************************************
*
*    FUNCTION:   speedDistance_getTrackedArea_m2
*	FULL NAME:	 speedDistance_getTrackedArea_m2
*
* DESCRIPTION:	 Returns the area calculated by using the tracked
*			     distance and thw working width.
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   UInt32_T
*

*		 DATE:   2014/12/17
*
*******************************************************************/
uint64_t speedDistance_get_pd116TotalArea_mm2(void) {
	return speedDistance.pd116TotalArea_mm2;
}
uint64_t speedDistance_get_pd117EffectiveTotalDistance_mm(void) {
	return speedDistance.pd117EffectiveTotalDistance_mm;
}
uint64_t speedDistance_get_pd118IneffectiveTotalDistance_mm(void) {
	return speedDistance.pd118IneffectiveTotalDistance_mm;
}
int64_t speedDistance_get_pd119EffectiveTotalTime_us(void) {
	return speedDistance.pd119EffectiveTotalTime_us;
}
int64_t speedDistance_get_pd120IneffectiveTotalTime_us(void) {
	return speedDistance.pd120IneffectiveTotalTime_us;
}
uint64_t speedDistance_get_pd271LifetimeTotalArea_mm2(void) {
	return speedDistance.pd271LifetimeTotalArea_mm2;
}
uint64_t speedDistance_get_pd272LifetimeEffectiveTotalDistance_mm(void) {
	return speedDistance.pd272LifetimeEffectiveTotalDistance_mm;
}
uint64_t speedDistance_get_pd273LifetimeIneffectiveTotalDistance_mm(void) {
	return  speedDistance.pd273LifetimeIneffectiveTotalDistance_mm;
}
int64_t speedDistance_get_pd274LifetimeEffectiveTotalTime_us(void) {
	return speedDistance.pd274LifetimeEffectiveTotalTime_us;
}
int64_t speedDistance_get_pd275LifetimeIneffectiveTotalTime_us(void) {
	return  speedDistance.pd275LifetimeIneffectiveTotalTime_us;
}



void speedDistance_ResetTotals(void){
	speedDistance.pd116TotalArea_mm2 =0;
	speedDistance.pd117EffectiveTotalDistance_mm =0;
	speedDistance.pd118IneffectiveTotalDistance_mm =0;
	speedDistance.pd119EffectiveTotalTime_us =0;
	speedDistance.pd120IneffectiveTotalTime_us =0;

}


uint64_t speedDistance_set_pd116TotalArea_mm2(uint64_t pd116TotalArea_mm2) {
	speedDistance.pd116TotalArea_mm2 = pd116TotalArea_mm2;
	return speedDistance.pd116TotalArea_mm2;
}
uint64_t speedDistance_set_pd117EffectiveTotalDistance_mm(uint64_t pd117EffectiveTotalDistance_mm) {
	speedDistance.pd117EffectiveTotalDistance_mm = pd117EffectiveTotalDistance_mm;
	return speedDistance.pd117EffectiveTotalDistance_mm;
}
uint64_t speedDistance_set_pd118IneffectiveTotalDistance_mm(uint64_t pd118IneffectiveTotalDistance_mm) {
	speedDistance.pd118IneffectiveTotalDistance_mm = pd118IneffectiveTotalDistance_mm;
	return speedDistance.pd118IneffectiveTotalDistance_mm;
}
int64_t speedDistance_set_pd119EffectiveTotalTime_us(int64_t pd119EffectiveTotalTime_us) {
	speedDistance.pd119EffectiveTotalTime_us = pd119EffectiveTotalTime_us;
	return speedDistance.pd119EffectiveTotalTime_us;
}
int64_t speedDistance_set_pd120IneffectiveTotalTime_us(int64_t pd120IneffectiveTotalTime_us) {
	speedDistance.pd120IneffectiveTotalTime_us = pd120IneffectiveTotalTime_us;
	return speedDistance.pd120IneffectiveTotalTime_us;
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


/* Callbackfunction for Wheel-based speed and distance */
void speedDistance_CbPGNReceiveWheelbasedSpeed( const PGNDAT_T* psData )
{
   iso_u32 u32WheelBasedSpeed = 0uL;
   iso_u32 u32WheelBasedDistance = 0uL;

   if ( psData->qTimedOut == ISO_FALSE )
   {
	   iso_SpnDataRead( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINESPEED, &u32WheelBasedSpeed );
	   iso_SpnDataRead( s16HaC1WhSpeedDis, SPN_WHEELBASEDMACHINEDISTANCE, &u32WheelBasedDistance );
     wheelSpeed.value=u32WheelBasedSpeed;
     wheelSpeed.status=DATA_GOODDATA;
     wheelDistance.value=u32WheelBasedDistance;
     wheelDistance.status=DATA_GOODDATA;

#ifdef DEBUG_SPEED_DISTANCE_HELPER
     printf("got Distance: %li\n",wheelDistance.value);
#endif
   }
   else
   {

	 wheelSpeed.value=0;
	 wheelSpeed.status=DATA_NOTAVAILABLE;
	 wheelDistance.value=0;
	 wheelDistance.status=DATA_NOTAVAILABLE;
#ifdef DEBUG_SPEED_DISTANCE_HELPER
     printf("got Distance: %li\n",wheelDistance.value);
#endif
   }
}

/* Callbackfunction for Ground-based speed and distance */
void speedDistance_CbPGNReceiveGroundbasedSpeed( const PGNDAT_T* psData )
{
	iso_u32 u32GroundBasedSpeed = 0uL;
	iso_u32 u32GroundBasedDistance = 0uL;

	if (psData->qTimedOut == ISO_FALSE)
	{
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINESPEED, &u32GroundBasedSpeed);
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_GROUNDBASEDMACHINEDISTANCE, &u32GroundBasedDistance);
		 groundSpeed.value=u32GroundBasedSpeed;
		 groundSpeed.status=DATA_GOODDATA;
		 groundDistance.value=u32GroundBasedDistance;
		 groundDistance.status=DATA_GOODDATA;
   }
   else
   {
	   groundSpeed.value=0;
	   groundSpeed.status=DATA_NOTAVAILABLE;
	   groundDistance.value=0;
	   groundDistance.status=DATA_NOTAVAILABLE;
   }
}

void speedDistance_CbPGNReceiveMachineSelectedSpeed( const PGNDAT_T* psData )
{
	iso_u32 u32SelectedSpeed = 0uL;
	iso_u32 u32SelectedDistance = 0uL;
	if (psData->qTimedOut == ISO_FALSE)
	{
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_SELECTEDMACHINESPEED, &u32SelectedSpeed);
		iso_SpnDataRead(s16HaC1GdSpeedDis, SPN_SELECTEDMACHINEDISTANCE, &u32SelectedDistance);
		 machineSelectedSpeed.value=u32SelectedSpeed;
		 machineSelectedSpeed.status=DATA_GOODDATA;
		 machineSelectedDistance.value=u32SelectedDistance;
		 machineSelectedDistance.status=DATA_GOODDATA;
   }
   else
   {
	   machineSelectedSpeed.value=0;
	   machineSelectedSpeed.status=DATA_NOTAVAILABLE;
	   machineSelectedDistance.value=0;
	   machineSelectedDistance.status=DATA_NOTAVAILABLE;
   }
}

void speedDistance_CbPGNReceiveGPSSpeed( const PGNDAT_T* psData )
{
   iso_u32 u32DatVal1 = 0uL;
   if ( psData->qTimedOut == ISO_FALSE )
   {
	 iso_SpnDataReadCom(  0, 16, psData->pau8Data, &u32DatVal1 );
     navigationBasedVehicleSpeed.value=u32DatVal1;
     navigationBasedVehicleSpeed.status=DATA_GOODDATA;
   }
   else
   {
	   navigationBasedVehicleSpeed.value=0;
	   navigationBasedVehicleSpeed.status=DATA_NOTAVAILABLE;
   }
}





//TODO:PGN_COG_SOG_RAPID_UPDATE

/*******************************************************************/
