/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_flag.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        : USB IAP APP����
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#ifndef _APP_FLAG_H_
#define _APP_FLAG_H_

#include "CH58x_common.h"

void SwitchImageFlag(uint8_t new_flag);
void jumpToIap(void);

#define FLAG_USER_CALL_IAP   0x55
#define FLAG_USER_CALL_APP   0xaa

/* �����DataFlash��ַ������ռ��������λ�� */
#define IAP_FLAG_DATAFLASH_ADD               0

/* �����DataFlash���OTA��Ϣ */
typedef struct
{
    unsigned char ImageFlag;            //��¼�ĵ�ǰ��image��־
    unsigned char Revd[3];
} IAPDataFlashInfo_t;


#endif /* _APP_FLAG_H_ */
