/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
#include "CONFIG.h"
#include "RF_PHY.h"
#include "HAL.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
uint8_t taskID;
uint8_t TX_DATA[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

/*********************************************************************
 * @fn      RF_2G4StatusCallBack
 *
 * @brief   RF ״̬�ص���ע�⣺�����ڴ˺�����ֱ�ӵ���RF���ջ��߷���API����Ҫʹ���¼��ķ�ʽ����
 *
 * @param   sta     - ״̬����
 * @param   crc     - crcУ����
 * @param   rxBuf   - ����bufָ��
 *
 * @return  none
 */
void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
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
            if (crc == 0) {
                uint8_t i;

                PRINT("tx recv,rssi:%d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);

                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
            break;
        }
        case TX_MODE_RX_TIMEOUT: // Timeout is about 200us
        {
            break;
        }
        case TX_MODE_HOP_SHUT:
        {
            PRINT("TX_MODE_HOP_SHUT...\n");
            tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_TX_EVT);
            break;
        }

        case RX_MODE_RX_DATA:
        {
            if (crc == 0) {
                uint8_t i;

                PRINT("rx recv, rssi: %d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);
                
                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
            break;
        }
        case RX_MODE_TX_FINISH:
        {
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
            break;
        }
        case RX_MODE_TX_FAIL:
        {
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
            break;
        }
        case RX_MODE_HOP_SHUT:
        {
            PRINT("RX_MODE_HOP_SHUT...\n");
            tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_RX_EVT);
            break;
        }
    }
}

/*********************************************************************
 * @fn      RF_ProcessEvent
 *
 * @brief   RF �¼�����
 *
 * @param   task_id - ����ID
 * @param   events  - �¼���־
 *
 * @return  δ����¼�
 */
uint16_t RF_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if(events & SBP_RF_START_DEVICE_EVT)
    {
        tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1000);
        return events ^ SBP_RF_START_DEVICE_EVT;
    }
    if(events & SBP_RF_PERIODIC_EVT)
    {
        RF_Shut();
        TX_DATA[0]--;
        RF_Tx(TX_DATA, 10, 0xFF, 0xFF);
        tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1000);
        return events ^ SBP_RF_PERIODIC_EVT;
    }
    if(events & SBP_RF_RF_RX_EVT)
    {
        uint8_t state;
        RF_Shut();
        TX_DATA[0]++;
        state = RF_Rx(TX_DATA, 10, 0xFF, 0xFF);
        return events ^ SBP_RF_RF_RX_EVT;
    }
    // ������Ƶ����
    if(events & SBP_RF_CHANNEL_HOP_TX_EVT)
    {
        PRINT("\n------------- hop tx...\n");
        if(RF_FrequencyHoppingTx(16))
        {
            tmos_start_task(taskID, SBP_RF_CHANNEL_HOP_TX_EVT, 100);
        }
        else
        {
            tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1000);
        }
        return events ^ SBP_RF_CHANNEL_HOP_TX_EVT;
    }
    // ������Ƶ����
    if(events & SBP_RF_CHANNEL_HOP_RX_EVT)
    {
        PRINT("hop rx...\n");
        if(RF_FrequencyHoppingRx(200))
        {
            tmos_start_task(taskID, SBP_RF_CHANNEL_HOP_RX_EVT, 400);
        }
        else
        {
            RF_Rx(TX_DATA, 10, 0xFF, 0xFF);
        }
        return events ^ SBP_RF_CHANNEL_HOP_RX_EVT;
    }
    return 0;
}

/*********************************************************************
 * @fn      RF_Init
 *
 * @brief   RF ��ʼ��
 *
 * @return  none
 */
void RF_Init(void)
{
    uint8_t    state;
    rfConfig_t rfConfig;

    tmos_memset(&rfConfig, 0, sizeof(rfConfig_t));
    taskID = TMOS_ProcessEventRegister(RF_ProcessEvent);
    rfConfig.accessAddress = 0x71764129; // ��ֹʹ��0x55555555�Լ�0xAAAAAAAA ( ���鲻����24��λ��ת���Ҳ�����������6��0��1 )
    rfConfig.CRCInit = 0x555555;
    rfConfig.ChannelMap = 0xFFFFFFFF;
    rfConfig.LLEMode = LLE_MODE_AUTO;
    rfConfig.rfStatusCB = RF_2G4StatusCallBack;
    rfConfig.RxMaxlen = 251;
#if (CLK_OSC32K != 0)
    //It is better to choose a shorter heartbeat interval for the internal clock.
    rfConfig.HeartPeriod = 4;
#endif
    state = RF_Config(&rfConfig);
    PRINT("rf 2.4g init: %x\n", state);
    //    { // RX mode
    //        PRINT("RX mode...\n");
    //        tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_RX_EVT);
    //    }

    { // TX mode
        PRINT("TX mode...\n");
        tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_TX_EVT);
    }
}

/******************************** endfile @ main ******************************/
