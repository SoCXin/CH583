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
#include "app_flag.h"

/*********************************************************************
 * @fn      SwitchImageFlag
 *
 * @brief   �л�dataflash���Flag,�����л���IAP���򽫸�flag����ΪImageFlag_USER_PROGRAM_CALL_IAP����ת��0λ�ü���
 *
 * @param   new_flag    - �л���Flag
 *
 * @return  none
 */
void SwitchImageFlag(uint8_t new_flag)
{
    UINT16 i;
    UINT32 ver_flag;
    __attribute__((aligned(4)))   IAPDataFlashInfo_t imgFlag;
    /* ��ȡ��һ�� */
    EEPROM_READ(IAP_FLAG_DATAFLASH_ADD, (PUINT32) &imgFlag, 4);
    if (imgFlag.ImageFlag != new_flag)
    {
        /* ������һ�� */
        EEPROM_ERASE(IAP_FLAG_DATAFLASH_ADD, EEPROM_PAGE_SIZE);

        /* ����Image��Ϣ */
        imgFlag.ImageFlag = new_flag;

        /* ���DataFlash */
        EEPROM_WRITE(IAP_FLAG_DATAFLASH_ADD, (PUINT32) &imgFlag, 4);
    }
}

/*********************************************************************
 * @fn      jumpToIap
 *
 * @brief   ��ת��IAP
 *
 * @return  none
 */
void jumpToIap(void)
{
    uint32_t irq_status;
    SwitchImageFlag(FLAG_USER_CALL_IAP);//���IAP���ð�����⣬���þ仰ע�͵�
    SYS_DisableAllIrq(&irq_status);
    SYS_ResetExecute();
}
