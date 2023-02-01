/********************************** (C) COPYRIGHT *******************************
 * File Name          : EXAM1.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/11
 * Description        : C���Ե�U�̴������ļ����ļ�����
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

typedef struct __attribute__((packed)) _LONG_NAME
{                               //�ֽڶ���
    uint8_t  LDIR_Ord;          /*���ļ�������ţ����Ϊ0X40���ʾ���һ����*/
    uint16_t LDIR_Name1[5];     /*���ļ�����ǰ5���ֽ�*/
    uint8_t  LDIR_Attr;         /*���Ա���ΪATTR_LONG_NAME*/
    uint8_t  LDIR_Type;         /* Ϊ0��ʾ���ļ���������*/
    uint8_t  LDIR_Chksum;       /*���ļ�����У���*/
    uint16_t LDIR_Name2[6];     /*���ļ�����6-11���ַ�*/
    uint8_t  LDIR_FstClusLO[2]; /*Ϊ0*/
    uint16_t LDIR_Name3[2];     /*���ļ�����12-13���ԡ��ַ�*/

} F_LONG_NAME; /*���峤�ļ���*/

typedef F_LONG_NAME *P_LONG_NAME;

#define MAX_LONG_NAME        4
#define FILE_LONG_NAME       MAX_LONG_NAME * 13 + 1
#define DATA_BASE_BUF_LEN    512

uint8_t DATA_BASE_BUF0[DATA_BASE_BUF_LEN];
uint8_t DATA_BASE_BUF1[DATA_BASE_BUF_LEN];

uint16_t LongFileName[FILE_LONG_NAME]; /*���ļ����ռ�ֻ�洢�ļ�������·��*/

/*********************************************************************
 * @fn      ChkSum
 *
 * @brief   ������ļ�����У���
 *
 * @param   pDir1   - FAT���������ļ�Ŀ¼��Ϣ
 *
 * @return  У���
 */
unsigned char ChkSum(PX_FAT_DIR_INFO pDir1)
{
    unsigned char FcbNameLen;
    unsigned char Sum;
    Sum = 0;
    for(FcbNameLen = 0; FcbNameLen != 11; FcbNameLen++)
    {
        //if(pDir1->DIR_Name[FcbNameLen]==0x20)continue;
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + pDir1->DIR_Name[FcbNameLen];
    }
    return (Sum);
}

/*********************************************************************
 * @fn      mLDirCheck
 *
 * @brief   ������������Ŀ¼��ͳ��ļ����Ƿ���ͬ
 *
 * @param   pDir2   - FAT���������ļ�Ŀ¼��Ϣ
 * @param   pLdir1  - ���ļ���
 *
 * @return  ����00-15Ϊ�ҵ����ļ�����ͬ���ļ�00-15��ʾ��Ӧ���ļ�����Ŀ¼���λ��,
 *          ����0X80-8F��ʾ������Ŀ¼��Ľ�β,�Ժ���δ�õ�Ŀ¼��,����0FF��ʾ��������ƥ���Ŀ¼��
 */
