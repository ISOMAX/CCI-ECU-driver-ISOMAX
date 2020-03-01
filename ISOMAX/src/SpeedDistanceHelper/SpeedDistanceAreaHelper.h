//-HEADER for SpeedDistanceAreaHelper.h | TinyOs_Win32
/*******************************************************************
*
* DESCRIPTION: 
*			   
*			   
*
* NOTES: speedDistanceAreaHelper_Task() needs to be called periodically
*		 for module to work propper!
*
*******************************************************************/
/*******************************************************************
*
* URL                : SpeedDistanceAreaHelper
*******************************************************************/
//~HEADER


#ifndef SPEEDDISTANCEAREAHELPER_H
#define SPEEDDISTANCEAREAHELPER_H

/*******************************************************************/
/*******************************************************************/

// Include Files **************************************************/




// Public TypeS ***************************************************/
typedef unsigned char   UInt8_T;
typedef signed char   	SInt8_T;
typedef unsigned int    UInt16_T;
typedef unsigned long   UInt32_T;


typedef struct {
	unsigned groundSpeedAvailable : 1;
	unsigned groundDistanceAvailable : 1;
	unsigned wheelSpeedAvailable : 1;
	unsigned wheelDistanceAvailable : 1;
	unsigned machineSelectedSpeedAvailable : 1;
	unsigned machineSelectedDistanceAvailable : 1;
	unsigned speedOverGroundAvailable : 1;
	unsigned navigationBasedVehicleSpeedAvailable : 1;
} s_speedStatus;

typedef union _data3{
	iso_u32 i;
	char b[4];
}uint322Byte;
typedef union _data1{
	iso_u16 i;
	char b[2];
}uint162Byte;


typedef union{
	unsigned char u8b[4];
	uint162Byte u16b[2];
	uint322Byte u32b;
	iso_u32 i;
	UInt16_T i16[2];

}U_intValConverter;

typedef struct{
	UInt16_T speed_mm_s;
}s_speedValues;

typedef struct{
	uint322Byte distance_mm;
}s_distanceValues;

typedef struct{
	s_speedValues groundBasedSpeed;
	s_speedValues wheelBasedSpeed;
	s_speedValues operator_Vehicle_Speed;
	s_speedValues machineSelectedSpeed;
	s_speedValues navigationBasedVehicleSpeed;
	s_speedValues speedOverGround;

	s_distanceValues groundBasedDistance;
	s_distanceValues wheelBasedDistance;
	s_distanceValues machineSelectedDistance;
	s_speedStatus status;

	uint64_t pd116TotalArea_mm2;						// area in mm2 Accumulated Area
	uint64_t pd117EffectiveTotalDistance_mm;			// distance in mm Accumulated Distance in working position
	uint64_t pd118IneffectiveTotalDistance_mm;			// distance in mm Accumulated Distance out of working position
	int64_t pd119EffectiveTotalTime_us;
	int64_t pd120IneffectiveTotalTime_us;
	uint64_t pd271LifetimeTotalArea_mm2;				// area in mm2 Entire Total Area of the device lifetime.
	uint64_t pd272LifetimeEffectiveTotalDistance_mm;	// distance in mm Entire Total Distance of the device lifetime in working position.
	uint64_t pd273LifetimeIneffectiveTotalDistance_mm;	// distance in mm Entire Ineffective Total Distance of the device lifetime out of working position.
	int64_t pd274LifetimeEffectiveTotalTime_us;
	int64_t pd275LifetimeIneffectiveTotalTime_us;





}s_SpeedDistance;

typedef struct SYSTEMVARIABLE
{
	SInt8_T status;
	UInt32_T value;
}SYSTEMVARIABLE_T;

