/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_unicast_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/19
 * Description        : lwns������������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_unicast_example.h"

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
static lwns_addr_t dst_addr = {{0xab, 0xdf, 0x38, 0xe4, 0xc2, 0x84}}; //Ŀ��ڵ��ַ������ʱ������ݵ�·��оƬMAC��ַ��ͬ�����޸ġ��޸�Ϊ���շ���MAC��ַ������ʹ���Լ���MAC��ַ
#else
static lwns_addr_t dst_addr = {{0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84}};
#endif

static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static uint8_t  RX_DATA[10];
static uint16_t lwns_unicast_ProcessEvent(uint8_t task_id, uint16_t events);
static void     unicast_recv(lwns_controller_ptr c, const lwns_addr_t *from); //�������ջص�����
static void     unicast_sent(lwns_controller_ptr ptr);                        //����������ɻص�����

static lwns_unicast_controller unicast; //�����������ƽṹ��

static uint8_t unicast_taskID; //����������������id

/*********************************************************************
 * @fn      unicast_recv
 *
 * @brief   lwns unicast���ջص�����
 *
 * @param   ptr     -   ���ν��յ������������ĵ������ƽṹ��ָ��.
 * @param   sender  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 *
 * @return  None.
 */
static void unicast_recv(lwns_controller_ptr ptr, const lwns_addr_t *sender)
{
    uint8_t len;
    len = lwns_buffer_datalen(); //��ȡ��ǰ���������յ������ݳ���
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
        PRINTF("unicast %d rec from %02x %02x %02x %02x %02x %02x\n",
               get_lwns_object_port(ptr),
               sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
               sender->v8[4], sender->v8[5]); //senderΪ���յ������ݵķ��ͷ���ַ
        PRINTF("data:");
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
 * @fn      unicast_sent
 *
 * @brief   lwns unicast������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵĿɿ��������ƽṹ��ָ��.
 *
 * @return  None.
 */
static void unicast_sent(lwns_controller_ptr ptr)
{
    PRINTF("unicast %d sent\n", get_lwns_object_port(ptr));
}

/**
 * lwns �����ص������ṹ�壬ע��ص�����
 */
static const struct lwns_unicast_callbacks unicast_callbacks =
    {unicast_recv, unicast_sent};

/*********************************************************************
 * @fn      lwns_unicast_process_init
 *
 * @brief   lwns unicast���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_unicast_process_init(void)
{
    unicast_taskID = TMOS_ProcessEventRegister(lwns_unicast_ProcessEvent);
    lwns_unicast_init(&unicast,
                      136,                 //��һ���˿ں�Ϊ136�ĵ���
                      &unicast_callbacks); //����0�����ʧ�ܡ�����1�򿪳ɹ���
    tmos_start_task(unicast_taskID, UNICAST_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_unicast_ProcessEvent
 *
 * @brief   lwns unicast Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_unicast_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & UNICAST_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; //��λ�������ݣ��Ա�۲�Ч��
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        lwns_unicast_send(&unicast, &dst_addr);          //�����������ݸ�ָ���ڵ�
        tmos_start_task(unicast_taskID, UNICAST_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); //�����Է���
        return events ^ UNICAST_EXAMPLE_TX_PERIOD_EVT;
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
