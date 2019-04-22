/*
This file is edited as a part of CanFestival.

STM32F407 Port: Jim
*/

#include "stm32f4xx_hal.h"
#include "can_STM32.h"
#include "canfestival.h"
//#include "misc.h"
extern CAN_HandleTypeDef hcan1;
/**
 * Send a CAN Message Passed from the CANOpen Stack
 * @param  CANx CAN_PORT used 1 or 2.
 * @param  m    Pointer to Message to send
 * @return      0-Failed, 1-Success
 */
unsigned char canSend(CAN_PORT CANx, Message *m)
{
	unsigned char i;
	CAN_TxHeaderTypeDef   TxHeader;
	uint8_t               TxData[8];
	uint32_t              TxMailbox;
	
	TxHeader.StdId = (uint32_t)(m->cob_id);
	TxHeader.ExtId = 0x00;
	TxHeader.RTR = m->rtr;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = m->len; 
	TxHeader.TransmitGlobalTime = DISABLE;	
	for(i=0; i<m->len; i++) {
		TxData[i] = m->data[i];
	}
	if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
		printf("CAN transmit error.\n");
		return 0;
	} else {
		return 1;			
	}     
}

/****************** (C) COPYRIGHT CSST Robot Research Center *****END OF FILE****/
