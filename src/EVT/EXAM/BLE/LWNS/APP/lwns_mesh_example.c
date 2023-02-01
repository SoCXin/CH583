/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_mesh_example.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/28
 * Description        : mesh�����������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "lwns_mesh_example.h"

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

#define MESH_TEST_ROUTE_AUTO       1  //�����Ƿ��Զ�����·�ɱ�
#define MESH_TEST_SELF_ADDR_IDX    2  //��ǰ�����нڵ��ַ����device_addr������
#define MESH_TEST_ADDR_MAX_IDX     2  //��ǰ�����У��ж��ٸ��ڵ�

static uint8_t mesh_test_send_dst = 0;

static lwns_mesh_controller lwns_mesh_test;

static uint8_t           lwns_mesh_test_taskID;
static uint16_t          lwns_mesh_test_ProcessEvent(uint8_t task_id, uint16_t events);
static uint8_t           TX_DATA[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                              0x38, 0x39};
static const lwns_addr_t device_addr[] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, //���ڵ��ַ
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x01}, //��1·�ɽڵ�1
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x02}, //��1�ڵ�2
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x03}, //��1�ڵ�3
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x04}, //��1�ڵ�4
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x01}, //��2·�ɽڵ�1
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x02}, //��2�ڵ�2
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x03}, //��2�ڵ�3
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x04}, //��2�ڵ�4
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x01}, //��3·�ɽڵ�1
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x02}, //��3�ڵ�2
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x03}, //��3�ڵ�3
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x04}, //��3�ڵ�4
};

static void lwns_mesh_recv(lwns_controller_ptr ptr,
                           const lwns_addr_t *sender, uint8_t hops);
static void lwns_mesh_sent(lwns_controller_ptr ptr);
static void lwns_mesh_timedout(lwns_controller_ptr ptr);

/*********************************************************************
 * @fn      lwns_mesh_recv
 *
 * @brief   lwns mesh���ջص�����
 *
 * @param   ptr     -   ���ν��յ�������������mesh���ƽṹ��ָ��.
 * @param   sender  -   ���ν��յ������ݵķ����ߵ�ַָ��.
 * @param   hops    -   ���ν��յ������ݵĴӷ��ͷ������ڵ㾭��������.
 *
 * @return  None.
 */
static void lwns_mesh_recv(lwns_controller_ptr ptr,
                           const lwns_addr_t *sender, uint8_t hops)
{
    PRINTF("mesh %d received from %02x.%02x.%02x.%02x.%02x.%02x: %s (%d)\n",
           get_lwns_object_port(ptr), sender->v8[0], sender->v8[1],
           sender->v8[2], sender->v8[3], sender->v8[4], sender->v8[5],
           (char *)lwns_buffer_dataptr(), lwns_buffer_datalen());
    if(MESH_TEST_SELF_ADDR_IDX != 0)
    {
        //��Ϊ0��֤���Ǵӻ����յ���ظ�����
        lwns_buffer_save_data(TX_DATA);
        tmos_set_event(lwns_mesh_test_taskID, MESH_EXAMPLE_TX_NODE_EVT);
    }
}

/*********************************************************************
 * @fn      lwns_mesh_sent
 *
 * @brief   lwns mesh������ɻص�����
 *
 * @param   ptr     -   ���η�����ɵ�mesh���ƽṹ��ָ��.
 *
 * @return  None.
 */
static void lwns_mesh_sent(lwns_controller_ptr ptr)
{
    PRINTF("mesh %d packet sent\n", get_lwns_object_port(ptr));
}

/*********************************************************************
 * @fn      lwns_mesh_timedout
 *
 * @brief   lwns mesh���ͳ�ʱ�ص�����
 *
 * @param   ptr     -   ���η�����ɵ�mesh���ƽṹ��ָ��.
 *
 * @return  None.
 */
static void lwns_mesh_timedout(lwns_controller_ptr ptr)
{
    //Ѱ��·�ɳ�ʱ�Ż����ûص�������Ѿ���·�ɣ�������һ���ڵ���ߣ��������ûص�����lwns_route_init(TRUE, 60, HTIMER_SECOND_NUM);���������ʱ�䡣
    PRINTF("mesh %d packet timedout\n", get_lwns_object_port(ptr));
}

/**
 * lwns mesh�ص������ṹ�壬ע��ص�����
 */
static const struct lwns_mesh_callbacks callbacks = {lwns_mesh_recv,
                                                     lwns_mesh_sent, lwns_mesh_timedout};

