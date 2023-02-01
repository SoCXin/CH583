/********************************** (C) COPYRIGHT *******************************
 * File Name          : OTAprofile.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/11
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef OTAPROFILE_H
#define OTAPROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// OTA Profileͨ��Index����
#define OTAPROFILE_CHAR         0

// OTA �����UUID����
#define OTAPROFILE_SERV_UUID    0xFEE0

// OTA ͨѶͨ��UUID����
#define OTAPROFILE_CHAR_UUID    0xFEE1

// Simple Keys Profile Services bit fields
#define OTAPROFILE_SERVICE      0x00000001

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// ��д���������ص�
typedef void (*OTAProfileRead_t)(unsigned char paramID);
typedef void (*OTAProfileWrite_t)(unsigned char paramID, unsigned char *p_data, unsigned char w_len);

typedef struct
{
    OTAProfileRead_t  pfnOTAProfileRead;
    OTAProfileWrite_t pfnOTAProfileWrite;
} OTAProfileCBs_t;

/*********************************************************************
 * API FUNCTIONS
 */

/**
 * @brief   OTA Profile��ʼ��
 *
 * @param   services    - ���������
 *
 * @return  ��ʼ����״̬
 */
bStatus_t OTAProfile_AddService(uint32_t services);

/**
 * @brief   OTA Profile��д�ص�����ע��
 *
 * @param   appCallbacks    - �����ṹ��ָ��
 *
 * @return  ����ִ��״̬
 */
bStatus_t OTAProfile_RegisterAppCBs(OTAProfileCBs_t *appCallbacks);

/**
 * @brief   OTA Profileͨ����������
 *
 * @param   paramID     - OTAͨ��ѡ��
 * @param   p_data      - ����ָ��
 * @param   send_len    - �������ݳ���
 *
 * @return  ����ִ��״̬
 */
bStatus_t OTAProfile_SendData(unsigned char paramID, unsigned char *p_data, unsigned char send_len);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
