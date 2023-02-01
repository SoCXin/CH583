/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_netflood_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/07/12
 * Description        : netflood�����緺�鴫������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_netflood_example.h"

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

static uint8_t  TX_DATA[LWNS_DATA_SIZE] = {0}; //��󳤶������շ�����
static uint8_t  RX_DATA[LWNS_DATA_SIZE] = {0}; //��󳤶������շ�����
static uint16_t lwns_netflood_ProcessEvent(uint8_t task_id, uint16_t events);
static int      netflood_recv(lwns_controller_ptr ptr,
                              const lwns_addr_t  *from,
                              const lwns_addr_t *originator, uint8_t hops);
static void     netflood_sent(lwns_controller_ptr ptr);
static void     netflood_dropped(lwns_controller_ptr ptr);

/**
 * lwns ���緺��ص������ṹ�壬ע��ص�����
 */
static const struct lwns_netflood_callbacks callbacks = {netflood_recv,
                                                         netflood_sent, netflood_dropped};

static uint8_t netflood_taskID;

void lwns_netflood_process_init(void);

static lwns_netflood_controller netflood; //���緺����ƽṹ��

/*********************************************************************
 * @fn      netflood_recv
 *
 * @brief   lwns netflood���ջص�����
 *
 * @param   ptr         -   ���ν��յ�������������netflood���ƽṹ��ָ��.
 * @param   from        -   ���ν��յ������ݵ���һ��ת���ߵĵ�ַָ��.
 * @param   originator  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 * @param   hops        -   ���ν��յ������ݵĴӷ��ͷ������ڵ㾭��������.
 *
 * @return  0/1         -   0��ʾ���ڵ㲻��ת�������ݰ���1��ʾ���ڵ����ת�������ݰ�.
 */
static int netflood_recv(lwns_controller_ptr ptr,
                         const lwns_addr_t  *from,
                         const lwns_addr_t *originator, uint8_t hops)
{
    uint8_t len;
    len = lwns_buffer_datalen(); //��ȡ��ǰ���������յ������ݳ���
    PRINTF("netflood %d rec %02x %02x %02x %02x %02x %02x,hops=%d\r\n", get_lwns_object_port(ptr),
           from->v8[0], from->v8[1], from->v8[2], from->v8[3], from->v8[4],
           from->v8[5], hops); //��ӡת���ߣ���Ϊ�յ��ı���ת��������˭ת���ġ�
    PRINTF("netflood orec %02x %02x %02x %02x %02x %02x,hops=%d\r\n",
           originator->v8[0], originator->v8[1], originator->v8[2],
           originator->v8[3], originator->v8[4], originator->v8[5],
           hops);                   //��ӡ����Ϣ�����ߣ���Ϊ���𱾴����緺��Ľڵ��ַ��
    lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
    PRINTF("data:");
    for(uint8_t i = 0; i < len; i++)
    {
        PRINTF("%02x ", RX_DATA[i]);
    }
    PRINTF("\n");
    return 1; //����1���򱾽ڵ㽫��������netflood��ת������
    //return 0;//����0���򱾽ڵ㲻�ټ���netflood��ֱ����ֹ
}

/*********************************************************************
 * @fn      netflood_sent
 *
 * @brief   lwns netflood������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵ����緺����ƽṹ��ָ��.
 *
 * @return  None.
 */
static void netflood_sent(lwns_controller_ptr ptr)
{
    PRINTF("netflood %d sent\n", get_lwns_object_port(ptr));
}

/*********************************************************************
 * @fn      netflood_dropped
 *
 * @brief   lwns netflood���ݰ������ص�����
 *
 * @param   ptr     -   ���η�����ɵ����緺����ƽṹ��ָ��.
 *
 * @return  None.
 */
static void netflood_dropped(lwns_controller_ptr ptr)
{
    PRINTF("netflood %d dropped\n", get_lwns_object_port(ptr));
}

/*********************************************************************
 * @fn      lwns_netflood_process_init
 *
 * @brief   lwns netflood���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_netflood_process_init(void)
{
    netflood_taskID = TMOS_ProcessEventRegister(lwns_netflood_ProcessEvent);
    for(uint8_t i = 0; i < LWNS_DATA_SIZE; i++)
    {
        TX_DATA[i] = i;
    }
    lwns_netflood_init(&netflood,
                       137,                   //��һ���˿ں�Ϊ137�ķ���ṹ��
                       HTIMER_SECOND_NUM * 1, //�ȴ�ת��ʱ��
                       1,                     //�ڵȴ��ڼ䣬���յ�����ͬ�������ݰ���ȡ�������ݰ��ķ���
                       3,                     //���ת���㼶
                       FALSE,                 //�ڵȴ�ת�������У��յ����µ���Ҫת�������ݰ��������ݰ������̷��ͳ�ȥ���Ƕ�����FALSEΪ���̷��ͣ�TRUEΪ������
                       50,                    //����ָ���������ֵ������һ����࣬�������ű��ڴ��ڱ�������ݰ����С��ֵ���ڴ�ֵ�������Ϊ������ϻָ����������ո����ݰ���
                       //ͬʱ����ֵҲ�������ж�Ϊ�����ݰ��Ĳ�ֵ��������ͬһ���ڵ�������ݰ�����Ų����Ա��ڴ��еĴ���࣬���ȴ�ֵ����
                       //���磬�ڴ��б����Ϊ10�������ݰ����Ϊ60����ֵΪ50�����ڵ��ڴ�ʱ���õ�50�����Խ����ᱻ��ΪΪ�µ����ݰ�����������
                       //ֻ�����Ϊ59����ֵΪ49��С�ڸ�ֵ���Żᱻ���ա�
                       &callbacks); //����0�����ʧ�ܡ�����1�򿪳ɹ���
#if 1
    tmos_start_task(netflood_taskID, NETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
#endif
}

/*********************************************************************
 * @fn      lwns_netflood_ProcessEvent
 *
 * @brief   lwns netflood Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
static uint16_t lwns_netflood_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & NETFLOOD_EXAMPLE_TX_PERIOD_EVT)
    {
        PRINTF("send\n");
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        lwns_netflood_send(&netflood);                   //�������緺�����ݰ�
        tmos_start_task(netflood_taskID, NETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(3000)); //10s����һ��
        return (events ^ NETFLOOD_EXAMPLE_TX_PERIOD_EVT);
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