/*
 * mesh����netfloodģ�����Ѱ��·�ɣ����Գ�ʼ�������а�����netflood����Ҫ�Ĳ���
 * meshΪ����·�ɽ��ж���ת���Ļ����ṹ
 * meshģ��ʹ��ǰ�����ʼ��·�ɣ��������ʼ��ʱ��·�ɱ��ṩ���ڴ�ռ䡣
 */

/*********************************************************************
 * @fn      lwns_mesh_process_init
 *
 * @brief   lwns mesh���̳�ʼ��.
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_mesh_process_init(void)
{
    uint8_t route_enable = FALSE;
    lwns_addr_set(
        (lwns_addr_t *)(&device_addr[MESH_TEST_SELF_ADDR_IDX])); //�ı�lwns�ڲ�addr
    if(device_addr[MESH_TEST_SELF_ADDR_IDX].v8[5] == 1)
    {
        //ÿһ���һ���ڵ��·�ɹ��ܣ������ڵ㲻��·�ɹ���
        route_enable = TRUE;
    }
    lwns_mesh_test_taskID = TMOS_ProcessEventRegister(lwns_mesh_test_ProcessEvent);
#if MESH_TEST_ROUTE_AUTO
    lwns_route_init(TRUE, 60, HTIMER_SECOND_NUM);
#else
    lwns_route_init(TRUE, 0, HTIMER_SECOND_NUM); //disable auto clean route entry
#endif                                    /*MESH_TEST_ROUTE_AUTO*/
    lwns_mesh_init(&lwns_mesh_test, 132,  //��һ���˿ں�132��mesh���磬ͬʱռ�ú������������˿�����Ѱ��·�ɣ���133��134Ҳͬʱ���ˡ�
                   HTIMER_SECOND_NUM / 2, //netflood��QUEUETIME����
                   1,                     //netflood��dups����
                   2,                     //�����Ծ�����㼶Ϊ5���������ݰ������Ծ���5��ת���������Ͷ������ݰ�
                   FALSE,                 //��·�������ת�������У��յ����µ���Ҫת�������ݰ��������ݰ������̷��ͳ�ȥ���Ƕ�����FALSEΪ���̷��ͣ�TRUEΪ������
                   50,                    //����ָ���������ֵ������һ����࣬�������ű��ڴ��ڱ�������ݰ����С��ֵ���ڴ�ֵ�������Ϊ������ϻָ����������ո����ݰ���
                   //ͬʱ����ֵҲ�������ж�Ϊ�����ݰ��Ĳ�ֵ��������ͬһ���ڵ�������ݰ�����Ų����Ա��ڴ��еĴ���࣬���ȴ�ֵ����
                   //���磬�ڴ��б����Ϊ10�������ݰ����Ϊ60����ֵΪ50�����ڵ��ڴ�ʱ���õ�50�����Խ����ᱻ��ΪΪ�µ����ݰ�����������
                   //ֻ�����Ϊ59����ֵΪ49��С�ڸ�ֵ���Żᱻ���ա�
                   route_enable,          //�Ƿ�ʹ�ܱ�����·�ɹ��ܣ����Ϊfalse��������Ӧ�����ڵ��·������
                   TRUE,                  //�����Ƿ����·�ɻ�·�������յ���һ������a�ڵ��mesh���ݰ��������Ƿ���Ҫ�洢ǰ��a�ڵ��·�ɱ�FALSE���棬TRUE�档
                   HTIMER_SECOND_NUM * 2, //·�ɳ�ʱʱ�䣬����ʱ�䣬ֹͣѰ��·�ɣ�����timeout�ص�����ֵӦ���� route_discovery_hoptime * (hops+1) * 2
                   &callbacks);           //û�г�ʼ��·�ɱ�Ļ����᷵��0�������ʧ�ܡ�����1�򿪳ɹ���
    if(MESH_TEST_SELF_ADDR_IDX == 0)
    { //�������������ʼ��������ѵ�����ڵ㡣
        mesh_test_send_dst = 1;
        tmos_start_task(lwns_mesh_test_taskID, MESH_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(200));
#if MESH_TEST_ROUTE_AUTO
        PRINTF("Auto route\n");
#else
        uint8_t i;
        for(i = 1; i < 5; i++)
        {
            lwns_route_add(&device_addr[i], &device_addr[1], 2); //�ֶ��������·����Ŀ
        }
        for(i = 5; i < 9; i++)
        {
            lwns_route_add(&device_addr[i], &device_addr[5], 2); //�ֶ��������·����Ŀ
        }
        for(i = 9; i < 13; i++)
        {
            lwns_route_add(&device_addr[i], &device_addr[9], 2); //�ֶ��������·����Ŀ
        }
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 1)
    {
        lwns_route_add(&device_addr[4], &device_addr[4], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[3], &device_addr[3], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[2], &device_addr[2], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[0], &device_addr[0], 1); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 2)
    {
        lwns_route_add(&device_addr[0], &device_addr[1], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 3)
    {
        lwns_route_add(&device_addr[0], &device_addr[1], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 4)
    {
        lwns_route_add(&device_addr[0], &device_addr[1], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 5)
    {
        lwns_route_add(&device_addr[6], &device_addr[6], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[7], &device_addr[7], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[8], &device_addr[8], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[0], &device_addr[0], 1); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 6)
    {
        lwns_route_add(&device_addr[0], &device_addr[5], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 7)
    {
        lwns_route_add(&device_addr[0], &device_addr[5], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 8)
    {
        lwns_route_add(&device_addr[0], &device_addr[5], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 9)
    {
        lwns_route_add(&device_addr[10], &device_addr[10], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[11], &device_addr[11], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[12], &device_addr[12], 1); //�ֶ��������·����Ŀ
        lwns_route_add(&device_addr[0], &device_addr[0], 1);   //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 10)
    {
        lwns_route_add(&device_addr[0], &device_addr[9], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 11)
    {
        lwns_route_add(&device_addr[0], &device_addr[9], 2); //�ֶ��������·����Ŀ
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 12)
    {
        lwns_route_add(&device_addr[0], &device_addr[9], 2); //�ֶ��������·����Ŀ
#endif /*MESH_TEST_ROUTE_AUTO*/
    }
}

/*********************************************************************
 * @fn      lwns_mesh_test_ProcessEvent
 *
 * @brief   lwns mesh Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
static uint16_t lwns_mesh_test_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & MESH_EXAMPLE_TX_PERIOD_EVT)
    { //������������ѯ�ӻ�����
        uint8_t                  temp;
        struct lwns_route_entry *rt;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; //��λ�������ݣ��Ա�۲�Ч��
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA));          //������Ҫ���͵����ݵ�������
        rt = lwns_route_lookup(&device_addr[mesh_test_send_dst]); //��·�ɱ���Ѱ����һ��·��
        if(rt != NULL)
        {
            //��ӡ��һ��·����Ϣ
            PRINTF("dst:%d,forwarding to %02x.%02x.%02x.%02x.%02x.%02x\n",
                   mesh_test_send_dst, rt->nexthop.v8[0], rt->nexthop.v8[1],
                   rt->nexthop.v8[2], rt->nexthop.v8[3], rt->nexthop.v8[4],
                   rt->nexthop.v8[5]);
        }
        else
        {
            PRINTF("no route to dst:%d\n", mesh_test_send_dst);
        }
        lwns_mesh_send(&lwns_mesh_test, &device_addr[mesh_test_send_dst]); //����mesh��Ϣ
        mesh_test_send_dst++;                                              //��ѯĿ��ڵ��Ϊ��һ��
        if(mesh_test_send_dst > MESH_TEST_ADDR_MAX_IDX)
        { //ѭ��������ѯ
            mesh_test_send_dst = 1;
        }
        tmos_start_task(lwns_mesh_test_taskID, MESH_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(2500)); //��������ѯ
        return (events ^ MESH_EXAMPLE_TX_PERIOD_EVT);
    }
    if(events & MESH_EXAMPLE_TX_NODE_EVT)
    { //�ڵ�������������ݵ�����
        struct lwns_route_entry *rt;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //������Ҫ���͵����ݵ�������
        rt = lwns_route_lookup(&device_addr[0]);         //��·�ɱ���Ѱ����һ��·��
        if(rt != NULL)
        {
            //��ӡ����һ��·����Ϣ
            PRINTF("src:%d,forwarding to %02x.%02x.%02x.%02x.%02x.%02x\n",
                   MESH_TEST_SELF_ADDR_IDX, rt->nexthop.v8[0],
                   rt->nexthop.v8[1], rt->nexthop.v8[2], rt->nexthop.v8[3],
                   rt->nexthop.v8[4], rt->nexthop.v8[5]);
        }
        lwns_mesh_send(&lwns_mesh_test, &device_addr[0]); //����mesh���ͺ������������ݵķ���
        return (events ^ MESH_EXAMPLE_TX_NODE_EVT);
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
