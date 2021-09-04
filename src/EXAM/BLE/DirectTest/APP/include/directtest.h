/********************************** (C) COPYRIGHT *******************************
* File Name          : directtest..h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/11
* Description        : 
*******************************************************************************/

#ifndef TEST_H
#define TEST_H

#ifdef __cplusplus
extern "C"
{
#endif



/*********************************************************************
 * FUNCTIONS
 */
 #define  TEST_EVENT    1
  
extern void TEST_Init( void );
 
extern tmosEvents TEST_ProcessEvent( tmosTaskID task_id, tmosEvents events );
  
/*******************************************************************************
 * @fn          API_LE_ReceiverTestCmd API
 *
 * @brief       used to start a test where the DUT generates test reference packets at a fixed interval
 *
 * input parameters
 *
 * @param       tx_channel - RF channel(0-39)
 *              len        - �������ݰ�����(1-255)
 *              payload    - ���Ե���������(0-7),8���������͡�
 *                           0��PRBS9       1��ȫ��Ϊ0xF0  2��ȫ��Ϊ0xAA  3��PRBS15  
 *                           4��ȫ��Ϊ0xFF  5��ȫ��Ϊ0x00  6��ȫ��Ϊ0x0F  7��ȫ��Ϊ0x55 
 *              tx_power   - bit0~5: ���Եķ��͹��ʣ�1-31����Ӧ��-20dBM��5dBm��
 *                           bit7: 1-�ذ׻�(�����ַ0x71764129);0-���׻�(�����ַ0x8E86BED9)
 *
 * output parameters
 *
 * @param       None
 *
 * @return      Command Status.
 *
 * Command Complete event
 */
extern u8 API_LE_TransmitterTestCmd( u8 tx_channel, u8 len, u8 payload, u8 tx_power );

/*******************************************************************************
 * @fn          API_LE_TestEndCmd API
 *
 * @brief       used to stop any test which is in progress
 *
 * input parameters
 *
 * @param       None
 *
 * output parameters
 *
 * @param       None
 *
 * @return      Command Status.
 *
 * Command Complete event
 */
extern u8 API_LE_TestEndCmd( void  );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