uint8_t mLDirCheck(PX_FAT_DIR_INFO pDir2, F_LONG_NAME *pLdir1)
{
    uint8_t      i, j, k, sum, nodir, nodir1;
    F_LONG_NAME *pLdir2;
    uint16_t    *pLName;
    for(i = 0; i != 16; i++)
    {
        if(pDir2->DIR_Name[0] == 0xe5)
        {
            pDir2 += 1;
            continue;
        } /*������ɾ������������һĿ¼*/ /*�Ǳ�ɾ�����ļ���������*/
        if(pDir2->DIR_Name[0] == 0x00)
        {
            return i | 0x80;
        } /*���������¿ռ�û���ļ��������˳�*/
        if((pDir2->DIR_Attr == 0x0f) | (pDir2->DIR_Attr == 0x8))
        {
            pDir2 += 1;
            continue;
        } /*����ҵ����Ǿ����߳��ļ�������*/
        /*�ҵ�һ�����ļ���*/
        k = i - 1; /*���ļ�����Ӧ�ڶ��ļ�������*/
        if(i == 0)
        {                    /*����˶��ļ����ڱ�������һ��*/
            pLdir2 = pLdir1; /*���ļ���Ӧ����һ���������һ��*/
            k = 15;          /*��¼���ļ���λ��*/
            pLdir2 += 15;    /*ƫ�Ƶ���β*/
        }
        else
            pLdir2 = (F_LONG_NAME *)(pDir2 - 1); /*ȡ���ļ���Ŀ¼��*/
        sum = ChkSum(pDir2);                     /*�����ۼӺ�*/
        pLName = LongFileName;                   /*ָ��ָ���ĳ��ļ���*/
        nodir = 0;                               /*��ʼ����־*/
        nodir1 = 1;
        while(1)
        {
            if((pLdir2->LDIR_Ord != 0xe5) & (pLdir2->LDIR_Attr == ATTR_LONG_NAME) & (pLdir2->LDIR_Chksum == sum))
            { /*�ҵ�һ�����ļ���*/
                for(j = 0; j != 5; j++)
                {
                    if((pLdir2->LDIR_Name1[j] == 0x00) | (*pLName == 0))
                        continue; /*���������ļ�����β*/
                    if((pLdir2->LDIR_Name1[j] == 0xff) | (*pLName == 0))
                        continue; /*���������ļ�����β*/
                    if(pLdir2->LDIR_Name1[j] != *pLName)
                    { /*���������ñ�־*/
                        nodir = 1;
                        break;
                    }
                    pLName++;
                }
                if(nodir == 1)
                    break; /*�ļ�����ͬ�˳�*/
                for(j = 0; j != 6; j++)
                {
                    if((pLdir2->LDIR_Name2[j] == 0x00) | (*pLName == 0))
                        continue;
                    if((pLdir2->LDIR_Name2[j] == 0xff) | (*pLName == 0))
                        continue;
                    if(*pLName != pLdir2->LDIR_Name2[j])
                    {
                        nodir = 1;
                        break;
                    }
                    pLName++;
                }
                if(nodir == 1)
                    break; /*�ļ�����ͬ�˳�*/
                for(j = 0; j != 2; j++)
                {
                    if((pLdir2->LDIR_Name3[j] == 0x00) | (*pLName == 0))
                        continue;
                    if((pLdir2->LDIR_Name3[j] == 0xff) | (*pLName == 0))
                        continue;
                    if(*pLName != pLdir2->LDIR_Name3[j])
                    {
                        nodir = 1;
                        break;
                    }
                    pLName++;
                }
                if(nodir == 1)
                    break; /*�ļ�����ͬ�˳�*/
                if((pLdir2->LDIR_Ord & 0x40) == 0x40)
                {
                    nodir1 = 0;
                    break;
                } /*�ҵ����ļ��������ұȽϽ���*/
            }
            else
                break; /*����������Ӧ�ĳ��ļ����˳�*/
            if(k == 0)
            {
                pLdir2 = pLdir1;
                pLdir2 += 15;
                k = 15;
            }
            else
            {
                k = k - 1;
                pLdir2 -= 1;
            }
        }
        if(nodir1 == 0)
            return i; /*��ʾ�ҵ����ļ��������ض��ļ����ڵ�Ŀ¼��*/
        pDir2 += 1;
    }
    return 0xff; /*ָ��������һ������û�ҵ���Ӧ�ĳ��ļ���*/
}

/*********************************************************************
 * @fn      mChkName
 *
 * @brief   ����ϼ���Ŀ¼����
 *
 * @param   pJ      - ����һ������
 *
 * @return  ״̬
 */
