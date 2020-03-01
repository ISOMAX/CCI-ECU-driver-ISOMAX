/* ************************************************************************ */
/*! 
   \file
   \brief      Hardware simulation ( Windows PC CAN BUS implementation )
   \author     Wegscheider Peter
   \date       Created XX.02.15
   \copyright  Wegscheider Hammerl Ingenieure Partnerschaft
   \par        History:
   \par
   02.02.2015 - author P. Wegscheider
   - created
   \par
   22.08.2016 - author P. Wegscheider
   - modified
   \par
   01.12.2017 - author P. Wegscheider
   - Bug ID 8120:  Fixed declaration of HW_CanMsgPrint() and updated calls
*/
/* ************************************************************************ */




#include "AppHW.h"

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/can.h"
#include "IsoConf.h"
#include "lwip\def.h"
#include "sdkconfig.h"


#define CANBUS_TAG             "CAN Master"
#define TX_GPIO_NUM             5
#define RX_GPIO_NUM             35

/* ************************************************************************ */

static const can_timing_config_t t_config = CAN_TIMING_CONFIG_250KBITS();
static const can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
//static const can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_GPIO_NUM, (gpio_num_t)RX_GPIO_NUM, CAN_MODE_NORMAL);
static const can_general_config_t g_config = {.mode = CAN_MODE_NORMAL,
                                              .tx_io = (gpio_num_t)TX_GPIO_NUM, .rx_io = (gpio_num_t)RX_GPIO_NUM,
                                              .clkout_io = (gpio_num_t)CAN_IO_UNUSED, .bus_off_io = (gpio_num_t)CAN_IO_UNUSED,
                                              .tx_queue_len = 150, .rx_queue_len = 120,
                                              .alerts_enabled = CAN_ALERT_NONE,
                                              .clkout_divider = 0};

/* ************************************************************************ */


/* ************************************************************************ */



/* ************************************************************************ */

static void HW_CanMsgPrint(uint8_t canNode_u8, can_message_t* can_msg_ps, uint8_t isRX);


/* ************************************************************************ */

//http://zone.ni.com/reference/en-XX/help/373144C-01/lvcgenhelp/cgen_determine_endian_align/
union max_align_t {

   char t0;
   short t1;
   int t2;
   long t3;
   long long t4;
   void* t5;
   float t6;
   double t7;

};

