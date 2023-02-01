/********************************** (C) COPYRIGHT *******************************
 * File Name          : EXAM11.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/11
 * Description        : CH573 C���Ե�U��Ŀ¼�ļ�ö�ٳ���
 * ֧��: FAT12/FAT16/FAT32
 * ע����� CHRV3UFI.LIB/USBHOST.C/DEBUG.C
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/** ��ʹ��U���ļ�ϵͳ�⣬��Ҫ�ڹ�������Ԥ�������޸� DISK_LIB_ENABLE=0        */
/** U�̹���USBhub���棬��Ҫ�ڹ�������Ԥ�������޸� DISK_WITHOUT_USB_HUB=0  */

#include "CH58x_common.h"
#include "CHRV3UFI.H"

__attribute__((aligned(4))) uint8_t RxBuffer[MAX_PACKET_SIZE]; // IN, must even address
__attribute__((aligned(4))) uint8_t TxBuffer[MAX_PACKET_SIZE]; // OUT, must even address

/*********************************************************************
 * @fn      mStopIfError
 *
 * @brief   ������״̬,�����������ʾ������벢ͣ��
 *
 * @param   iError  - ������
 *
 * @return  none
 */
void mStopIfError(uint8_t iError)
{
    if(iError == ERR_SUCCESS)
    {
        return; /* �����ɹ� */
    }
    PRINT("Error: %02X\n", (uint16_t)iError); /* ��ʾ���� */
    /* ���������,Ӧ�÷����������Լ�CHRV3DiskStatus״̬,�������CHRV3DiskReady��ѯ��ǰU���Ƿ�����,���U���ѶϿ���ô�����µȴ�U�̲����ٲ���,
     ��������Ĵ�����:
     1������һ��CHRV3DiskReady,�ɹ����������,����Open,Read/Write��
     2�����CHRV3DiskReady���ɹ�,��ôǿ�н���ͷ��ʼ����(�ȴ�U�����ӣ�CHRV3DiskReady��) */
    while(1)
    {
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   ������
 *
 * @return  none
 */
int main()
{
    uint8_t  s, i;
    uint8_t *pCodeStr;
    uint16_t j;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    PRINT("Start @ChipID=%02X \n", R8_CHIP_ID);

    pHOST_RX_RAM_Addr = RxBuffer;
    pHOST_TX_RAM_Addr = TxBuffer;
    USB_HostInit();
    CHRV3LibInit(); //��ʼ��U�̳������֧��U���ļ�

    FoundNewDev = 0;
    PRINT("Wait Device In\n");
    while(1)
    {
        s = ERR_SUCCESS;
        if(R8_USB_INT_FG & RB_UIF_DETECT) // �����USB��������ж�����
        {
            R8_USB_INT_FG = RB_UIF_DETECT; // �������жϱ�־
            s = AnalyzeRootHub();          // ����ROOT-HUB״̬
            if(s == ERR_USB_CONNECT)
            {
                FoundNewDev = 1;
            }
        }

        if(FoundNewDev || s == ERR_USB_CONNECT)
        {
            // ���µ�USB�豸����
            FoundNewDev = 0;
            mDelaymS(200);        // ����USB�豸�ղ�����δ�ȶ�,�ʵȴ�USB�豸���ٺ���,������ζ���
            s = InitRootDevice(); // ��ʼ��USB�豸
            if(s == ERR_SUCCESS)
            {
                PRINT("Start UDISK_demo @CHRV3UFI library\n");
                // U�̲������̣�USB���߸�λ��U�����ӡ���ȡ�豸������������USB��ַ����ѡ�Ļ�ȡ������������֮�󵽴�˴�����CHRV3�ӳ���������ɺ�������
                CHRV3DiskStatus = DISK_USB_ADDR;
                for(i = 0; i != 10; i++)
                {
                    PRINT("Wait DiskReady\n");
                    s = CHRV3DiskReady();
                    if(s == ERR_SUCCESS)
                    {
                        break;
                    }
                    mDelaymS(50);
                }
                if(CHRV3DiskStatus >= DISK_MOUNTED) //U��׼����
                {
                    /* ���ļ� */
                    PRINT("Open\n");
                    strcpy((uint8_t *)mCmdParam.Open.mPathName, "/C51/CHRV3HFT.C"); //����Ҫ�������ļ�����·��
                    s = CHRV3FileOpen();                                            //���ļ�
                    if(s == ERR_MISS_DIR)
                    {
                        PRINT("�����ڸ��ļ������г���Ŀ¼�����ļ�\n");
                        pCodeStr = (uint8_t *)"/*";
                    }
                    else
                    {
                        pCodeStr = (uint8_t *)"/C51/*"; //�г�\C51��Ŀ¼�µĵ��ļ�
                    }

                    PRINT("List file %s\n", pCodeStr);
                    for(j = 0; j < 10000; j++) //�޶�10000���ļ�,ʵ����û������
                    {
                        strcpy((uint8_t *)mCmdParam.Open.mPathName, (const uint8_t *)pCodeStr); //�����ļ���,*Ϊͨ���,�����������ļ�������Ŀ¼
                        i = strlen((uint8_t *)mCmdParam.Open.mPathName);
                        mCmdParam.Open.mPathName[i] = 0xFF; //�����ַ������Ƚ��������滻Ϊ���������,��0��254,�����0xFF��255��˵�����������CHRV3vFileSize������
                        CHRV3vFileSize = j;                 //ָ������/ö�ٵ����
                        i = CHRV3FileOpen();                //���ļ�,����ļ����к���ͨ���*,��Ϊ�����ļ�������
                        /* CHRV3FileEnum �� CHRV3FileOpen ��Ψһ�����ǵ����߷���ERR_FOUND_NAMEʱ��ô��Ӧ��ǰ�߷���ERR_SUCCESS */
                        if(i == ERR_MISS_FILE)
                        {
                            break; //��Ҳ��������ƥ����ļ�,�Ѿ�û��ƥ����ļ���
                        }
                        if(i == ERR_FOUND_NAME) //��������ͨ�����ƥ����ļ���,�ļ�����������·�������������
                        {
                            PRINT("  match file %04d#: %s\n", (unsigned int)j, mCmdParam.Open.mPathName); /* ��ʾ��ź���������ƥ���ļ���������Ŀ¼�� */
                            continue;                                                                      /* ����������һ��ƥ����ļ���,�´�����ʱ��Ż��1 */
                        }
                        else //����
                        {
                            mStopIfError(i);
                            break;
                        }
                    }
                    i = CHRV3FileClose(); //�ر��ļ�
                    PRINT("U����ʾ���\n");
                }
                else
                {
                    PRINT("U��û��׼���� ERR =%02X\n", (uint16_t)s);
                }
            }
            else
            {
                PRINT("��ʼ��U��ʧ�ܣ������U������\n");
            }
        }
        mDelaymS(100);  // ģ�ⵥƬ����������
        SetUsbSpeed(1); // Ĭ��Ϊȫ��
    }
}
