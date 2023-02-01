/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_ruc_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/30
 * Description        : reliable unicast���ɿ�������������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_ruc_example.h"

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

static lwns_ruc_controller ruc; //�����ɿ��������ƽṹ��

static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static uint8_t RX_DATA[10];

static uint8_t ruc_taskID;
uint16_t       lwns_ruc_ProcessEvent(uint8_t task_id, uint16_t events);

static void recv_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t  *sender);

static void sent_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t *to, uint8_t retransmissions);
static void timedout_ruc(lwns_controller_ptr ptr,
                         const lwns_addr_t  *to);

/*********************************************************************
 * @fn      recv_ruc
 *
 * @brief   lwns ruc���ջص�����
 *
 * @param   ptr     -   ���ν��յ������������Ŀɿ��������ƽṹ��ָ��.
 * @param   sender  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 *
 * @return  None.
 */
static void recv_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t  *sender)
{
    //ruc�н��յ����͸��Լ������ݺ󣬲Ż���øûص�
    uint8_t len;
    len = lwns_buffer_datalen(); //��ȡ��ǰ���������յ������ݳ���
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); //�������ݵ��û���������
        PRINTF("ruc %d rec %02x %02x %02x %02x %02x %02x\r\n", get_lwns_object_port(ptr), sender->v8[0],
               sender->v8[1], sender->v8[2], sender->v8[3], sender->v8[4], sender->v8[5]);
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
 * @fn      sent_ruc
 *
 * @brief   lwns ruc������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵĿɿ��������ƽṹ��ָ��.
 *
 * @return  None.
 */
static void sent_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t *to, uint8_t retransmissions)
{
    //ruc�з��ͳɹ��������յ�Ŀ��ڵ��ack�ظ��󣬲Ż���øûص�
    PRINTF("ruc %d sent %d\r\n", get_lwns_object_port(ptr), retransmissions);
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000)); //��������ʱ�䣬���Ͳ��յ��ظ���1���Ӻ��ٷ���
}

/*********************************************************************
 * @fn      timedout_ruc
 *
 * @brief   lwns ruc���ͳ�ʱ�ص�����
 *
 * @param   ptr     -   ���η�����ɵ�ruc���ƽṹ��ָ��.
 *
 * @return  None.
 */
static void timedout_ruc(lwns_controller_ptr ptr,
                         const lwns_addr_t  *to)
{
    //ruc�У����ط�������������ط������󣬻���øûص���
    PRINTF("ruc %d timedout\n", get_lwns_object_port(ptr));
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/**
 * lwns �ɿ������ص������ṹ�壬ע��ص�����
 */
static const struct lwns_ruc_callbacks ruc_callbacks = {
    recv_ruc, sent_ruc, timedout_ruc}; //�����ɿ������ص��ṹ��

/*********************************************************************
 * @fn      lwns_ruc_process_init
 *
 * @brief   lwns ruc���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_ruc_process_init(void)
{
    lwns_ruc_init(&ruc,
                  144,               //��һ���˿ں�Ϊ144�Ŀɿ�����
                  HTIMER_SECOND_NUM, //�ȴ�ackʱ������û�յ��ͻ��ط�
                  &ruc_callbacks);   //����0�����ʧ�ܡ�����1�򿪳ɹ���
    ruc_taskID = TMOS_ProcessEventRegister(lwns_ruc_ProcessEvent);
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_ruc_ProcessEvent
 *
 * @brief   lwns ruc Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_ruc_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & RUC_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; //��λ�������ݣ��Ա�۲�Ч��
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        lwns_ruc_send(&ruc,
                      &dst_addr, //�ɿ�����Ŀ���ַ
                      4          //����ط�����
        );                       //�ɿ��������ͺ��������Ͳ�����Ŀ���ַ������ط�������Ĭ��һ�����ط�һ��
        return events ^ RUC_EXAMPLE_TX_PERIOD_EVT;
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