void hw_Init(void)
{
	/* Print chip information */
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	ESP_LOGI(CANBUS_TAG,"This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
			(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
			(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

	ESP_LOGI(CANBUS_TAG,"silicon revision %d, ", chip_info.revision);

	ESP_LOGI(CANBUS_TAG,"%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
			(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
					: "external");





	//http://zone.ni.com/reference/en-XX/help/373144C-01/lvcgenhelp/cgen_determine_endian_align/
	printf("Alignment = %ul\n", __alignof(union max_align_t));
	if ( htonl(2010) == 2010 ) printf("Big Endian\n");
	else printf("Little Endian\n");


	static char ser[28] = "123456789+23456789+23456789+";
	Serial_Number_VariableGet(ser);
	printf("MAC = %s -\n", ser);
}


/*******************************************************************
*       public  Function Serial_Number_VariableGet
********************************************************************
*
*    FUNCTION:   Serial_Number_VariableGet
*	FULL NAME:	 Serial_Number_VariableGet
*
* DESCRIPTION:	 3rdParty\cci\ecu_driver
*
*      ACCESS:   public
*
*  PARAMETERS:	 iso_u8 c_Serial_Number_Variable[]
*
*     RETURNS:   void
*
*		 DATE:   2016/12/23
*
*******************************************************************/
int Serial_Number_VariableGet(char * c_Serial_Number_Variable)
{
    uint8_t a[6];
    esp_efuse_mac_get_default(a);

    ESP_LOGI(CANBUS_TAG,"MAC = ## " MACSTR " ##", MAC2STR(a));

    //Returns e.g.
    //
    // 123456789+23456789+234567
    // ##MAC:24:6f:28:42:e9:50##
    // is a 28 character String.





    return sprintf(&c_Serial_Number_Variable[0], "##MAC:%02x:%02x:%02x:%02x:%02x:%02x##",  (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]);


}

/*******************************************************************
*       public  Function u32SeriNoGet
********************************************************************
*
*    FUNCTION:   u32SeriNoGet
*	FULL NAME:	 u32SeriNoGet
*
*	u32SeriNo
*	24:6f:28:42:e9:50
*	24:6F:28 			--> Espressi	Espressif Inc.
*			 42:e9:50   --> 24 Bit, we need 21.
*
*
* DESCRIPTION:	 iso_u32
*			   	 				 			 
*      ACCESS:   public 
*
*  PARAMETERS:	 void
*
*     RETURNS:   iso_u32
*
*		 DATE:   2016/12/29
*
*******************************************************************/
uint32_t  u32SeriNoGet(void)
{



    uint8_t b[6];
    esp_efuse_mac_get_default(b);

    b[0] = 0x00;
    b[1] = 0x00;
    b[2] = 0x00;
    b[3] = b[3] & 0x1f; //cut 24 bits to 21


    ESP_LOGI(CANBUS_TAG,"SHORT-MAC = ## " MACSTR " ##", MAC2STR(b));


	return b[3] << 16 | b[4] << 8 | b[5];

}



void hw_Shutdown(void)
{

}

uint8_t hw_PowerSwitchIsOn(void)
{
   //Sleep( 2UL );
   return 1u;
}



int32_t hw_GetTimeMs(void)
{
	int64_t tt = esp_timer_get_time();
	return (int32_t)(tt/1000);
}


/* ################### CAN Functions ################ */

/*******************************************************************
*       public  Function hw_CanInit
********************************************************************
*
*    FUNCTION:   hw_CanInit
*	FULL NAME:	 hw_CanInit
*
* DESCRIPTION:	 3rdParty\cci\ecu_driver
*
*      ACCESS:   public
*
*  PARAMETERS:	 void
*
*     RETURNS:   void
*

*		 DATE:   2016/12/23
*
*******************************************************************/
void hw_CanInit(uint8_t maxCanNodes_u8)
{
    //Install CAN driver
    ESP_ERROR_CHECK(can_driver_install(&g_config, &t_config, &f_config));
    ESP_LOGI(CANBUS_TAG, "Driver installed");

    ESP_ERROR_CHECK(can_start());
    ESP_LOGI(CANBUS_TAG, "Driver started");
}


/*******************************************************************
*       public  Function hw_CanSendMsg
********************************************************************
*
*    FUNCTION:   hw_CanSendMsg
*	FULL NAME:	 hw_CanSendMsg
*
* DESCRIPTION:	 iso_u8
*			   	 				 			 
*      ACCESS:   public 
*
*  PARAMETERS:	 iso_u8 u8CanNode
*  PARAMETERS:	 iso_u32 u32MsgId
*  PARAMETERS:	 const iso_u8 au8MsgData[]
*  PARAMETERS:	 iso_u8 u8Dlc
*
*     RETURNS:   iso_s16
*

*		 DATE:   2016/12/29
*
*******************************************************************/
int16_t hw_CanSendMsg(uint8_t canNode_u8, uint32_t canId_u32, const uint8_t canData_au8[], uint8_t canDataLength_u8)
{
   can_message_t can_msg_send;
   switch (canNode_u8)
   {
		case ISO_CAN_NODE1:
		case ISO_CAN_NODE2:
		default:
			break;
   }
   int16_t  ret_16 = 0;
   int iLoop = 0;
   can_msg_send.identifier = canId_u32;
   can_msg_send.data_length_code = canDataLength_u8;
   can_msg_send.flags = CAN_MSG_FLAG_EXTD; /* extended */

   for (iLoop = 0; (iLoop < 8) && (iLoop < canDataLength_u8); iLoop++)
      can_msg_send.data[iLoop] = canData_au8[iLoop];


   esp_err_t ok_q = can_transmit(&can_msg_send, pdMS_TO_TICKS(1));
   //ESP_LOGI(CANBUS_TAG, "can_transmit");
   HW_CanMsgPrint(canNode_u8, &can_msg_send, 0u);
   if (ok_q != ESP_OK)
   {
      ret_16 = -6; /* E_OVERFLOW */
      hw_DebugPrint("Tx error: %x %x \n", can_msg_send.identifier, can_msg_send.data[0]);
   }
   return ret_16;
}

/*******************************************************************
*       public  Function hw_CanReadMsg
********************************************************************
*
*    FUNCTION:   hw_CanReadMsg
*	FULL NAME:	 hw_CanReadMsg
*
* DESCRIPTION:	 iso_u8
*			   	 				 			 
*      ACCESS:   public 
*
*  PARAMETERS:	 iso_u8 canNode_u8
*  PARAMETERS:	 iso_u32 * canId_pu32
*  PARAMETERS:	 iso_u8 canData_pau8[]
*  PARAMETERS:	 iso_u8 * canDataLength_pu8
*
*     RETURNS:   iso_s16
*

*		 DATE:   2016/12/29
*
*******************************************************************/
int16_t hw_CanReadMsg(uint8_t canNode_u8, uint32_t *canId_pu32, uint8_t canData_pau8[], uint8_t *canDataLength_pu8)
{
   can_message_t can_msg_read;
   switch (canNode_u8)
   {
		case ISO_CAN_NODE1:
		case ISO_CAN_NODE2:
		default:
			break;
   }
	
   if (can_receive(&can_msg_read, pdMS_TO_TICKS(1)) == ESP_OK)
   {
      if (can_msg_read.identifier != 0xCCCCCCCCuL)
      {
         HW_CanMsgPrint(canNode_u8, &can_msg_read, 1u);
         *canId_pu32 = can_msg_read.identifier;
         *canDataLength_pu8 = can_msg_read.data_length_code;
         for (uint8_t i_u8 = 0u; i_u8 < can_msg_read.data_length_code; i_u8++)
         {
            canData_pau8[i_u8] = can_msg_read.data[i_u8];
         }
         //ESP_LOGI(CANBUS_TAG, "can_receive OK");
         return 1;
      }
   }
   //ESP_LOGI(CANBUS_TAG, "can_receive ERROR");
   return 0;
}

int16_t  hw_CanGetFreeSendMsgBufferSize(uint8_t canNode_u8)
{  /* we return always 20 free buffer entries.... */
   return 20;
}



void hw_CanClose(void)
{
    //Uninstall CAN driver
    ESP_ERROR_CHECK(can_stop());
    ESP_LOGI(CANBUS_TAG, "Driver stopped");

    //Uninstall CAN driver
    ESP_ERROR_CHECK(can_driver_uninstall());
    ESP_LOGI(CANBUS_TAG, "Driver uninstalled");
}

static void HW_CanMsgPrint(uint8_t canNode_u8, can_message_t* can_msg_ps, uint8_t isRX)
{
//   const char_t *pcRxTx;
//
//   pcRxTx = (isRX > 0u) ? "Rx" : "Tx";
//   printf("%2u %12d %2s %8x %1u ", canNode_u8, can_msg_ps->TimeStamp, pcRxTx, can_msg_ps->ID, can_msg_ps->DLC);
//
//   if (can_msg_ps->data_length_code == 3)
//   {
//      printf("%2.2x %2.2x %2.2x\n", can_msg_ps->data[0], can_msg_ps->data[1], can_msg_ps->data[2]);
//   }
//   else
//   {
//      printf("%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n",
//         can_msg_ps->data[0], can_msg_ps->data[1], can_msg_ps->data[2], can_msg_ps->data[3],
//         can_msg_ps->data[4], can_msg_ps->data[5], can_msg_ps->data[6], can_msg_ps->data[7]);
//   }
}



/* ************************************************************************ */
