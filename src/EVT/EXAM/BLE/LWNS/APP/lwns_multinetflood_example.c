/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_multinetflood_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/11/10
 * Description        : multinetflood���鲥���緺�鴫������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_multinetflood_example.h"

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

/*
 * ���ĵĵ�ַΪ2�ֽ�u16���͡�
 */
static uint8_t subaddrs_index = 0;                 //���Ͷ��ĵ�ַ���
#define SUBADDR_NUM    3                           //���ĵ�ַ����
static uint16_t subaddrs[SUBADDR_NUM] = {1, 2, 3}; //���ĵ�ַ����

static uint8_t  TX_DATA[LWNS_DATA_SIZE] = {0}; //��󳤶������շ�����
static uint8_t  RX_DATA[LWNS_DATA_SIZE] = {0}; //��󳤶������շ�����
static uint16_t lwns_multinetflood_ProcessEvent(uint8_t task_id, uint16_t events);
static void     multinetflood_recv(lwns_controller_ptr ptr, uint16_t subaddr, const lwns_addr_t *sender, uint8_t hops); //�鲥���緺����ջص�����
static void     multinetflood_sent(lwns_controller_ptr ptr);                                                            //�鲥���緺�鷢����ɻص�����

static lwns_multinetflood_controller multinetflood; //�����鲥���緺����ƽṹ��

static uint8_t multinetflood_taskID; //�鲥���緺���������id

/*********************************************************************
 * @fn      multinetflood_recv
 *
 * @brief   lwns multinetflood���ջص�����
 *
 * @param   ptr     -   ���ν��յ�������������multinetflood���ƽṹ��ָ��.
 * @param   subaddr -   ���ν��յ������ݵĶ��ĵ�ַ.
 * @param   sender  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 * @param   hops    -   ���ν��յ������ݵĴӷ��ͷ������ڵ㾭��������.
 *
 * @return  None.
 */
static void multinetflood_recv(lwns_controller_ptr ptr, uint16_t subaddr, const lwns_addr_t *sender, uint8_t hops)
{
    uint8_t len;
    len = lwns_buffer_datalen();    //��ȡ��ǰ���������յ������ݳ���
    lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
    PRINTF("multinetflood %d rec from %02x %02x %02x %02x %02x %02x\n",
           get_lwns_object_port(ptr),
           sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
           sender->v8[4], sender->v8[5]); //fromΪ���յ������ݵķ��ͷ���ַ
    PRINTF("subaddr:%d,data:", subaddr);
    for(uint8_t i = 0; i < len; i++)
    {
        PRINTF("%02x ", RX_DATA[i]); //��ӡ������
    }
    PRINTF("\n");
}

/*********************************************************************
 * @fn      multinetflood_sent
 *
 * @brief   lwns multinetflood������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵ��鲥���緺����ƽṹ��ָ��.
 *
 * @return  None.
 */
static void multinetflood_sent(lwns_controller_ptr ptr)
{
    PRINTF("multinetflood %d sent\n", get_lwns_object_port(ptr));
}

/**
 * lwns �鲥���緺��ص������ṹ�壬ע��ص�����
 */
static const struct lwns_multinetflood_callbacks multinetflood_callbacks =
    {multinetflood_recv, multinetflood_sent};

/*********************************************************************
 * @fn      lwns_multinetflood_process_init
 *
 * @brief   lwns multinetflood���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_multinetflood_process_init(void)
{
    multinetflood_taskID = TMOS_ProcessEventRegister(lwns_multinetflood_ProcessEvent);
    for(uint8_t i = 0; i < LWNS_DATA_SIZE; i++)
    {
        TX_DATA[i] = i;
    }
    lwns_multinetflood_init(&multinetflood,
                            137,               //��һ���˿ں�Ϊ137���鲥���緺��ṹ��
                            HTIMER_SECOND_NUM, //���ȴ�ת��ʱ��
                            1,                 //�ڵȴ��ڼ䣬���յ�����ͬ�������ݰ���ȡ�������ݰ��ķ���
                            3,                 //���ת���㼶
                            FALSE,             //�ڵȴ�ת�������У��յ����µ���Ҫת�������ݰ��������ݰ������̷��ͳ�ȥ���Ƕ�����FALSEΪ���̷��ͣ�TRUEΪ������
                            50,                //����ָ���������ֵ������һ����࣬�������ű��ڴ��ڱ�������ݰ����С��ֵ���ڴ�ֵ�������Ϊ������ϻָ����������ո����ݰ���
                            //ͬʱ����ֵҲ�������ж�Ϊ�����ݰ��Ĳ�ֵ��������ͬһ���ڵ�������ݰ�����Ų����Ա��ڴ��еĴ���࣬���ȴ�ֵ����
                            //���磬�ڴ��б����Ϊ10�������ݰ����Ϊ60����ֵΪ50�����ڵ��ڴ�ʱ���õ�50�����Խ����ᱻ��ΪΪ�µ����ݰ�����������
                            //ֻ�����Ϊ59����ֵΪ49��С�ڸ�ֵ���Żᱻ���ա�
                            TRUE,                      //�����Ƿ�ת��Ŀ��Ǳ��������ݰ�������������mesh�Ƿ�����relay���ܡ�
                            subaddrs,                  //���ĵĵ�ַ����ָ��
                            SUBADDR_NUM,               //���ĵ�ַ����
                            &multinetflood_callbacks); //����0�����ʧ�ܡ�����1�򿪳ɹ���
#if 1
    tmos_start_task(multinetflood_taskID, MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
#endif
}

/*********************************************************************
 * @fn      lwns_multinetflood_ProcessEvent
 *
 * @brief   lwns multinetflood Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_multinetflood_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; //��λ�������ݣ��Ա�۲�Ч��
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        if(subaddrs_index >= SUBADDR_NUM)
        {
            subaddrs_index = 0;
        }
        lwns_multinetflood_send(&multinetflood, subaddrs[subaddrs_index]); //�鲥���緺�鷢�����ݵ����ĵ�ַ
        subaddrs_index++;

        tmos_start_task(multinetflood_taskID, MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); //�����Է���
        return events ^ MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT;
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
