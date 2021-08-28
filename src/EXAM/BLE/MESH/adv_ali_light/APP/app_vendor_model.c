/********************************** (C) COPYRIGHT *******************************
* File Name          : app_vendor_model.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/03/24
* Description        : 
*******************************************************************************/



/******************************************************************************/
#include "app_mesh_config.h"
#include "CH58x_common.h"
#include "MESH_LIB.h"
#include "CONFIG.h"
#include "app_vendor_model.h"
#include "app_generic_onoff_model.h"


/*********************************************************************
 * GLOBAL TYPEDEFS
 */
///* Mac Address Light1 */
//#define PID				7049884
//#define MAC_ADDR 		{0x18,0x14,0x6c,0x49,0x54,0x98}
//#define ALI_SECRET		{0x35,0x06,0xa0,0xe9,0x60,0xde,0xbd,0x59,0xcf,0xcc,0xce,0x11,0x2c,0x91,0xbf,0x3f}
/* Mac Address Light2 */
#define PID				9701
#define MAC_ADDR  		{0xf8,0xa7,0x63,0x6a,0xec,0x3f}
#define ALI_SECRET		{0x6b,0xfa,0x68,0x6f,0x9d,0x1b,0x37,0x00,0x01,0xd1,0xfd,0xb8,0x27,0x7d,0xc0,0x81}

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

u8_t tm_uuid[16];
u8_t static_key[16];
u8C MacAddr[6] = MAC_ADDR;

static const struct bt_als_cfg cfg = 
{
	.mac = MAC_ADDR,
	.secret = ALI_SECRET,
	.pid = PID,
	.cid = 0x01a8,
	.version = 0x00060000,
};

static uint8 als_vendor_model_TaskID = 0;    // Task ID for internal task/event processing

static u8_t als_tid=0;

static struct net_buf ind_buf[CONFIG_INDICATE_NUM]={0};
static struct bt_mesh_indicate indicate[CONFIG_INDICATE_NUM]={0};

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
static void ind_reset(struct bt_mesh_indicate *ind, int err);
static uint16 als_vendor_model_ProcessEvent( uint8 task_id, uint16 events );

static void tm_attr_get(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
	APP_DBG(" ");
}

static void tm_attr_set(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
	APP_DBG(" ");
}

static void tm_attr_set_unack(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
	APP_DBG(" ");
}

static void tm_attr_status(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
	APP_DBG(" ");
}

/*******************************************************************************
* Function Name  : tm_attr_confirm
* Description    : �յ���è���鷢����confirm - ����Ϣ����Vendor Model Client�ظ���Vendor Model Server��
										���ڱ�ʾ���յ�Vendor Model Server������Indication
* Input          : None
* Return         : None
*******************************************************************************/
static void tm_attr_confirm(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
	u8_t recv_tid;
	
	recv_tid = net_buf_simple_pull_u8(buf);
	
	APP_DBG("src: 0x%04x dst: 0x%04x tid 0x%02x rssi: %d", 
					ctx->addr, ctx->recv_dst, recv_tid, ctx->recv_rssi);

	for(int i = 0; i < CONFIG_INDICATE_NUM; i++)
	{
		if (indicate[i].param.tid == recv_tid)
		{
			ind_reset(&indicate[i], 0);
			tmos_stop_task(als_vendor_model_TaskID, indicate[i].event);
			continue;
		}
	}
}

/*******************************************************************************
* Function Name  : tm_attr_trans
* Description    : ����Ϣ����Mesh�豸����è����֮��͸������
* Input          : None
* Return         : None
*******************************************************************************/
static void tm_attr_trans(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
	APP_DBG(" ");
}

// opcode ��Ӧ�Ĵ�����
static const struct bt_mesh_model_op vnd_model_op[] = {
  {OP_VENDOR_MESSAGE_ATTR_GET, 				0, tm_attr_get},
	{OP_VENDOR_MESSAGE_ATTR_SET, 				0, tm_attr_set},
	{OP_VENDOR_MESSAGE_ATTR_SET_UNACK, 			0, tm_attr_set_unack},
	{OP_VENDOR_MESSAGE_ATTR_STATUS, 			0, tm_attr_status},
	{OP_VENDOR_MESSAGE_ATTR_CONFIRMATION, 		1, tm_attr_confirm},
	{OP_VENDOR_MESSAGE_ATTR_TRANSPARENT_MSG, 	0, tm_attr_trans},
  BLE_MESH_MODEL_OP_END,
};

