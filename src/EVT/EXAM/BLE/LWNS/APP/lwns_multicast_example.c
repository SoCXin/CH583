/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_multicast_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/20
 * Description        : single-hop multicast���鲥��������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_multicast_example.h"

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

static uint8_t subaddrs_index = 0;                 //���Ͷ��ĵ�ַ���
#define SUBADDR_NUM    3                           //���ĵ�ַ����
static uint16_t subaddrs[SUBADDR_NUM] = {1, 2, 3}; //���ĵ�ַ����

static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static uint8_t  RX_DATA[10];
static uint16_t lwns_multicast_ProcessEvent(uint8_t task_id, uint16_t events);
static void     multicast_recv(lwns_controller_ptr c, uint16_t subaddr, const lwns_addr_t *sender); //�鲥���ջص�����
static void     multicast_sent(lwns_controller_ptr ptr);                                            //�鲥������ɻص�����

static lwns_multicast_controller multicast; //�����鲥���ƽṹ��

static uint8_t multicast_taskID; //�����鲥��������id

/*********************************************************************
 * @fn      multicast_recv
 *
 * @brief   lwns multicast���ջص�����
 *
 * @param   ptr     -   ���ν��յ��������������鲥���ƽṹ��ָ��.
 * @param   subaddr -   ���ν��յ������ݵĶ��ĵ�ַ.
 * @param   sender  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 *
 * @return  None.
 */
static void multicast_recv(lwns_controller_ptr ptr, uint16_t subaddr, const lwns_addr_t *sender)
{
    uint8_t len;
    len = lwns_buffer_datalen(); //��ȡ��ǰ���������յ������ݳ���
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
        PRINTF("multicast %d rec from %02x %02x %02x %02x %02x %02x\n",
               get_lwns_object_port(ptr),
               sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
               sender->v8[4], sender->v8[5]); //fromΪ���յ������ݵķ��ͷ���ַ
        PRINTF("subaddr:%d data:", subaddr);
        for(uint8_t i = 0; i < len; i++)
        {
            PRINTF("%02x ", RX_DATA[i]);
        }
        PRINTF("\n");
    }
    else
    {
        PRINTF("data len err\n");
    }
}

/*********************************************************************
 * @fn      multicast_sent
 *
 * @brief   lwns multicast������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵ��鲥���ƽṹ��ָ��.
 *
 * @return  None.
 */
static void multicast_sent(lwns_controller_ptr ptr)
{
    PRINTF("multicast %d sent\n", get_lwns_object_port(ptr));
}

/**
 * lwns�鲥�ص������ṹ�壬ע��ص�����
 */
static const struct lwns_multicast_callbacks multicast_callbacks =
    {multicast_recv, multicast_sent};

/*********************************************************************
 * @fn      lwns_multicast_process_init
 *
 * @brief   lwns multicast���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_multicast_process_init(void)
{
    multicast_taskID = TMOS_ProcessEventRegister(lwns_multicast_ProcessEvent);
    lwns_multicast_init(&multicast,
                        136,                   //��һ���˿ں�Ϊ136���鲥
                        subaddrs,              //���ĵ�ַ����ָ��
                        SUBADDR_NUM,           //���ĵ�ַ����
                        &multicast_callbacks); //����0�����ʧ�ܡ�����1�򿪳ɹ���
    tmos_start_task(multicast_taskID, MULTICAST_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_multicast_ProcessEvent
 *
 * @brief   lwns multicast Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_multicast_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & MULTICAST_EXAMPLE_TX_PERIOD_EVT)
    { //�������ڲ�ͬ���鲥��ַ�Ϸ����鲥��Ϣ
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
        lwns_multicast_send(&multicast, subaddrs[subaddrs_index]); //�鲥�������ݸ�ָ���ڵ�
        subaddrs_index++;
        tmos_start_task(multicast_taskID, MULTICAST_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); //�����Է���
        return events ^ MULTICAST_EXAMPLE_TX_PERIOD_EVT;
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
