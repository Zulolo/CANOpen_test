#include "stdio.h"
#include "cmsis_os.h"

#include "canfestival.h"
#include "can_STM32.h"
#include "ARM_OD.h"

/*store date CAN receive*/
xQueueHandle xQ_CAN_MSG = NULL;
xTaskHandle  xT_CANOpen = NULL;

CO_Data *CO_CAN1;
extern CAN_HandleTypeDef hcan1;

void canopen_dataprocess_thread(void * pvParameters)
{
	int i;
	Message RxMSG = Message_Initializer; /* 标准的CAN2.0A信息，初始化清零 */
	CANOpen_Message CAN_Rx_m;            /* CANOpen Message 包含CAN Port(CANx) */

	/* CANOpen Initialising */
	initTimer();
	setNodeId (&ARM_OD_Data, 0xA5);

	CO_CAN1 = &ARM_OD_Data;
	CO_CAN1->canHandle = &hcan1;  //Config CANOpen Port CAN1

	/* State Machine Change to Initialisation and automatic go into Pre_operational*/
	//setState(&ObjDict_CAN2_Data, Initialisation);
	setState(&ARM_OD_Data, Initialisation);
	printf("State Machine change to Initialisation...\r\n");
	/*****************************************************************************
	* Initialisation -> Pre_operational -> StartOrStop ->
	* startSYNC | heartbeatInit | emergencyInit | PDOInit | slaveSendBootUp
	* startSYNC(CO_Data* d)       :监视OD-0x1005、0x1006，并根据其数值设定Alarm;
	* heartbeatInit(CO_Data* d)   :监视OD-0x1017，consumers心跳，而后定Alarm;
	* emergencyInit(CO_Data* d)   :监视OD-0x1003;
	* PDOInit (CO_Data * d)       :监视OD-0x1800，初始化PDO;
	* slaveSendBootUp(CO_Data* d) :发送BootUp指令。
	*****************************************************************************/
	/* State Machine change to Pro_operational */
	printf("State Machine change to Pro_operational...\r\n");
	printf("Slave Send a BootUp Message...\r\n");

	/*create a queue can store 20 data*/
	xQ_CAN_MSG = xQueueCreate(20, sizeof(CANOpen_Message));/* 20个CAN信息？ */

	/* Success Or Fail */
	if(NULL == xQ_CAN_MSG) {
	/*failed to creat the queue*/
		while(1) {	 
			printf("!!!CANOpen Queue created failed!!!\r\n");
			vTaskDelay(100);
		}
	} else {
		printf("CANOpen Queue created successfully...\r\n");
		printf("<<<<<CANOpen Thread ALL CLEAR>>>>>\r\n");
	}

	while(1) {
		if(xQueueReceive(xQ_CAN_MSG, &(CAN_Rx_m), (portTickType)100)) {
			printf("Thread get a CAN package ");

			/* 将队列中的数据存储到RxMSG中 */
			RxMSG.cob_id = (uint16_t)(CAN_Rx_m.m.RxHeader.StdId);
			RxMSG.rtr = CAN_Rx_m.m.RxHeader.RTR;
			RxMSG.len = CAN_Rx_m.m.RxHeader.DLC;
			for(i=0;i<RxMSG.len;i++) {
				RxMSG.data[i] = CAN_Rx_m.m.Data[i]; //Transfer data[0-7] from CAN_Rx_m to RxMSG
			}
			printf("from CAN%u: 0x%x| ", CAN_Rx_m.CANx,CAN_Rx_m.m.RxHeader.StdId);
			for(i=0;i<RxMSG.len;i++) {
				printf("0x%x ", RxMSG.data[i]);
			}
			printf("\r\n");

			/*Handle The Data Receive, 此处和对象字典进行交互*/
			canDispatch(CO_CAN1, &RxMSG); 

			vTaskDelay(50);
		}
	}
}

