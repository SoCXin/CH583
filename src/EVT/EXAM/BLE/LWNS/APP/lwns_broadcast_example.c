/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_broadcast_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/20
 * Description        : broadcast���㲥��������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_broadcast_example.h"

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

//�û���Ҫ���͵����ݻ�����
static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

static uint8_t  RX_DATA[10]; //�û��������ݵĻ�����
static uint16_t lwns_broadcast_ProcessEvent(uint8_t task_id, uint16_t events);
static void     broadcast_recv(lwns_controller_ptr ptr,
                               const lwns_addr_t  *sender); //���ջص���������
static void     broadcast_sent(lwns_controller_ptr ptr);   //���ͻص���������

static lwns_broadcast_controller broadcast; //�㲥���ƽṹ��

static uint8_t broadcast_taskID;

/*********************************************************************
 * @fn      broadcast_recv
 *
 * @brief   lwns broadcast���ջص�����
 *
 * @param   ptr     -   ���ν��յ������������Ĺ㲥���ƽṹ��ָ��.
 * @param   sender  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 *
 * @return  None.
 */
static void broadcast_recv(lwns_controller_ptr ptr,
                           const lwns_addr_t  *sender)
{
    uint8_t len;
    len = lwns_buffer_datalen(); //��ȡ��ǰ���������յ������ݳ���
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
        PRINTF("broadcast %d rec from %02x %02x %02x %02x %02x %02x\n", get_lwns_object_port(ptr),
               sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
               sender->v8[4], sender->v8[5]); //��ӡ��������Ϣ�ķ����ߵ�ַ
        PRINTF("data:");
        for(uint8_t i = 0; i < len; i++)
        {
            PRINTF("%02x ", RX_DATA[i]); //��ӡ����
        }
        PRINTF("\n");
    }
    else
    {
        PRINTF("data len err\n");
    }
}

/*********************************************************************
 * @fn      broadcast_sent
 *
 * @brief   lwns broadcast������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵĹ㲥���ƽṹ��ָ��.
 *
 * @return  None.
 */
static void broadcast_sent(lwns_controller_ptr ptr)
{
    PRINTF("broadcast %d sent\n", get_lwns_object_port(ptr)); //��ӡ���������Ϣ
}

/**
 * lwns�㲥�ص������ṹ�壬ע��ص�����
 */
static const struct lwns_broadcast_callbacks broadcast_call = {
    broadcast_recv, broadcast_sent};

/*********************************************************************
 * @fn      lwns_broadcast_process_init
 *
 * @brief   lwns broadcast���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_broadcast_process_init(void)
{
    broadcast_taskID = TMOS_ProcessEventRegister(lwns_broadcast_ProcessEvent);
    lwns_broadcast_init(&broadcast, 136, &broadcast_call); //��һ���˿ں�Ϊ136�Ĺ㲥�˿ڣ�ע��ص�����
    tmos_start_task(broadcast_taskID, BROADCAST_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000)); //��ʼ�����Թ㲥����
}

/*********************************************************************
 * @fn      lwns_broadcast_ProcessEvent
 *
 * @brief   lwns broadcast Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_broadcast_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & BROADCAST_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; //��λ�������ݣ��Ա�۲�Ч��
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        lwns_broadcast_send(&broadcast);                 //�㲥��������
        tmos_start_task(broadcast_taskID, BROADCAST_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); //�����Է���

        return events ^ BROADCAST_EXAMPLE_TX_PERIOD_EVT;
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
