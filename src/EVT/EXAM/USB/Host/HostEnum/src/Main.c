/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/01/25
 * Description        : USB�豸ö��
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#include "CH58x_common.h"

__attribute__((aligned(4))) uint8_t RxBuffer[MAX_PACKET_SIZE]; // IN, must even address
__attribute__((aligned(4))) uint8_t TxBuffer[MAX_PACKET_SIZE]; // OUT, must even address

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main()
{
    uint8_t i, s, k, len, endp;
    uint16_t  loc;

    SetSysClock(CLK_SOURCE_PLL_60MHz);
    DelayMs(5);
    /* ������ѹ��� */
    PowerMonitor(ENABLE, HALevel_2V1);

    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    PRINT("Start @ChipID=%02X\n", R8_CHIP_ID);

    pHOST_RX_RAM_Addr = RxBuffer;
    pHOST_TX_RAM_Addr = TxBuffer;
    USB_HostInit();
    PRINT("Wait Device In\n");
    while(1)
    {
        s = ERR_SUCCESS;
        if(R8_USB_INT_FG & RB_UIF_DETECT)
        { // �����USB��������ж�����
            R8_USB_INT_FG = RB_UIF_DETECT;
            s = AnalyzeRootHub();
            if(s == ERR_USB_CONNECT)
                FoundNewDev = 1;
        }

        if(FoundNewDev || s == ERR_USB_CONNECT)
        { // ���µ�USB�豸����
            FoundNewDev = 0;
            mDelaymS(200);        // ����USB�豸�ղ�����δ�ȶ�,�ʵȴ�USB�豸���ٺ���,������ζ���
            s = InitRootDevice(); // ��ʼ��USB�豸
            if(s != ERR_SUCCESS)
            {
                PRINT("EnumAllRootDev err = %02X\n", (uint16_t)s);
            }
        }

        /* ����¶����ӵ���HUB������ö��HUB */
        s = EnumAllHubPort(); // ö������ROOT-HUB�˿����ⲿHUB��Ķ���USB�豸
        if(s != ERR_SUCCESS)
        { // ������HUB�Ͽ���
            PRINT("EnumAllHubPort err = %02X\n", (uint16_t)s);
        }

        /* ����豸����� */
        loc = SearchTypeDevice(DEV_TYPE_MOUSE); // ��ROOT-HUB�Լ��ⲿHUB���˿�������ָ�����͵��豸���ڵĶ˿ں�
        if(loc != 0xFFFF)
        { // �ҵ���,���������MOUSE��δ���?
            i = (uint8_t)(loc >> 8);
            len = (uint8_t)loc;
            SelectHubPort(len);                                                // ѡ�����ָ����ROOT-HUB�˿�,���õ�ǰUSB�ٶ��Լ��������豸��USB��ַ
            endp = len ? DevOnHubPort[len - 1].GpVar[0] : ThisUsbDev.GpVar[0]; // �ж϶˵�ĵ�ַ,λ7����ͬ����־λ
            if(endp & USB_ENDP_ADDR_MASK)
            {                                                                                                       // �˵���Ч
                s = USBHostTransact(USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? RB_UH_R_TOG | RB_UH_T_TOG : 0, 0); // ��������,��ȡ����,NAK������
                if(s == ERR_SUCCESS)
                {
                    endp ^= 0x80; // ͬ����־��ת
                    if(len)
                        DevOnHubPort[len - 1].GpVar[0] = endp; // ����ͬ����־λ
                    else
                        ThisUsbDev.GpVar[0] = endp;
                    len = R8_USB_RX_LEN; // ���յ������ݳ���
                    if(len)
                    {
                        PRINT("Mouse data: ");
                        for(i = 0; i < len; i++)
                        {
                            PRINT("x%02X ", (uint16_t)(RxBuffer[i]));
                        }
                        PRINT("\n");
                    }
                }
                else if(s != (USB_PID_NAK | ERR_USB_TRANSFER))
                {
                    PRINT("Mouse error %02x\n", (uint16_t)s); // �����ǶϿ���
                }
            }
            else
            {
                PRINT("Mouse no interrupt endpoint\n");
            }
            SetUsbSpeed(1); // Ĭ��Ϊȫ��
        }

        /* ����豸�Ǽ��� */
        loc = SearchTypeDevice(DEV_TYPE_KEYBOARD); // ��ROOT-HUB�Լ��ⲿHUB���˿�������ָ�����͵��豸���ڵĶ˿ں�
        if(loc != 0xFFFF)
        { // �ҵ���,���������KeyBoard��δ���?
            i = (uint8_t)(loc >> 8);
            len = (uint8_t)loc;
            SelectHubPort(len);                                                // ѡ�����ָ����ROOT-HUB�˿�,���õ�ǰUSB�ٶ��Լ��������豸��USB��ַ
            endp = len ? DevOnHubPort[len - 1].GpVar[0] : ThisUsbDev.GpVar[0]; // �ж϶˵�ĵ�ַ,λ7����ͬ����־λ
            if(endp & USB_ENDP_ADDR_MASK)
            {                                                                                                       // �˵���Ч
                s = USBHostTransact(USB_PID_IN << 4 | endp & 0x7F, endp & 0x80 ? RB_UH_R_TOG | RB_UH_T_TOG : 0, 0); // CH554��������,��ȡ����,NAK������
                if(s == ERR_SUCCESS)
                {
                    endp ^= 0x80; // ͬ����־��ת
                    if(len)
                        DevOnHubPort[len - 1].GpVar[0] = endp; // ����ͬ����־λ
                    else
                        ThisUsbDev.GpVar[0] = endp;
                    len = R8_USB_RX_LEN; // ���յ������ݳ���
                    if(len)
                    {
                        SETorOFFNumLock(RxBuffer);
                        PRINT("keyboard data: ");
                        for(i = 0; i < len; i++)
                        {
                            PRINT("x%02X ", (uint16_t)(RxBuffer[i]));
                        }
                        PRINT("\n");
                    }
                }
                else if(s != (USB_PID_NAK | ERR_USB_TRANSFER))
                {
                    PRINT("keyboard error %02x\n", (uint16_t)s); // �����ǶϿ���
                }
            }
            else
            {
                PRINT("keyboard no interrupt endpoint\n");
            }
            SetUsbSpeed(1); // Ĭ��Ϊȫ��
        }
    }
}