enum statlist
   {
   DATA_GOODDATA = 0,
   DATA_ValidAboveNormal,
   DATA_ValidBelowNormal,
   DATA_ErraticOrInvalid,
   DATA_VoltageAboveNormal,
   DATA_VoltageBelowNormal,
   DATA_CurrentBelowNormal,
   DATA_CurrentAboveNormal,
   DATA_MechanicalOutOfAdjustment,
   DATA_ErraticFrequency,
   DATA_AbnormalUpdateRate,
   DATA_AbnormalRateOfChange,
   DATA_FailureNotIdentified, // illegal var id
   DATA_BadDeviceOrComponent,
   DATA_OutOfCalibration,
   DATA_SpecialInstructions,
   DATA_ValidAboveNormalLeastSevere,
   DATA_ValidAboveNormalModeratelySevere,
   DATA_ValidBelowNormalLeastSevere,
   DATA_ValidBelowNormalModeratelySevere,
   DATA_ReceivedNetworkDataInError,
   // Reserved for SAE assignment 15 - 30
   DATA_NOTAVAILABLE = 31     // data never written after init
   };
/** Defines ********************************************************/

/** Constants ******************************************************/

/** Public Functions ***********************************************/
//
// --> Required:
extern void speedDistanceAreaHelper_Task(void *arg);   // call periodically!
extern void speedDistanceAreaHelper_Task_StorageLifetime(void *arg);   // call periodically!
extern void speedDistance_CbPGNReceiveGPSSpeed( const PGNDAT_T* psData ); //callbackfunction have to be installed
extern void speedDistance_CbPGNReceiveMachineSelectedSpeed( const PGNDAT_T* psData );
extern void speedDistance_CbPGNReceiveGroundbasedSpeed( const PGNDAT_T* psData );
extern void speedDistance_CbPGNReceiveWheelbasedSpeed( const PGNDAT_T* psData );

// --> Public Getter:


extern UInt16_T speedDistance_getSpeed(void);
extern UInt16_T speedDistance_getOperatorSpeedCommand(void);
extern UInt16_T speedDistance_getMachineSelectedSpeed(void);
extern UInt8_T speedDistance_GBSD_available(void);
extern UInt8_T speedDistance_MSSD_available(void);
extern UInt8_T speedDistance_WBSD_available(void);
extern UInt8_T speedDistance_NBS_available(void);
extern UInt8_T speedDistance_SOG_available(void);
extern void speedDistance_ResetTotals(void);


extern uint64_t speedDistance_get_pd116TotalArea_mm2(void);
extern uint64_t speedDistance_get_pd117EffectiveTotalDistance_mm(void);
extern uint64_t speedDistance_get_pd118IneffectiveTotalDistance_mm(void);
extern int64_t speedDistance_get_pd119EffectiveTotalTime_us(void);
extern int64_t speedDistance_get_pd120IneffectiveTotalTime_us(void);
extern uint64_t speedDistance_get_pd271LifetimeTotalArea_mm2(void);
extern uint64_t speedDistance_get_pd272LifetimeEffectiveTotalDistance_mm(void);
extern uint64_t speedDistance_get_pd273LifetimeIneffectiveTotalDistance_mm(void);
extern int64_t speedDistance_get_pd274LifetimeEffectiveTotalTime_us(void);
extern int64_t speedDistance_get_pd275LifetimeIneffectiveTotalTime_us(void);




extern uint64_t speedDistance_set_pd116TotalArea_mm2(uint64_t pd116TotalArea_mm2);
extern uint64_t speedDistance_set_pd117EffectiveTotalDistance_mm(uint64_t pd117EffectiveTotalDistance_mm);
extern uint64_t speedDistance_set_pd118IneffectiveTotalDistance_mm(uint64_t pd118IneffectiveTotalDistance_mm);
extern int64_t speedDistance_set_pd119EffectiveTotalTime_us(int64_t pd119EffectiveTotalTime_us);
extern int64_t speedDistance_set_pd120IneffectiveTotalTime_us(int64_t pd120IneffectiveTotalTime_us);



/** Extern Functions ***********************************************/

/*******************************************************************/
/*******************************************************************/

#endif // SPEEDDISTANCEAREAHELPER_H


