/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_adapter_csma_mac.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/20
 * Description        : lwns��������ģ��csma��macЭ��
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef _LWNS_ADAPTER_CSMA_MAC_H_
#define _LWNS_ADAPTER_CSMA_MAC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lwns_config.h"

#define LWNS_USE_CSMA_MAC    0  //�Ƿ�ʹ��ģ��csma��macЭ�飬ע��ֻ��ʹ��һ��mac��Э�顣

#if LWNS_USE_CSMA_MAC

struct csma_mac_phy_manage_struct
{
    struct csma_mac_phy_manage_struct *next;
    uint8_t                           *data;
}; //ģ��csma mac�㷢�͹���ṹ��

typedef enum
{
    BLE_PHY_MANAGE_STATE_FREE = 0,
    BLE_PHY_MANAGE_STATE_RECEIVED,
    BLE_PHY_MANAGE_STATE_WAIT_SEND,
    BLE_PHY_MANAGE_STATE_SENDING,
} BLE_PHY_MANAGE_STATE_t;

  #define LWNS_MAC_TRANSMIT_TIMES          2                      //һ�η��ͣ�����Ӳ�����ͼ���

  #define LWNS_MAC_PERIOD_MS               20                     //mac���ͽ��ռ�����ڣ��������Ҫ���͵����ݰ�����ʼ����ӳټ���ͻ��//Ϊ(1000/HTIMER_SECOND_NUM)

  #define LWNS_MAC_SEND_DELAY_MAX_625US    LWNS_NEIGHBOR_MAX_NUM  //����ӳ٣���ֵԽС��Խ���׳��ִ����Ƽ���һЩ��8���Ը��ʻ����ԡ�

  #define BLE_PHY_ONE_PACKET_MAX_625US     5

  #define LWNS_MAC_SEND_PACKET_MAX_NUM     8                      //�����������֧�ּ������ݰ��ȴ�����

  #define LWNS_MAC_SEND_DELAY_MAX_TIMES    LWNS_NEIGHBOR_MAX_NUM / 2 //�ڷ��ͱ�ȡ�����ӳټ��κ󣬲�������ȴ������̷���

  #define LLE_MODE_ORIGINAL_RX             (0x80) //�������LLEMODEʱ���ϴ˺꣬����յ�һ�ֽ�Ϊԭʼ���ݣ�ԭ��ΪRSSI��

    //RF_TX��RF_RX���õ����ͣ������޸ģ����Ƽ���
  #define USER_RF_RX_TX_TYPE               0xff

  #define LWNS_PHY_OUTPUT_TIMEOUT_MS       5
    //receive process evt
  #define LWNS_PHY_RX_OPEN_EVT             1
    //send process evt
  #define LWNS_PHY_PERIOD_EVT              1
  #define LWNS_PHY_OUTPUT_EVT              2
  #define LWNS_PHY_OUTPUT_FINISH_EVT       4

extern void RF_Init(void);

extern void lwns_init(void);

extern void lwns_shut(void);

extern void lwns_start(void);

#endif /* LWNS_USE_CSMA_MAC */

#ifdef __cplusplus
}
#endif

#endif /* _LWNS_ADAPTER_CSMA_MAC_H_ */