struct bt_mesh_model vnd_models[] = {
  BLE_MESH_MODEL_VND_CB(CID_ALI_GENIE, 0x0000, vnd_model_op, NULL, NULL, &bt_mesh_als_vendor_model_cb),
};

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*******************************************************************************
* Function Name  : als_avail_tid_get
* Description    : TODO TID selection method
* Input          : None
* Return         : None
*******************************************************************************/
u8_t als_avail_tid_get(void)
{
	return als_tid++;
}

/*******************************************************************************
* Function Name  : uuid_generate
* Description    : ���ɰ���淶��UUID
* Input          : cfg��������Ϣ
* Return         : None
*******************************************************************************/
static void uuid_generate(struct bt_als_cfg const *cfg)
{
	/* Company ID */
	tm_uuid[0] = cfg->cid;
	tm_uuid[1] = cfg->cid >> 8;
	
	/*	bit3��0 �������㲥���汾�ţ�Ŀǰ��0x01
			bit4Ϊ1��һ��һ��
			bit5Ϊ1��֧��OTA
			bit7��6������Э��汾
								00��BLE4.0
								01��BLE4.2
								10��BLE5.0
								11��BLE5.0���� */
	/* Advertising Verison */
	tm_uuid[2] = BIT(0) |	/* adv version */
							 BIT(4) |	/* secret */
							 BIT(5) |	/* ota */
							 BIT(7); 	/* ble verison */
	
	/* Product Id */
	tm_uuid[2 + 1] = cfg->pid;
	tm_uuid[2 + 2] = cfg->pid >> 8;
	tm_uuid[2 + 3] = cfg->pid >> 16;
	tm_uuid[2 + 4] = cfg->pid >> 24;

	/* Device Mac Address */
	for(int i=0;i<6;i++) tm_uuid[2 + 1 + 4 + i] = cfg->mac[5-i];
	
	/* UUID Verison */
	tm_uuid[2 + 1 + 4 + 6] = BIT(1);
	
	/* RFU */
	tm_uuid[2 + 1 + 4 + 6 + 1] = 0x00;
	tm_uuid[2 + 1 + 4 + 6 + 2] = 0x00;
}

/*******************************************************************************
* Function Name  : num_to_str
* Description    : ����ת�ַ�
* Input          : 
* Return         : None
*******************************************************************************/
static void num_to_str(u8_t *out, const u8_t *in, u16_t in_len)
{
	u16_t i;
	static const char hex[] = "0123456789abcdef";

	for (i = 0; i < in_len; i++)
	{
        out[i * 2]     = hex[in[i] >> 4];
        out[i * 2 + 1] = hex[in[i] & 0xf];
	}
}

/*******************************************************************************
* Function Name  : oob_key_generate
* Description    : ����OOB key
* Input          : cfg��������Ϣ
* Return         : None
*******************************************************************************/
static void oob_key_generate(struct bt_als_cfg const *cfg)
{
	int err;
	u32_t pid;
	u8_t out[8 + 1 + 12 + 1 + 32], dig[32];
	struct tc_sha256_state_struct s;

  tc_sha256_init(&s);
			
	/** pid value */
	pid = ((u32_t)((((cfg->pid) >> 24) & 0xff) |  \
								 (((cfg->pid) >> 8) & 0xff00) | \
								 (((cfg->pid)&0xff00) << 8) |   \
								 (((cfg->pid)&0xff) << 24)));
	num_to_str(out, (void *)&pid, 4);

	/** Separator */
  strcpy((void *)(out + 8), ",");
	/** mac value */
	num_to_str(out + 8 + 1, (void *)cfg->mac, 6);

	/** Separator */
  strcpy((void *)(out + 8 + 1 + 12), ",");
	/** secret value */
	num_to_str(out + 8 + 1 + 12 + 1, (void *)cfg->secret, 16);

	err = tc_sha256_update(&s, out, sizeof(out));
	if (err != TC_CRYPTO_SUCCESS)
	{
			APP_DBG("Unable Update Sha256");
			return;
	}

	err = tc_sha256_final(dig, &s);
	if (err != TC_CRYPTO_SUCCESS)
	{
			APP_DBG("Unable Generate sha256 value");
			return;
	}
	
	memcpy(static_key, dig, 16);
}

