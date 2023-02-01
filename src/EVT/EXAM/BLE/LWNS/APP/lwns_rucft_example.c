/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_rucft_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/30
 * Description        : reliable unicast file transfer���ɿ������ļ���������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_rucft_example.h"

//ÿ���ļ�����debug��ӡ�Ŀ��أ���0���Խ�ֹ���ļ��ڲ���ӡ
#define DEBUG_PRINT_IN_THIS_FILE    1
#if DEBUG_PRINT_IN_THIS_FILE
  #define PRINTF(...)    PRINT(__VA_ARGS__)
#else
  #define PRINTF(...) \
    do                \
    {                 \
    } while(0)
#endif

#if 1
static lwns_addr_t dst_addr = {{0x66, 0xdf, 0x38, 0xe4, 0xc2, 0x84}}; //Ŀ��ڵ��ַ������ʱ������ݵ�·��оƬMAC��ַ��ͬ�����޸ġ��޸�Ϊ���շ���MAC��ַ������ʹ���Լ���MAC��ַ
#else
static lwns_addr_t dst_addr = {{0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84}};
#endif

static uint8_t rucft_taskID;

static lwns_rucft_controller rucft; //����rucft���ƽṹ��

#define FILESIZE    4000
static char  strsend[FILESIZE]; //���ͻ�����
static char *strp;
static void  write_file(lwns_controller_ptr ptr, const lwns_addr_t *sender,
                        int offset, int flag, char *data, int datalen);
static int   read_file(lwns_controller_ptr ptr, int offset, char *to);
static void  timedout_rucft(lwns_controller_ptr ptr);

/**
 * lwns �ɿ������ļ�����ص������ṹ�壬ע��ص�����
 */
const static struct lwns_rucft_callbacks rucft_callbacks = {write_file,
                                                            read_file, timedout_rucft};

uint16_t lwns_rucft_ProcessEvent(uint8_t task_id, uint16_t events);

/*********************************************************************
 * @fn      write_file
 *
 * @brief   lwns rucft���ջص���������Ϊд���ļ��ص�����
 *
 * @param   ptr         -   ���ν��յ�������������netflood���ƽṹ��ָ��.
 * @param   sender      -   ���ν��յ������ݵķ����ߵ�ַָ��.
 * @param   offset      -   ���ν��յ������ݵ�ƫ������Ҳ�Ǳ����ļ���������Ѿ��յ���������.
 * @param   flag        -   ���ν��յ����ݵı�־��LWNS_RUCFT_FLAG_NONE/LWNS_RUCFT_FLAG_NEWFILE/LWNS_RUCFT_FLAG_END.
 * @param   data        -   ���ν��յ������ݵ�ͷָ��.
 * @param   datalen     -   ���ν��յ������ݵĳ���.
 *
 * @return  None.
 */
static void write_file(lwns_controller_ptr ptr, const lwns_addr_t *sender,
                       int offset, int flag, char *data, int datalen)
{
    //senderΪ���ͷ��ĵ�ַ
    //�����Ҫ���ղ�ͬ���ļ�����Ҫ�ڴ˺��������ýӿ�
    if(datalen > 0)
    { //�����������data��ȡ���ݴ�ӡ
        PRINTF("r:%c\n", *data);
    }
    if(flag == LWNS_RUCFT_FLAG_END)
    {
        PRINTF("re\n");
        //�����ļ���������һ����
    }
    else if(flag == LWNS_RUCFT_FLAG_NONE)
    {
        PRINTF("ru\n");
        //�����ļ����������İ�
    }
    else if(flag == LWNS_RUCFT_FLAG_NEWFILE)
    {
        PRINTF("rn\n");
        //�����ļ�����ĵ�һ����
    }
}

/*********************************************************************
 * @fn      read_file
 *
 * @brief   lwns rucft������ɻص���������Ϊ��ȡ�ļ��ص�����
 *
 * @param   ptr         -   ���ν��յ�������������netflood���ƽṹ��ָ��.
 * @param   offset      -   ���ν��յ������ݵ�ƫ������Ҳ�Ǳ����ļ����䷢���Ѿ����͵�������.
 * @param   to          -   ������Ҫ���͵����ݻ�������ͷָ�룬�û������ݿ�������ָ��ָ����ڴ�ռ䡣.
 *
 * @return  size        -   ���ص�size��Ϊ������Ҫ���͵����ݳ��ȣ����ɴ���LWNS_RUCFT_DATASIZE.
 */
static int read_file(lwns_controller_ptr ptr, int offset, char *to)
{
    //toΪ��Ҫ�������ݹ�ȥ��ָ��
    //�����Ҫ���Ͳ�ͬ���ļ�����Ҫ�ڴ˺��������ýӿ�
    int size = LWNS_RUCFT_DATASIZE;
    if(offset >= FILESIZE)
    {
        //�ϴ��Ѿ�����,���������ȷ��
        PRINTF("Send done\n");
        tmos_start_task(rucft_taskID, RUCFT_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(5000)); //5���Ӻ�������Ͳ���
        return 0;
    }
    else if(offset + LWNS_RUCFT_DATASIZE >= FILESIZE)
    {
        size = FILESIZE - offset;
    }
    //�ѱ�����Ҫ���͵�����ѹ��������
    tmos_memcpy(to, strp + offset, size);
    return size;
}

/*********************************************************************
 * @fn      timedout_rucft
 *
 * @brief   lwns rucft���ͳ�ʱ�ص�����
 *
 * @param   ptr     -   ���η�����ɵ�ruc���ƽṹ��ָ��.
 *
 * @return  None.
 */
static void timedout_rucft(lwns_controller_ptr ptr)
{
    //rucft�У����ͷ����ط�������������ط������󣬻���øûص���
    //���շ���ʱû���յ���һ����Ҳ�����
    PRINTF("rucft %d timedout\r\n", get_lwns_object_port(ptr));
}

/*********************************************************************
 * @fn      lwns_rucft_process_init
 *
 * @brief   lwns rucft���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_rucft_process_init(void)
{
    lwns_addr_t MacAddr;
    rucft_taskID = TMOS_ProcessEventRegister(lwns_rucft_ProcessEvent);
    lwns_rucft_init(&rucft, 137,            //�˿ں�
                    HTIMER_SECOND_NUM / 10, //�ȴ�Ŀ��ڵ�ackʱ��
                    5,                      //����ط���������ruc�е�ruc_send���ط���������һ��
                    &rucft_callbacks        //�ص�����
    );                                      //����0�����ʧ�ܡ�����1�򿪳ɹ���
    int i;
    for(i = 0; i < FILESIZE; i++)
    { //LWNS_RUCFT_DATASIZE��LWNSNK_RUCFT_DATASIZE��b���ȵȣ���ʼ����Ҫ���͵�����
        strsend[i] = 'a' + i / LWNS_RUCFT_DATASIZE;
    }
    strp = strsend;
    GetMACAddress(MacAddr.v8);
    if(lwns_addr_cmp(&MacAddr, &dst_addr))
    {
    }
    else
    {
        tmos_start_task(rucft_taskID, RUCFT_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000));
    }
}

/*********************************************************************
 * @fn      lwns_rucft_ProcessEvent
 *
 * @brief   lwns rucft Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_rucft_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & RUCFT_EXAMPLE_TX_PERIOD_EVT)
    {
        PRINTF("send\n");
        lwns_rucft_send(&rucft, &dst_addr); //��ʼ������Ŀ��ڵ㣬�û����÷���ʱҪ���úûص������е����ݰ���ȡ
        return events ^ RUCFT_EXAMPLE_TX_PERIOD_EVT;
    }
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
    return 0;
}
