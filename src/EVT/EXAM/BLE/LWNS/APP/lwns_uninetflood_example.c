/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_uninetflood_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/10/25
 * Description        : lwns�������緺�鴫�����ӣ����������緺�鷢����ָ���ڵ�
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_uninetflood_example.h"

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
static lwns_addr_t dst_addr = {{0xa3, 0xdf, 0x38, 0xe4, 0xc2, 0x84}}; //Ŀ��ڵ��ַ������ʱ������ݵ�·��оƬMAC��ַ��ͬ�����޸ġ��޸�Ϊ���շ���MAC��ַ������ʹ���Լ���MAC��ַ
#else
static lwns_addr_t dst_addr = {{0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84}};
#endif

static uint8_t  TX_DATA[LWNS_DATA_SIZE] = {0}; //��󳤶������շ�����
static uint8_t  RX_DATA[LWNS_DATA_SIZE] = {0}; //��󳤶������շ�����
static uint16_t lwns_uninetflood_ProcessEvent(uint8_t task_id, uint16_t events);
static void     uninetflood_recv(lwns_controller_ptr ptr, const lwns_addr_t *sender, uint8_t hops); //�������緺����ջص�����
static void     uninetflood_sent(lwns_controller_ptr ptr);                                          //�������緺�鷢����ɻص�����

static lwns_uninetflood_controller uninetflood; //�������緺����ƽṹ��

static uint8_t uninetflood_taskID; //�������緺���������id

/*********************************************************************
 * @fn      uninetflood_recv
 *
 * @brief   lwns uninetflood���ջص�����
 *
 * @param   ptr         -   ���ν��յ�������������uninetflood���ƽṹ��ָ��.
 * @param   sender      -   ���ν��յ������ݵķ����ߵ�ַָ��.
 * @param   hops        -   ���ν��յ������ݵĴӷ��ͷ������ڵ㾭��������.
 *
 * @return  None.
 */
static void uninetflood_recv(lwns_controller_ptr ptr, const lwns_addr_t *sender, uint8_t hops)
{
    uint8_t len;
    len = lwns_buffer_datalen();    //��ȡ��ǰ���������յ������ݳ���
    lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
    PRINTF("uninetflood %d rec from %02x %02x %02x %02x %02x %02x\n",
           get_lwns_object_port(ptr),
           sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
           sender->v8[4], sender->v8[5]); //fromΪ���յ������ݵķ��ͷ���ַ
    PRINTF("data:");
    for(uint8_t i = 0; i < len; i++)
    {
        PRINTF("%02x ", RX_DATA[i]); //��ӡ������
    }
    PRINTF("\n");
}

/*********************************************************************
 * @fn      uninetflood_sent
 *
 * @brief   lwns uninetflood������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵĿɿ��������ƽṹ��ָ��.
 *
 * @return  None.
 */
static void uninetflood_sent(lwns_controller_ptr ptr)
{
    PRINTF("uninetflood %d sent\n", get_lwns_object_port(ptr));
}

/**
 * lwns �������緺��ص������ṹ�壬ע��ص�����
 */
static const struct lwns_uninetflood_callbacks uninetflood_callbacks =
    {uninetflood_recv, uninetflood_sent};

/*********************************************************************
 * @fn      lwns_uninetflood_process_init
 *
 * @brief   lwns uninetflood���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_uninetflood_process_init(void)
{
    uninetflood_taskID = TMOS_ProcessEventRegister(lwns_uninetflood_ProcessEvent);
    for(uint8_t i = 0; i < LWNS_DATA_SIZE; i++)
    {
        TX_DATA[i] = i;
    }
    lwns_uninetflood_init(&uninetflood,
                          137,                   //��һ���˿ں�Ϊ137�ĵ������緺��ṹ��
                          HTIMER_SECOND_NUM * 2, //���ȴ�ת��ʱ��
                          1,                     //�ڵȴ��ڼ䣬���յ�����ͬ�������ݰ���ȡ�������ݰ��ķ���
                          3,                     //���ת���㼶
                          FALSE,                 //�ڵȴ�ת�������У��յ����µ���Ҫת�������ݰ��������ݰ������̷��ͳ�ȥ���Ƕ�����FALSEΪ���̷��ͣ�TRUEΪ������
                          50,                    //����ָ���������ֵ������һ����࣬�������ű��ڴ��ڱ�������ݰ����С��ֵ���ڴ�ֵ�������Ϊ������ϻָ����������ո����ݰ���
                          //ͬʱ����ֵҲ�������ж�Ϊ�����ݰ��Ĳ�ֵ��������ͬһ���ڵ�������ݰ�����Ų����Ա��ڴ��еĴ���࣬���ȴ�ֵ����
                          //���磬�ڴ��б����Ϊ10�������ݰ����Ϊ60����ֵΪ50�����ڵ��ڴ�ʱ���õ�50�����Խ����ᱻ��ΪΪ�µ����ݰ�����������
                          //ֻ�����Ϊ59����ֵΪ49��С�ڸ�ֵ���Żᱻ���ա�
                          TRUE,                    //�����Ƿ�ת��Ŀ��Ǳ��������ݰ�������������mesh�Ƿ�����relay���ܡ�
                          &uninetflood_callbacks); //����0�����ʧ�ܡ�����1�򿪳ɹ���
    tmos_start_task(uninetflood_taskID, UNINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_uninetflood_ProcessEvent
 *
 * @brief   lwns uninetflood Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_uninetflood_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & UNINETFLOOD_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; //��λ�������ݣ��Ա�۲�Ч��
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        lwns_uninetflood_send(&uninetflood, &dst_addr);  //�������緺�鷢�����ݸ�Ŀ���ַ
        tmos_start_task(uninetflood_taskID, UNINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); //�����Է���
        return events ^ UNINETFLOOD_EXAMPLE_TX_PERIOD_EVT;
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