uint8_t mChkName(unsigned char *pJ)
{
    uint8_t i, j;
    j = 0xFF;
    for(i = 0; i != sizeof(mCmdParam.Create.mPathName); i++)
    { /* ���Ŀ¼·�� */
        if(mCmdParam.Create.mPathName[i] == 0)
            break;
        if(mCmdParam.Create.mPathName[i] == PATH_SEPAR_CHAR1 || mCmdParam.Create.mPathName[i] == PATH_SEPAR_CHAR2)
            j = i; /* ��¼�ϼ�Ŀ¼ */
    }
    i = ERR_SUCCESS;
    if((j == 0) || ((j == 2) && (mCmdParam.Create.mPathName[1] == ':')))
    { /* �ڸ�Ŀ¼�´��� */
        mCmdParam.Open.mPathName[0] = '/';
        mCmdParam.Open.mPathName[1] = 0;
        i = CHRV3FileOpen(); /*�򿪸�Ŀ¼*/
        if(i == ERR_OPEN_DIR)
            i = ERR_SUCCESS; /* �ɹ����ϼ�Ŀ¼ */
    }
    else
    {
        if(j != 0xFF)
        { /* ���ھ���·��Ӧ�û�ȡ�ϼ�Ŀ¼����ʼ�غ� */
            mCmdParam.Create.mPathName[j] = 0;
            i = CHRV3FileOpen(); /* ���ϼ�Ŀ¼ */
            if(i == ERR_SUCCESS)
                i = ERR_MISS_DIR; /* ���ļ�����Ŀ¼ */
            else if(i == ERR_OPEN_DIR)
                i = ERR_SUCCESS;                              /* �ɹ����ϼ�Ŀ¼ */
            mCmdParam.Create.mPathName[j] = PATH_SEPAR_CHAR1; /* �ָ�Ŀ¼�ָ��� */
        }
    }
    *pJ = j; /*ָ���з���һ������*/
    return i;
}

/*********************************************************************
 * @fn      CreatLongName
 *
 * @brief   ���������ļ��ĳ��ļ������ڶ��ļ����ռ�����·���Լ��ο����ļ������ڳ��ļ����ռ�������ļ����ļ�����UNICODE����
 *
 * @return  ����00��ʾ�ɹ��������ڶ��ļ����ռ䷵����ʵ�Ķ��ļ���������Ϊ���ɹ�״̬
 */
