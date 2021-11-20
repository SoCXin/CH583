/********************************** (C) COPYRIGHT *******************************
* File Name          : app_als_windspeed_attr.h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/11/12
* Description        : 
*******************************************************************************/

#ifndef app_als_windspeed_attr_H
#define app_als_windspeed_attr_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "MESH_LIB.h"


u8_t read_windspeed( void );
void set_windspeed( u8_t windspeed );

void gen_windspeed_get(struct bt_mesh_model *model,
															struct bt_mesh_msg_ctx *ctx,
															struct net_buf_simple *buf);
void gen_windspeed_set(struct bt_mesh_model *model,
															struct bt_mesh_msg_ctx *ctx,
															struct net_buf_simple *buf);
void gen_windspeed_set_unack(struct bt_mesh_model *model,
																		struct bt_mesh_msg_ctx *ctx,
																		struct net_buf_simple *buf);


#ifdef __cplusplus
}
#endif

#endif