/*******************************************************************************
* Function Name  : ind_reset
* Description    : �Ƴ��б����÷�����ɻص����ͷŻ���
* Input          : ind����Ҫ���õ�֪ͨ
										err�� ������
* Return         : None
*******************************************************************************/
static void ind_reset(struct bt_mesh_indicate *ind, int err)
{
	if (ind->param.cb && ind->param.cb->end)
	{
			ind->param.cb->end(err, ind->param.cb_data);
	}

	tmos_msg_deallocate(ind->buf->__buf);
	ind->buf->__buf = NULL;
}

/*******************************************************************************
* Function Name  : bt_mesh_indicate_reset
* Description    : �ͷ�����δ���͵�֪ͨ
* Input          : None
* Return         : None
*******************************************************************************/
void bt_mesh_indicate_reset(void)
{
	uint8 i;
	for(i = 0; i < CONFIG_INDICATE_NUM; i++)
	{
		if( indicate[i].buf->__buf != NULL )
		{
			ind_reset(&indicate[i], -ECANCELED);			
		}
	}
}

/*******************************************************************************
* Function Name  : ind_start
* Description    : ���� indicate ��ʼ�ص�
* Input          : duration�����η��ͽ�Ҫ������ʱ��
										err�� ������
										cb_data�� �ص�����
* Return         : None
*******************************************************************************/
static void ind_start(u16_t duration, int err, void *cb_data)
{
	struct bt_mesh_indicate *ind = cb_data;

	if (ind->buf->__buf == NULL)
	{
		return;
	}

	if (err)
	{
		APP_DBG("Unable send indicate (err:%d)", err);
		tmos_start_task(als_vendor_model_TaskID, ind->event, K_MSEC(100) );
		return;
	}
}

/*******************************************************************************
* Function Name  : ind_end
* Description    : ���� indicate �����ص�
* Input          : err�� ������
										cb_data�� �ص�����
* Return         : None
*******************************************************************************/
static void ind_end(int err, void *cb_data)
{
	struct bt_mesh_indicate *ind = cb_data;

	if (ind->buf->__buf == NULL)
	{
		return;
	}
	tmos_start_task(als_vendor_model_TaskID, ind->event, ind->param.period );
}

// ���� indicate �ص��ṹ��
const struct bt_mesh_send_cb ind_cb = 
{
	.start = ind_start,
	.end = ind_end,
};

/*******************************************************************************
* Function Name  : adv_ind_send
* Description    : ���� indicate 
* Input          : ind����Ҫ���͵�֪ͨ
* Return         : None
*******************************************************************************/
static void adv_ind_send(struct bt_mesh_indicate *ind)
{
	int err;
	NET_BUF_SIMPLE_DEFINE(msg, 32);

	struct bt_mesh_msg_ctx ctx = {
		.app_idx = vnd_models[0].keys[0],
		.addr = ALI_TM_SUB_ADDRESS,
	};

	if (ind->buf->__buf == NULL)
	{
		APP_DBG("NULL buf");
		return;
	}

	if (ind->param.trans_cnt == 0)
	{
		ind_reset(ind, -ETIMEDOUT);
		return;
	}

	ind->param.trans_cnt --;

	ctx.send_ttl = ind->param.send_ttl;

	/** TODO */
	net_buf_simple_add_mem(&msg, ind->buf->data, ind->buf->len);

	err = bt_mesh_model_send(vnd_models, &ctx, &msg, &ind_cb, ind);
	if (err)
	{
		APP_DBG("Unable send model message (err:%d)", err);
		ind_reset(ind, -EIO);
		return;
	}
}

