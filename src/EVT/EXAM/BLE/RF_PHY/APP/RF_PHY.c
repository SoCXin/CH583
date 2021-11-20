/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2020/08/06
* Description        : 
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include "CONFIG.h"
#include "CH58x_common.h"
#include "RF_PHY.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
uint8 taskID;
uint8 TX_DATA[300] ={1,2,3,4,5,6,7,8,9,0};


/*******************************************************************************
* Function Name  : RF_2G4StatusCallBack
* Description    : RF ״̬�ص���ע�⣺�����ڴ˺�����ֱ�ӵ���RF���ջ��߷���API����Ҫʹ���¼��ķ�ʽ����
* Input          : sta - ״̬����
*                   crc - crcУ����
*                   rxBuf - ����bufָ��
* Output         : None
* Return         : None
*******************************************************************************/
void RF_2G4StatusCallBack( uint8 sta , uint8 crc, uint8 *rxBuf )
{
  switch( sta )
	{
    case TX_MODE_TX_FINISH:
    {
      break;
    }
    case TX_MODE_TX_FAIL:
    {
      break;
    }		
    case TX_MODE_RX_DATA:
    {
      RF_Shut();
      if( crc == 1 )
			{
        PRINT("crc error\n");
      }
			else if( crc == 2 )
			{
        PRINT("match type error\n");
      }
 			else
			{
        uint8 i;      
        PRINT("tx recv,rssi:%d\n",(s8)rxBuf[0]);
        PRINT("len:%d-",rxBuf[1]);
        for(i=0;i<rxBuf[1];i++) PRINT("%x ",rxBuf[i+2]);
        PRINT("\n");
      }
      break;
    }
    case TX_MODE_RX_TIMEOUT:		// Timeout is about 200us
    {
      break;
    }		
    case RX_MODE_RX_DATA:
    {
      if( crc == 1 )
			{
        PRINT("crc error\n");
      }
			else if( crc == 2 )
			{
        PRINT("match type error\n");
      }
			else
      {
        uint8 i;      
        PRINT("rx recv, rssi: %d\n",(s8)rxBuf[0]);
        PRINT("len: %d-",rxBuf[1]);
        for(i=0;i<rxBuf[1];i++) PRINT("%x ",rxBuf[i+2]);
        PRINT("\n");
      }
      tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
      break;
    }
    case RX_MODE_TX_FINISH:
    {
      tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
      break;
    }
    case RX_MODE_TX_FAIL:
    {
      break;
    }		
  }
  PRINT("STA: %x\n",sta);
}


/*******************************************************************************
* Function Name  : RF_ProcessEvent
* Description    : RF �¼�����
* Input          : task_id - ����ID
*                   events - �¼���־
* Output         : None
* Return         : None
*******************************************************************************/
uint16 RF_ProcessEvent( uint8 task_id, uint16 events )
{
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = tmos_msg_receive( task_id )) != NULL )
    {
      // Release the TMOS message
      tmos_msg_deallocate( pMsg );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  if( events & SBP_RF_START_DEVICE_EVT )
	{
    tmos_start_task( taskID , SBP_RF_PERIODIC_EVT ,1000 );
    return events^SBP_RF_START_DEVICE_EVT;
  }
  if ( events & SBP_RF_PERIODIC_EVT )
	{
    RF_Shut( );
    TX_DATA[0]++;

    RF_Tx( TX_DATA,TX_DATA[0], 0xFF, 0xFF );
    tmos_start_task( taskID , SBP_RF_PERIODIC_EVT ,160 );
    return events^SBP_RF_PERIODIC_EVT;
  }
  if( events & SBP_RF_RF_RX_EVT )
  {
    uint8 state;
    RF_Shut();
    TX_DATA[0]=100;
    state = RF_Rx( TX_DATA,TX_DATA[0], 0xFF, 0xFF );
    PRINT("RX mode.state = %x\n",state);
    return events^SBP_RF_RF_RX_EVT;
  }
  return 0;
}

/*******************************************************************************
* Function Name  : RF_Init
* Description    : RF ��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RF_Init( void )
{
  uint8 state;
  rfConfig_t rfConfig;

  tmos_memset( &rfConfig, 0, sizeof(rfConfig_t) );
  taskID = TMOS_ProcessEventRegister( RF_ProcessEvent );
  rfConfig.accessAddress = 0x8E89bed6;	// ��ֹʹ��0x55555555�Լ�0xAAAAAAAA ( ���鲻����24��λ��ת���Ҳ�����������6��0��1 )
  rfConfig.CRCInit = 0x555555;
  rfConfig.Channel = 39;
  rfConfig.Frequency = 2480000;
  rfConfig.LLEMode = LLE_MODE_BASIC; // ʹ�� LLE_MODE_EX_CHANNEL ��ʾ ѡ�� rfConfig.Frequency ��Ϊͨ��Ƶ��
  rfConfig.rfStatusCB = RF_2G4StatusCallBack;
  state = RF_Config( &rfConfig );
  PRINT("rf 2.4g init: %x\n",state);
	{ // RX mode
		state = RF_Rx( TX_DATA,10, 0xFF, 0xFF );
		PRINT("RX mode.state = %x\n",state);
	}

//	{ // TX mode
//		tmos_set_event( taskID , SBP_RF_PERIODIC_EVT );
//	}
}

/******************************** endfile @ main ******************************/
