/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_als_led_color_attr.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/03/24
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
#include "CONFIG.h"
#include "MESH_LIB.h"
#include "app_mesh_config.h"
#include "app_als_led_color_attr.h"
#include "app_vendor_model.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

#define ALI_DEF_TTL    (10)

// ģ��led_colorֵ
uint8_t device_led_color[6] = {0};

/*******************************************************************************
 * Function Name  : read_led_color
 * Description    : ��ȡ��ǰled_color
 * Input          : None
 * Return         : None
 *******************************************************************************/
void read_led_color(uint8_t *pcolor)
{
    APP_DBG("device_led_color_adj: %d ", device_led_color[0]);
    tmos_memcpy(pcolor, device_led_color, 6);
}

/*******************************************************************************
 * Function Name  : set_led_colors
 * Description    : ���õ�ǰled_color
 * Input          : None
 * Return         : None
 *******************************************************************************/
void set_led_color(uint8_t *pcolor)
{
    tmos_memcpy(device_led_color, pcolor, 6);
}

/*******************************************************************************
 * Function Name  : gen_led_color_status
 * Description    : �ظ���è����led_color
 * Input          : model: ģ�Ͳ���
 *										ctx�����ݲ���
 * Return         : None
 *******************************************************************************/
static void gen_led_color_status(struct bt_mesh_model   *model,
                                 struct bt_mesh_msg_ctx *ctx)
{
    NET_BUF_SIMPLE_DEFINE(msg, 32);
    int     err;
    ////////////////////////////////////////////////////////////////////////
    //  0xD3  0xA8  0x01  |  0x##   |  0x##  0x##       |  0x##  0x## ....//
    //      Opcode        |  TID    | Attribute Type    | Attribute Value //
    ////////////////////////////////////////////////////////////////////////
    bt_mesh_model_msg_init(&msg, OP_VENDOR_MESSAGE_ATTR_STATUS);
    net_buf_simple_add_u8(&msg, als_avail_tid_get());
    net_buf_simple_add_le16(&msg, ALI_GEN_ATTR_TYPE_COLOR);
    net_buf_simple_add_mem(&msg, device_led_color, 6);

    APP_DBG("ttl: 0x%02x dst: 0x%04x", ctx->recv_ttl, ctx->recv_dst);

    if(ctx->recv_ttl != ALI_DEF_TTL)
    {
        ctx->send_ttl = BLE_MESH_TTL_DEFAULT;
    }
    else
    {
        ctx->send_ttl = 0;
    }

    err = bt_mesh_model_send(model, ctx, &msg, NULL, NULL);
    if(err)
    {
        APP_DBG("send status failed: %d", err);
    }
}

/*******************************************************************************
 * Function Name  : gen_led_color_get
 * Description    : ��è�����·��Ļ�ȡled_color����
 * Input          : model: ģ�Ͳ���
 *										ctx�����ݲ���
 *										buf: ��������
 * Return         : None
 *******************************************************************************/
void gen_led_color_get(struct bt_mesh_model   *model,
                       struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple  *buf)
{
    APP_DBG(" ");
    gen_led_color_status(model, ctx);
}

/*******************************************************************************
* Function Name  : gen_led_color_set
* Description    : ��è�����·�������led_color����
                                        ����뵱ǰled_color��ͬ,����Ҫ����ind����è
* Input          : model: ģ�Ͳ���
*										ctx�����ݲ���
*										buf: ��������
* Return         : None
*******************************************************************************/
void gen_led_color_set(struct bt_mesh_model   *model,
                       struct bt_mesh_msg_ctx *ctx,
                       struct net_buf_simple  *buf)
{
    struct indicate_param param = {
        .trans_cnt = 0x09,
        .period = K_MSEC(300),
        .rand = (tmos_rand() % 50),
        .tid = als_avail_tid_get(),
    };

    APP_DBG("ttl: 0x%02x dst: 0x%04x rssi: %d len %d",
            ctx->recv_ttl, ctx->recv_dst, ctx->recv_rssi, buf->len);

    if((buf->data[1] | (buf->data[2] << 8)) == ALI_GEN_ATTR_TYPE_COLOR)
    {
        APP_DBG("%x %x %x %x %x %x %x %x %x",
                buf->data[0], buf->data[1], buf->data[2], buf->data[3], buf->data[4], buf->data[5]
              , buf->data[6], buf->data[7], buf->data[8]);
        // ����Ϊ�趨ֵ
        set_led_color(&(buf->data[3]));
    }

    if(ctx->recv_ttl != ALI_DEF_TTL)
    {
        param.send_ttl = BLE_MESH_TTL_DEFAULT;
    }

    /* Overwrite default configuration */
    if(BLE_MESH_ADDR_IS_UNICAST(ctx->recv_dst))
    {
        param.rand = 0;
        param.send_ttl = BLE_MESH_TTL_DEFAULT;
        param.period = K_MSEC(100);
    }

    send_led_color_indicate(&param);

    gen_led_color_status(model, ctx);
}

/*******************************************************************************
 * Function Name  : gen_led_color_set_unack
 * Description    : ��è�����·�������led_color����(��Ӧ��)
 * Input          : model: ģ�Ͳ���
 *										ctx�����ݲ���
 *										buf: ��������
 * Return         : None
 *******************************************************************************/
void gen_led_color_set_unack(struct bt_mesh_model   *model,
                             struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple  *buf)
{
    APP_DBG(" ");

    if((buf->data[1] | (buf->data[2] << 8)) == ALI_GEN_ATTR_TYPE_COLOR)
    {
        APP_DBG("%x %x %x %x %x %x %x %x %x",
                buf->data[0], buf->data[1], buf->data[2], buf->data[3], buf->data[4], buf->data[5]
              , buf->data[6], buf->data[7], buf->data[8]);
        // ����Ϊ�趨ֵ
        set_led_color(&(buf->data[3]));
    }
}

/******************************** endfile @ main ******************************/