/*******************************************************************************
* Function Name  : bt_mesh_ind_alloc��һ���յ�
* Description    : ��һ���յ�indicate���������ڴ�
* Input          : buf: ��Ҫ�������������
* Return         : indicate�ṹ��ָ��
*******************************************************************************/
struct bt_mesh_indicate *bt_mesh_ind_alloc( uint16 len )
{
	uint8 i;
	for(i = 0; i < CONFIG_INDICATE_NUM; i++)
	{
		if( indicate[i].buf->__buf == NULL )
			break;
	}
	if( i == CONFIG_INDICATE_NUM )
		return NULL;
	
	indicate[i].buf->__buf = tmos_msg_allocate(len);
	indicate[i].buf->size = len;
	
	if( indicate[i].buf->__buf == NULL )
		return NULL;
			
	return &indicate[i];
}

/*******************************************************************************
* Function Name  : bt_mesh_indicate_send
* Description    : ��������֪ͨ���¼�
* Input          : None
* Return         : None
*******************************************************************************/
void bt_mesh_indicate_send( struct bt_mesh_indicate *ind )
{
	tmos_start_task(als_vendor_model_TaskID, ind->event, ind->param.rand );
}

/*******************************************************************************
* Function Name  : send_led_indicate
* Description    : ���͵�ǰLED״̬������LED״̬����ʱ����Ҫ���ô˺���
* Input          : param�� ����֪ͨ�ķ��Ͳ���
* Return         : None
*******************************************************************************/
void send_led_indicate(struct indicate_param *param)
{
	struct bt_mesh_indicate *ind;

	ind = bt_mesh_ind_alloc( 16 );
	if (!ind)
	{
		APP_DBG("Unable allocate buffers");
		return;
	}
	memcpy(&(ind->param), param, sizeof(struct indicate_param));

	/* Init indication opcode */
	bt_mesh_model_msg_init(&(ind->buf->b), OP_VENDOR_MESSAGE_ATTR_INDICATION);
    
	/* Add tid field */
	net_buf_simple_add_u8(&(ind->buf->b), param->tid);
	
	/* Add generic onoff attrbute op */
	net_buf_simple_add_le16(&(ind->buf->b), ALI_GEN_ATTR_TYPE_POWER_STATE);
	
	/* Add current generic onoff status */
	net_buf_simple_add_u8(&(ind->buf->b), read_led_state(MSG_PIN));
	
	bt_mesh_indicate_send(ind);
}

/*******************************************************************************
* Function Name  : als_vendor_init
* Description    : ���� ����ģ�� ��ʼ��
* Input          : model: �ص�ģ�Ͳ���
* Return         : None
*******************************************************************************/
static int als_vendor_init(struct bt_mesh_model *model)
{
	u32_t ran;
	
	uuid_generate(&cfg);
	oob_key_generate(&cfg);

	/** Random Local TID Value
  *  @Caution Don't use single octer only.
	*/
	ran = tmos_rand();
	als_tid += ((u8_t *)&ran)[0];
	als_tid += ((u8_t *)&ran)[1];
	als_tid += ((u8_t *)&ran)[2];
	als_tid += ((u8_t *)&ran)[3];
	
	
	for(int i = 0; i < CONFIG_INDICATE_NUM; i++)
	{
		indicate[i].buf = &ind_buf[i];
		indicate[i].event = (1<<i);
	}
	
  als_vendor_model_TaskID = TMOS_ProcessEventRegister( als_vendor_model_ProcessEvent );
	return 0;
}

/*******************************************************************************
* Function Name  : als_vendor_model_ProcessEvent
* Description    : �¼�����
* Input          : 
* Return         : None
*******************************************************************************/
static uint16 als_vendor_model_ProcessEvent( uint8 task_id, uint16 events )
{

	for(int i = 0; i < CONFIG_INDICATE_NUM; i++)
	{
		if ( events & indicate[i].event )
		{
			adv_ind_send( &indicate[i] );
			return ( events ^ indicate[i].event );
		}
	}
	
  // Discard unknown events
  return 0;
}


const struct bt_mesh_model_cb bt_mesh_als_vendor_model_cb = {
	.init = als_vendor_init,
};

/******************************** endfile @ main ******************************/
