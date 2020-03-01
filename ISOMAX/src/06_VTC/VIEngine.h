
#ifndef VIEngine_h__
#define VIEngine_h__

// SOFT KEY ACTIVATION (Response == Request)
struct SoftKeyActivation_S
{
	iso_s16    iVtFunction;       /**< Last function of VT response (#ISOVT_FUNC_e) */
	iso_u16    objectIdOfKeyObject;         /**< Depends of function/command */
	iso_s32    keyActivationCode;            /**< Value of VT change numeric value */
	iso_u16    objectIdOfVisibleParentDataOrAlarmMask;            /**< Word storage 1 */
	iso_u16    wPara2;            /**< Word storage 2 */
	iso_u8     softKeyCode;             /**< Byte storage i. e. soft key number */
};

// BUTTON ACTIVATION (Response == Request)
struct ButtonActivation_S
{
	iso_s16    iVtFunction;       /**< Last function of VT response (#ISOVT_FUNC_e) */
	iso_u16    objectIdOfButtonObject;         /**< Depends of function/command */
	iso_s32    keyActivationCode;            /**< Value of VT change numeric value */
	iso_u16    objectIdOfVisibleParentDataOrAlarmMask;            /**< Word storage 1 */
	iso_u16    wPara2;            /**< Word storage 2 */
	iso_u8     softKeyCode;             /**< Byte storage i. e. soft key number */
};

// Input Number
struct InputNumber_S
{
	iso_s16    iVtFunction;       /**< Last function of VT response (#ISOVT_FUNC_e) */
	iso_u16    objectIdOfInputNumber;         /**< Depends of function/command */
	iso_s32    newValue;
};

struct InputString_S
{
    iso_s16    iVtFunction;       /**< Last function of VT response (#ISOVT_FUNC_e) */
    iso_u16    objectIdOfInputString;         /**< Depends of function/command */
    iso_u32    lValue;            /**< Value of VT change numeric value */
    iso_u16    wPara1;            /**< Word storage 1 */
    iso_u16    wPara2;            /**< Word storage 2 */
    iso_u8     numberOfBytes;     /**< Byte storage i. e. soft key number */
    iso_s16    iErrorCode;        /**< Error iso_u8 of response messages */
    iso_s32    lTimeSt;           /**< Time stamp */
    iso_u8*    newStringValue;    /**< Data of VT e. g. string characters */
};


enum Button_States_E
{
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_HELD,
	BUTTON_STATE_ABORTED
};



enum AUX_PRESS_E {

	AUX_PRESS_OFF = 0, //0 = Off = backward, down, left, or not pressed
	AUX_PRESS_MOMENTARY = 1,//1 = Momentary = forward, up, right, or pressed
	AUX_PRESS_HELD = 2//2 = held
};



#define	AUX_DOWN_FLOAT  0xFAFF
#define	AUX_DOWN  		0xFAFE
#define	AUX_MIDDLE  	0x7D7F
#define	AUX_UP  		0x0



/* AUX input signal activation callback notification struct */
typedef struct
{
	iso_s16 iVtFunction;       /**< Last function of VT response (#ISOVT_FUNC_e) */
	iso_u16 objectIdOfAuxObject;
	iso_s32    value1;            /**< Value of VT change numeric value */
	iso_u16    inputObjectID;            /**< Word storage 1 */
	iso_u16    value2;            /**< Word storage 2 */
	iso_u8     bPara;             /**< Byte storage i. e. soft key number */
	iso_s16    iErrorCode;        /**< Error iso_u8 of response messages */
}AUX_InputSignalData_T;

enum VT_line_width
{
	Line_Width_0,
	Line_Width_1,
	Line_Width_2,
	Line_Width_3,
};

enum
{
	Null_Language,
	German,
	English,
	Czech,	
	French,		
	Hungarian,	
	Italian,	
	Polish,	
	Portuguese,	
	Russian,	
	Spanish,
	Swedish,
	Bulgarian,
	Dutch

};

#endif // VIEngine_h__
