/********************************** (C) COPYRIGHT *******************************
 * File Name          : iap.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2022/03/15
 * Description        : UART IAP����
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "iap.h"

/* ����Ȩ�ޣ������Ȳ������Ҳ�����ַ��Ҫ��APP��Ӧ��ַһ�²ſɸ������Ȩ�� */
uint8_t g_update_permition = 0;

uint8_t iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1, all_data_rec_cnt = 0, part_rec_cnt = 0, uart_rec_sign = 0;

iap_cmd_t iap_rec_data;

__attribute__((aligned(4)))                    uint8_t g_write_buf[256 + 64];

uint16_t g_buf_write_ptr = 0;

uint32_t g_flash_write_ptr = 0;

uint32_t g_tcnt;

uint32_t g_addr;

__attribute__((aligned(4)))   uint8_t iap_rsp_data[6] = {IAP_DATA_SOP1, IAP_DATA_SOP2, 0, 0, IAP_DATA_EOP1, IAP_DATA_EOP2};

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   IAP��ѭ��,�����ram�����У������ٶ�.
 *
 * @param   None.
 *
 * @return  None.
 */
__attribute__((section(".highcode")))
void Main_Circulation()
{
    while (1)
    {
        /* ���ò�ѯģʽ����ʹ���жϣ����ٴ���ռ��flash */
        if (R8_UART1_RFC)
        {
            /* ���������ݣ���λ���ڴ��ڽ��չ��̱�־λ */
            uart_rec_sign = 1;
            /* ���������ݣ���ճ�ʱʱ�� */
            g_tcnt = 0;
            /* ���ռ�����һ���洢��buf�� */
            all_data_rec_cnt++;
            /* buf����Խ�� */
            if (all_data_rec_cnt >= sizeof(iap_rec_data))
            {
                all_data_rec_cnt = 0;
            }
            /* ��ȡ���ڼĴ������ݣ�ֱ�Ӹ�ֵ��buf�У������ж����������ֵ�����Խ�Լ����flashռ�� */
            iap_rec_data.other.buf[all_data_rec_cnt] = R8_UART1_RBR;
            /*
             * ����Ϊ�˽�Լ����ռ�ÿռ�ʹ��������ٶȣ�������Ӧ�Ĵ���ü���
             * ����Ӧ�ø��������룬��У��ֵ�Ͱ�β��Ϣ�浽��Ӧ�Ľṹ���checksum��eop��Ա�����С�
             * ������Ĵ�����Ϊֱ�Ӱ�˳��洢��buf�У�
             * ���������ݳ��Ȳ�Ϊ�������ݳ���ʱ��ͨ���ṹ����ʳ�Աchecksum��eop�����᲻��ȷ��
             * �û������޸�ʱ��ע�⡣
             * �����ÿ�ζ����ݰ����ж���˵��ÿ�ζ���Լ�˶�������ж�ʱ�䡢����flash�ռ䡣
             */
            switch (iap_rec_data_state) /* ���ݽ���״̬�������ݵĶ�ȡ�жϺʹ洢 */
            {
            /* ״̬���ڵȴ���ͷ1ʱ���жϽ��յ��ֽ��Ƿ�ΪIAP_DATA_SOP1 */
            case IAP_DATA_REC_STATE_WAIT_SOP1:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_SOP1)
                {
                    /* ֻ�е�һ���ֽ�ʱ����һ���洢��buf��ȷ��λ�ã����´洢 */
                    iap_rec_data.other.buf[0] = iap_rec_data.other.buf[all_data_rec_cnt];
                    /* ���ռ�����ʼ�� */
                    all_data_rec_cnt = 0;
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP2;
                }
                break;
            /* ״̬���ڵȴ���ͷ2ʱ���жϽ��յ��ֽ��Ƿ�ΪIAP_DATA_SOP2 */
            case IAP_DATA_REC_STATE_WAIT_SOP2:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_SOP2)
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CMD;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            /* ״̬���ڵȴ�������ʱ���жϽ��յ��ֽ��Ƿ�Ϊ�Ϸ���cmd */
            case IAP_DATA_REC_STATE_WAIT_CMD:
                if ((iap_rec_data.other.buf[all_data_rec_cnt] < CMD_IAP_PROM) || (iap_rec_data.other.buf[all_data_rec_cnt] > CMD_IAP_END))
                {
                    /* error û�����cmd */
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_LEN;
                }
                break;
            /* ״̬���ڵȴ����ݳ���ʱ���жϽ��յ��ֽ��Ƿ�Ϊ�Ϸ��ĳ��� */
            case IAP_DATA_REC_STATE_WAIT_LEN:
                if (iap_rec_data.other.buf[all_data_rec_cnt] <= IAP_LEN)
                {
                    /* ��ղ��ֽṹ����������ֽ������� */
                    part_rec_cnt = 0;
                    if ((iap_rec_data.other.buf[2] == CMD_IAP_ERASE) || (iap_rec_data.other.buf[2] == CMD_IAP_VERIFY))
                    {
                        iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_ADDR;
                    }
                    else
                    {
                        /* �ж����ݳ����Ƿ�Ϊ0��Ϊ0��ֱ�ӽ���У���*/
                        if (iap_rec_data.other.buf[3] > 0)
                        {
                            iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_DATA;
                        }
                        else
                        {
                            iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CHECKNUM;
                        }
                    }
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            /* ״̬���ڵȴ���ַʱ */
            case IAP_DATA_REC_STATE_WAIT_ADDR:
                part_rec_cnt++;
                /* ��ַΪ4�ֽڣ����յ�4������ת������һ��״̬ */
                if (part_rec_cnt >= 4)
                {
                    /* ���ֽṹ����������ֽ������� */
                    part_rec_cnt = 0;
                    if (iap_rec_data.other.buf[3] > 0)
                    {
                        iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_DATA;
                    }
                    else
                    {
                        iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CHECKNUM;
                    }
                }
                break;
            /* ״̬���ڵȴ�����ʱ */
            case IAP_DATA_REC_STATE_WAIT_DATA:
                part_rec_cnt++;
                if (part_rec_cnt >= iap_rec_data.other.buf[3])
                {
                    /* �ж������Ƿ�������*/
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_CHECKNUM;
                    /* ���ֽṹ����������ֽ������� */
                    part_rec_cnt = 0;
                }
                break;
            /* ״̬���ڵȴ�У��ʱ */
            case IAP_DATA_REC_STATE_WAIT_CHECKNUM:
                part_rec_cnt++;
                if (part_rec_cnt >= 2)
                {
                    /* �ж�У���Ƿ������ɣ�У��Ϊ2�ֽں�У��*/
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_EOP1;
                }
                break;
            /* ״̬���ڵȴ���β1ʱ���жϽ��յ��ֽ��Ƿ�ΪIAP_DATA_EOP1 */
            case IAP_DATA_REC_STATE_WAIT_EOP1:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_EOP1)
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_EOP2;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            /* ״̬���ڵȴ���β2ʱ���жϽ��յ��ֽ��Ƿ�ΪIAP_DATA_EOP2 */
            case IAP_DATA_REC_STATE_WAIT_EOP2:
                if (iap_rec_data.other.buf[all_data_rec_cnt] == IAP_DATA_EOP2)
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_OK;
                }
                else
                {
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                }
                break;
            default:
                /* һ�㲻���ܳ���������� */
                break;
            }

            if (iap_rec_data_state == IAP_DATA_REC_STATE_OK)
            {
                /* ����У��� */
                uint16_t   check_num = 0, check_num_rec;
                /* У��ͼ������� */
                uint16_t   check_num_i;
                /* �ϱ���������ΪĬ���޴���״̬ */
                iap_rsp_data[2] = 0x00;
                iap_rsp_data[3] = 0x00;
                /* �ָ�Ĭ�ϵ�״̬ */
                iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                /* ������һ�����������ݰ����ͷſ��м�ʱ */
                uart_rec_sign = 0;
                g_tcnt = 0;
                /* ����У������� */
                for (check_num_i = 2; check_num_i < all_data_rec_cnt - 3; check_num_i++)
                {
                    check_num += iap_rec_data.other.buf[check_num_i];
                }
                check_num_rec = iap_rec_data.other.buf[check_num_i] | (iap_rec_data.other.buf[check_num_i + 1] << 8);
                /* ���ݰ�У���ͨ�� */
                if (check_num_rec == check_num)
                {
                    /* �ж����� */
                    switch (iap_rec_data.other.buf[2])
                    {
                    /* д������ */
                    case CMD_IAP_PROM:
                        /* �ж��Ƿ��Ȳ�����оƬ */
                        if (g_update_permition == 1)
                        {
                            if (iap_rec_data.program.len == 0)
                            {
                                /* ���һ��Ϊ�հ�������Ϊ���һ���� */
                                if (g_buf_write_ptr != 0)
                                {
                                    if (FLASH_ROM_WRITE(g_flash_write_ptr, (PUINT32)g_write_buf, g_buf_write_ptr))
                                    {
                                        iap_rsp_data[2] = 0xfe;
                                        iap_rsp_data[3] = IAP_ERR_WRITE_FAIL;
                                    }
                                    g_buf_write_ptr = 0;
                                }
                            }
                            else
                            {
                                my_memcpy(g_write_buf + g_buf_write_ptr, iap_rec_data.program.data, iap_rec_data.program.len);
                                g_buf_write_ptr += iap_rec_data.program.len;
                                if (g_buf_write_ptr >= 256)
                                {
                                    /* ��256�ֽ�дһ�� */
                                    if (FLASH_ROM_WRITE(g_flash_write_ptr, (PUINT32)g_write_buf, 256))
                                    {
                                        iap_rsp_data[2] = 0xfe;
                                        iap_rsp_data[3] = IAP_ERR_WRITE_FAIL;
                                        break;
                                    }
                                    /* �ƶ�ָ�� */
                                    g_flash_write_ptr += 256;
                                    /* ���¼��㳬���ĳ��ȣ������������ݿ����������ײ� */
                                    g_buf_write_ptr = g_buf_write_ptr - 256;
                                    my_memcpy(g_write_buf, g_write_buf + 256, g_buf_write_ptr);
                                }
                            }
                        }
                        else
                        {
                            /* û�в������裬���ɸ��£����� */
                            iap_rsp_data[2] = 0xfe;
                            iap_rsp_data[3] = IAP_ERR_PROG_NO_ERASE;
                        }
                        break;
                    /* �������� */
                    case CMD_IAP_ERASE:
                        if (iap_rec_data.erase.addr == APP_CODE_START_ADDR)
                        {
                            /* ������������󣬿����޸Ĳ����ĳ��ȣ��������������һ����flash�ų����� */
                            if (FLASH_ROM_ERASE(APP_CODE_START_ADDR, APP_CODE_END_ADDR - APP_CODE_START_ADDR) == 0)
                            {
                                /* ��ʼ��ַ��ȷ���������Ȩ�ޣ�������Ϊʧ�� */
                                g_update_permition = 1;
                                /* �������㣬flashдָ������ */
                                g_buf_write_ptr = 0;
                                g_flash_write_ptr = APP_CODE_START_ADDR;
                            }
                            else
                            {
                                /* ����ʧ�� */
                                iap_rsp_data[2] = 0xfe;
                                iap_rsp_data[3] = IAP_ERR_ERASE_FAIL;
                            }
                        }
                        else
                        {
                            /* ������ַ���� */
                            iap_rsp_data[2] = 0xfe;
                            iap_rsp_data[3] = IAP_ERR_ADDR;
                        }
                        break;
                    /* У������ */
                    case CMD_IAP_VERIFY:
                        if (((iap_rec_data.verify.addr % 4) == 0) && (iap_rec_data.verify.addr >= APP_CODE_START_ADDR) && (iap_rec_data.verify.addr < APP_CODE_END_ADDR))
                        {
                            my_memcpy(g_write_buf, iap_rec_data.verify.data, iap_rec_data.verify.len);
                            if (FLASH_ROM_VERIFY(iap_rec_data.verify.addr, g_write_buf, iap_rec_data.verify.len))
                            {
                                /* У��ʧ�ܣ����� */
                                iap_rsp_data[2] = 0xfe;
                                iap_rsp_data[3] = IAP_ERR_VERIFY;
                            }
                        }
                        else
                        {
                            /* У���ַ���� */
                            iap_rsp_data[2] = 0xfe;
                            iap_rsp_data[3] = IAP_ERR_ADDR;
                        }
                        break;
                    /* ������ת���� */
                    case CMD_IAP_END:
                        jumpApp();
                        break;
                    default:
                        /* ����ʱ���ж��������Ϊ����ֵ�����Բ�����ָ���� */
                        iap_rsp_data[2] = 0xfe;
                        iap_rsp_data[3] = IAP_ERR_UNKNOWN;
                        break;
                    }
                    if (iap_rsp_data[2] != 0)
                    {
                        /* У���ͨ����������������������ո���Ȩ�ޣ���Ҫ���£����������¿�ʼ */
                        g_update_permition = 0;
                    }
                }
                else
                {
                    /* ���ݰ�У���ʧ�ܣ���������ѡ���ط����ݰ�����Ӱ�� */
                    iap_rsp_data[2] = 0xfe;
                    iap_rsp_data[3] = IAP_ERR_CHECK;
                }
                /* ÿ�����ݰ������꣬��ջ������������ݣ���ֹ��Щ����ģ��żȻ��෢��һ�����ַ��źţ�����һ��һ�ظ� */
                while (R8_UART1_RFC)
                {
                    iap_rec_data.other.buf[all_data_rec_cnt] = R8_UART1_RBR;
                }
                /* �ظ����� */
                UART1_SendString(iap_rsp_data, sizeof(iap_rsp_data));
            }
        }
        else
        {
            /* �ӳ�115200�������´���ķ�֮һ���ֽڵ�ʱ�䣬���Ͷ�ȡ�Ĵ���Ƶ�ʺ�ʱ�䣬���㳬ʱ����������޸Ĳ����ʣ�Ҳ�������������ص�ʱ����� */
            DelayUs(20);
            g_tcnt++;
            if (uart_rec_sign)
            {
                if (g_tcnt >= 43)
                {
                    /* ����10���ֽڿ���ʱ��û�����ֽڵ�������û��һ�����������ݰ����ͳ�ʱ�������ݲ������޸� */
                    /* Ŀǰ������Ϊ115200,һ���ֽ�ʱ��Ϊ1s/11520 = 87us, 87us*10 / 20us = 43.5 */
                    /* ��������ѡ���ط����ݰ�����Ӱ�� */
                    iap_rec_data_state = IAP_DATA_REC_STATE_WAIT_SOP1;
                    uart_rec_sign = 0;
                    iap_rsp_data[2] = 0xfe;
                    iap_rsp_data[3] = IAP_ERR_OVERTIME;
                    UART1_SendString(iap_rsp_data, sizeof(iap_rsp_data));
                }
            }
            else
            {
                if (g_tcnt > 6000000)
                {
                    /* 120��û�����ݣ���Ϊ��ʱ������app��������������޸� */
                    jumpApp();
                }
            }
        }
    }
}


/*********************************************************************
 * @fn      my_memcpy
 *
 * @brief   ���ݿ�������,�����ram�����У������ٶ�
 *
 * @param   None.
 *
 * @return  None.
 */
__attribute__((section(".highcode")))
void my_memcpy(void *dst, const void *src, uint32_t l)
{
    uint32_t len = l;
    PUINT8 pdst = (PUINT8) dst;
    PUINT8 psrc = (PUINT8) src;
    while (len)
    {
        *pdst++ = *psrc++;
        len--;
    }
}