uint8_t CreatLongName()
{
    uint8_t         ParData[MAX_PATH_LEN]; /**/
    uint16_t        tempSec;               /*����ƫ��*/
    uint8_t         i, j, k, x, sum, y, z;
    P_LONG_NAME     pLDirName;
    PX_FAT_DIR_INFO pDirName, pDirName1;
    BOOL            FBuf;
    uint8_t        *pBuf1;
    uint16_t       *pBuf;
    CHRV3DirtyBuffer();
    for(k = 0; k != MAX_PATH_LEN; k++)
        ParData[k] = mCmdParam.Other.mBuffer[k];
    i = mChkName(&j);
    if(i == ERR_SUCCESS)
    {             /* �ɹ���ȡ�ϼ�Ŀ¼����ʼ�غ� */
        FBuf = 0; /*��ʼ��*/
        tempSec = 0;
        DATA_BASE_BUF1[0] = 0xe5; /*��Ч�ϴλ�����*/
        k = 0xff;
        while(1)
        {                                                                                        /*�����Ƕ�ȡ������Ŀ¼��*/
            pDirName = FBuf ? (PX_FAT_DIR_INFO)DATA_BASE_BUF1 : (PX_FAT_DIR_INFO)DATA_BASE_BUF0; /*���ļ���ָ��ָ�򻺳���*/
            pLDirName = FBuf ? (P_LONG_NAME)DATA_BASE_BUF0 : (P_LONG_NAME)DATA_BASE_BUF1;
            mCmdParam.Read.mSectorCount = 1;                                     /*��ȡһ��������*/
            mCmdParam.Read.mDataBuffer = FBuf ? DATA_BASE_BUF1 : DATA_BASE_BUF0; /*��ǰ������ļ�������,����ʹ��˫�򻺳�����ȥ�����ļ���*/
            FBuf = !FBuf;                                                        /*��������־��ת*/
            i = CHRV3FileRead();
            if(mCmdParam.Read.mSectorCount == 0)
            {
                k = 0xff;
                break;
            }
            tempSec += 1;
            k = mLDirCheck(pDirName, pLDirName);
            z = k;
            z &= 0x0f;
            if(k != 0x0ff)
            {
                break;
            } /*�ҵ��ļ������ҵ��ļ���β�˳�*/
        }
        if(k < 16)
        {
            pDirName += k; /*���ҵ��ļ����ļ����ڴ�Ŀ¼��*/
            if(j != 0xff)
            {
                for(k = 0; k != j + 1; k++)
                    mCmdParam.Other.mBuffer[k] = ParData[k];
            }
            pBuf1 = &mCmdParam.Other.mBuffer[j + 1]; /*ȡ�ļ����ĵ�ַ*/
            //else pBuf1=&mCmdParam.Other.mBuffer;
            for(i = 0; i != 8; i++)
            {
                if(pDirName->DIR_Name[i] == 0x20)
                    continue;
                else
                {
                    *pBuf1 = pDirName->DIR_Name[i];
                    pBuf1++;
                }
            }
            if(pDirName->DIR_Name[i] != 0x20)
            {
                *pBuf1 = '.';
                pBuf1++;
            }
            for(; i != 11; i++)
            {
                if(pDirName->DIR_Name[i] == 0x20)
                    continue;
                else
                {
                    *pBuf1 = pDirName->DIR_Name[i];
                    pBuf1++;
                }

            } /*���ƶ��ļ���*/
            i = CHRV3FileClose();
            i = CHRV3FileCreate(); /*�ɻ�����Ҫ��Ҫ�ָ����ս���˺���ʱ�Ĵغ�*/
            PRINT("k<16\r\n");
            return i; /*�����ļ�,����״̬*/
        }
        else
        { /*��ʾĿ¼��ö�ٵ�����λ�ã�Ҫ�����ļ�*/
            if(k == 0xff)
            {
                z = 00;
                tempSec += 1;
            }
            i = CHRV3FileClose();
            for(k = 0; k != MAX_PATH_LEN; k++)
                mCmdParam.Other.mBuffer[k] = ParData[k]; /*�Դ����ļ����ļ���*/
            for(x = 0x31; x != 0x3a; x++)
            { /*���ɶ��ļ���*/
                for(y = 0x31; y != 0x3a; y++)
                {
                    for(i = 0x31; i != 0x3a; i++)
                    {
                        mCmdParam.Other.mBuffer[j + 7] = i;
                        mCmdParam.Other.mBuffer[j + 6] = '~';
                        mCmdParam.Other.mBuffer[j + 5] = y;
                        mCmdParam.Other.mBuffer[j + 4] = x;
                        if(CHRV3FileOpen() != ERR_SUCCESS)
                            goto XAA1;
                        /**/
                    }
                }
            }
            i = 0xff;
            goto XBB;
        /*�����޷���ȷ����*/
        XAA1:

            i = CHRV3FileCreate();
            if(i != ERR_SUCCESS)
                return i; //{goto XCC;}			/*�������ܼ�������*/
            for(k = 0; k != MAX_PATH_LEN; k++)
                ParData[k] = mCmdParam.Other.mBuffer[k]; /*�Դ����ļ����ļ���*/
            i = mChkName(&j);
            mCmdParam.Locate.mSectorOffset = tempSec - 1;
            i = CHRV3FileLocate();
            if(i != ERR_SUCCESS)
                return i; //{goto XCC;}			/*�������ܼ�������*/
            mCmdParam.Read.mSectorCount = 1;
            mCmdParam.Read.mDataBuffer = DATA_BASE_BUF0;
            pDirName = (PX_FAT_DIR_INFO)DATA_BASE_BUF0;
            pDirName += z;       /*ָ�򴴽��ļ�����ƫ��*/
            i = CHRV3FileRead(); /*��ȡһ�����������ݣ�ȡ��һ��Ŀ¼����ǸղŴ����Ķ��ļ���*/
            if(i != ERR_SUCCESS)
                return i; //{goto XCC;}				/*����Ҫ����������*/

            for(i = 0; i != FILE_LONG_NAME; i++)
            {
                if(LongFileName[i] == 00)
                    break; /*���㳤�ļ����ĳ���*/
            }
            for(k = i + 1; k != FILE_LONG_NAME; k++)
            { /*����Ч��Ŀ¼�����*/
                LongFileName[k] = 0xffff;
            }
            k = i / 13; /*ȡ���ļ�������*/
            i = i % 13;
            if(i != 0)
                k = k + 1; /*����������һ��*/
            i = k;
            k = i + z; /*zΪ���ļ�ƫ��,z-1Ϊ���ļ�ƫ��*/
            if(k < 16)
            {
                pDirName1 = (PX_FAT_DIR_INFO)DATA_BASE_BUF0;
                pDirName1 += k;
                sum = ChkSum(pDirName1); /*�����ۼӺ�*/
                pLDirName = (P_LONG_NAME)DATA_BASE_BUF0;
                pLDirName += (k - 1);
            }
            else if(k == 16)
            {
                pDirName1 = (PX_FAT_DIR_INFO)DATA_BASE_BUF1;
                pDirName1 += (k - 16);
                pLDirName = (F_LONG_NAME *)DATA_BASE_BUF0;
                pLDirName += (k - 1);
            }
            else if(k > 16)
            {
                pDirName1 = (PX_FAT_DIR_INFO)DATA_BASE_BUF1;
                pDirName1 += (k - 16);
                pLDirName = (F_LONG_NAME *)DATA_BASE_BUF1;
                pLDirName += (k - 1 - 16);
            }
            /*���ƶ��ļ���,�����ļ������Ƶ�ָ������*/
            pDirName1->DIR_NTRes = pDirName->DIR_NTRes;
            pDirName1->DIR_CrtTimeTenth = pDirName->DIR_CrtTimeTenth;
            pDirName1->DIR_CrtTime = pDirName->DIR_CrtTime;
            pDirName1->DIR_CrtDate = pDirName->DIR_CrtDate;
            pDirName1->DIR_LstAccDate = pDirName->DIR_LstAccDate;
            pDirName1->DIR_FstClusHI = pDirName->DIR_FstClusHI;
            pDirName1->DIR_WrtTime = pDirName->DIR_WrtTime;
            pDirName1->DIR_WrtDate = pDirName->DIR_WrtDate;
            pDirName1->DIR_FstClusLO = pDirName->DIR_FstClusLO;
            pDirName1->DIR_FileSize = pDirName->DIR_FileSize;
            pDirName1->DIR_Attr = pDirName->DIR_Attr;

            pDirName1->DIR_Name[0] = pDirName->DIR_Name[0];
            pDirName1->DIR_Name[1] = pDirName->DIR_Name[1];
            pDirName1->DIR_Name[2] = pDirName->DIR_Name[2];
            pDirName1->DIR_Name[3] = pDirName->DIR_Name[3];
            pDirName1->DIR_Name[4] = pDirName->DIR_Name[4];
            pDirName1->DIR_Name[5] = pDirName->DIR_Name[5];
            pDirName1->DIR_Name[6] = pDirName->DIR_Name[6];
            pDirName1->DIR_Name[7] = pDirName->DIR_Name[7];
            pDirName1->DIR_Name[8] = pDirName->DIR_Name[8];
            pDirName1->DIR_Name[9] = pDirName->DIR_Name[9];
            pDirName1->DIR_Name[10] = pDirName->DIR_Name[10];
            sum = ChkSum(pDirName1); /*�����ۼӺ�*/
            pBuf = LongFileName;     /*ָ���ļ����ռ�*/
            y = 1;
            if(k > 16)
            {
                for(i = 1; i != k - 16 + 1; i++)
                {
                    pLDirName->LDIR_Ord = y;
                    pLDirName->LDIR_Name1[0] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name1[1] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name1[2] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name1[3] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name1[4] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Attr = 0x0f;
                    pLDirName->LDIR_Type = 0;
                    pLDirName->LDIR_Chksum = sum;
                    pLDirName->LDIR_Name2[0] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name2[1] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name2[2] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name2[3] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name2[4] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name2[5] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_FstClusLO[0] = 0;
                    pLDirName->LDIR_FstClusLO[1] = 0;
                    pLDirName->LDIR_Name3[0] = *pBuf;
                    pBuf++;
                    pLDirName->LDIR_Name3[1] = *pBuf;
                    pBuf++;
                    pLDirName--;
                    y += 1;
                }
                k = 16;
                i = 0;
                pLDirName = (F_LONG_NAME *)DATA_BASE_BUF0;
                pLDirName += (k - 1);
            }
            if(k > 16)
                k = 16;
            for(i = 1; i != k - z; i++)
            {
                pLDirName->LDIR_Ord = y;
                pLDirName->LDIR_Name1[0] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name1[1] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name1[2] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name1[3] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name1[4] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Attr = 0x0f;
                pLDirName->LDIR_Type = 0;
                pLDirName->LDIR_Chksum = sum;
                pLDirName->LDIR_Name2[0] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name2[1] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name2[2] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name2[3] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name2[4] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name2[5] = *pBuf;
                pBuf++;
                pLDirName->LDIR_FstClusLO[0] = 0;
                pLDirName->LDIR_FstClusLO[1] = 0;
                pLDirName->LDIR_Name3[0] = *pBuf;
                pBuf++;
                pLDirName->LDIR_Name3[1] = *pBuf;
                pBuf++;
                pLDirName--;
                y += 1;
            }
            pLDirName->LDIR_Ord = y | 0x40;
            pLDirName->LDIR_Name1[0] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name1[1] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name1[2] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name1[3] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name1[4] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Attr = 0x0f;
            pLDirName->LDIR_Type = 0;
            pLDirName->LDIR_Chksum = sum;
            pLDirName->LDIR_Name2[0] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name2[1] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name2[2] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name2[3] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name2[4] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name2[5] = *pBuf;
            pBuf++;
            pLDirName->LDIR_FstClusLO[0] = 0;
            pLDirName->LDIR_FstClusLO[1] = 0;
            pLDirName->LDIR_Name3[0] = *pBuf;
            pBuf++;
            pLDirName->LDIR_Name3[1] = *pBuf;
            pBuf++;
            pBuf = (uint16_t *)pDirName1;
            pBuf += 16;

            if(pBuf < (uint16_t *)(DATA_BASE_BUF0 + 0x200))
            {
                i = 2;
                while(1)
                {
                    *pBuf = 0;
                    pBuf++;
                    if(pBuf == (uint16_t *)(DATA_BASE_BUF0 + 0x200))
                        break;
                }
                i++;
            }
            else if(pBuf < (uint16_t *)(DATA_BASE_BUF1 + 0x200))
            {
                i = 1;
                while(1)
                {
                    *pBuf = 0;
                    pBuf++;
                    if(pBuf == (uint16_t *)(DATA_BASE_BUF1 + 0x200))
                        break;
                }
                i++;
            }
            mCmdParam.Locate.mSectorOffset = tempSec - 1;
            CHRV3DirtyBuffer();
            i = CHRV3FileLocate();
            if(i != ERR_SUCCESS)
                return i;                    /*�������ܼ�������*/
            mCmdParam.Read.mSectorCount = 1; /*��������*/
            mCmdParam.Read.mDataBuffer = DATA_BASE_BUF0;
            CHRV3DirtyBuffer();
            i = CHRV3FileWrite(); /*��ȡ��һ�����������ݣ�ȡ��һ��Ŀ¼����ǸղŴ����Ķ��ļ���*/
            CHRV3DirtyBuffer();
            if(i != ERR_SUCCESS)
                return i;                      /*����Ҫ����������*/
            pBuf = (uint16_t *)DATA_BASE_BUF1; /**/
            if(*pBuf != 0)
            {
                mCmdParam.Read.mSectorCount = 1;
                mCmdParam.Read.mDataBuffer = DATA_BASE_BUF1;
                i = CHRV3FileWrite();
                CHRV3DirtyBuffer();
            }
            /*������ڸ�Ŀ¼�²���Ӧ�رո�Ŀ¼*/
            /*���滹Ҫ���ļ�*/
            i = CHRV3FileClose();
            for(k = 0; k != MAX_PATH_LEN; k++)
                mCmdParam.Other.mBuffer[k] = ParData[k]; /*�Դ����ļ����ļ���*/
            i = CHRV3FileOpen();                         /*�򿪴������ļ�*/
            return i;
        }
    }
XBB:
{
    return i = 0xfe;
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
    uint8_t s, i;

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
    while(1)
    {
        s = ERR_SUCCESS;
        if(R8_USB_INT_FG & RB_UIF_DETECT) // �����USB��������ж�����
        {
            R8_USB_INT_FG = RB_UIF_DETECT; // �������жϱ�־
            s = AnalyzeRootHub();          // ����ROOT-HUB״̬
            if(s == ERR_USB_CONNECT)
                FoundNewDev = 1;
        }

        if(FoundNewDev || s == ERR_USB_CONNECT) // ���µ�USB�豸����
        {
            FoundNewDev = 0;
            mDelaymS(200);        // ����USB�豸�ղ�����δ�ȶ�,�ʵȴ�USB�豸���ٺ���,������ζ���
            s = InitRootDevice(); // ��ʼ��USB�豸
            if(s == ERR_SUCCESS)
            {
                // U�̲������̣�USB���߸�λ��U�����ӡ���ȡ�豸������������USB��ַ����ѡ�Ļ�ȡ������������֮�󵽴�˴�����CHRV3�ӳ���������ɺ�������
                CHRV3DiskStatus = DISK_USB_ADDR;
                for(i = 0; i != 10; i++)
                {
                    PRINT("Wait DiskReady\n");
                    s = CHRV3DiskReady(); //�ȴ�U��׼����
                    if(s == ERR_SUCCESS)
                    {
                        break;
                    }
                    else
                    {
                        PRINT("%02x\n", (uint16_t)s);
                    }
                    mDelaymS(50);
                }

                if(CHRV3DiskStatus >= DISK_MOUNTED)
                {
                    //�������ļ����ļ���ʾ
                    PRINT("Create Long Name\n");
                    strcpy((uint8_t *)mCmdParam.Create.mPathName, "/TCBD~1.CSV"); /* ���ļ���,�ڸ�Ŀ¼��,�����ļ��� */

                    LongFileName[0] = 0X0054; /*����UNICODE�ĳ��ļ���*/
                    LongFileName[1] = 0X0043; //TCBD_data_day.csv
                    LongFileName[2] = 0X0042;
                    LongFileName[3] = 0X0044;
                    LongFileName[4] = 0X005F;
                    LongFileName[5] = 0X0064;
                    LongFileName[6] = 0X0061;
                    LongFileName[7] = 0X0074;
                    LongFileName[8] = 0X0061;
                    LongFileName[9] = 0X005F;
                    LongFileName[10] = 0X0064;
                    LongFileName[11] = 0X0061;
                    LongFileName[12] = 0X0079;
                    LongFileName[13] = 0X002e;
                    LongFileName[14] = 0X0063;
                    LongFileName[15] = 0X0073;
                    LongFileName[16] = 0X0076;
                    LongFileName[17] = 0X0000;

                    s = CreatLongName(); /*�������ļ���*/
                    if(s != ERR_SUCCESS)
                        PRINT("Error: %02x\n", s);
                    else
                        PRINT("Creat end\n");
                }
            }
        }
        mDelaymS(100);  // ģ�ⵥƬ����������
        SetUsbSpeed(1); // Ĭ��Ϊȫ��
    }
}
