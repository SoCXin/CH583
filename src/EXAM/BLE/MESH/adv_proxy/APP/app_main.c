/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/03/24
* Description        : 
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include "CONFIG.h"
#include "CH58x_common.h"
#include "MESH_LIB.h"
#include "HAL.h"
#include "app_mesh_config.h"
#include "app.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE/4];
#if (defined (BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84,0xC2,0xE4,0x03,0x02,0x02};
#endif

/*******************************************************************************
* Function Name  : Main_Circulation
* Description    : ��ѭ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
__HIGH_CODE
void Main_Circulation()
{
  while(1){
    TMOS_SystemProcess( );
  }
}

/*******************************************************************************
* Function Name  : MeshTimer_Init
* Description    : mesh �����ʼ��
* Input          : None
* Return         : None
*******************************************************************************/
u8_t bt_mesh_lib_init(void)
{
	u8_t ret;
	
  if( tmos_memcmp( VER_MESH_LIB,VER_MESH_FILE,strlen(VER_MESH_FILE)) == FALSE ){
    PRINT("mesh head file error...\n");
    while(1);
  }

	ret = RF_RoleInit( );

#if ((CONFIG_BLE_MESH_PROXY) || \
	(CONFIG_BLE_MESH_PB_GATT) || \
	(CONFIG_BLE_MESH_OTA))
	ret = GAPRole_PeripheralInit();
#endif /* PROXY || PB-GATT || OTA */

#if (CONFIG_BLE_MESH_PROXY_CLI)
	ret = GAPRole_CentralInit();
#endif /* CONFIG_BLE_MESH_PROXY_CLI */

	MeshTimer_Init();
	MeshDeamon_Init();
	ble_sm_alg_ecc_init();

#if (CONFIG_BLE_MESH_IV_UPDATE_TEST)
	bt_mesh_iv_update_test(TRUE);
#endif
	return ret;
}

/*******************************************************************************
* Function Name  : main
* Description    : ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main( void ) 
{
  SetSysClock( CLK_SOURCE_PLL_60MHz );

#ifdef DEBUG
  GPIOA_SetBits( bTXD1 );
  GPIOA_ModeCfg( bTXD1, GPIO_ModeOut_PP_5mA );
	UART1_DefInit( );
#endif  
  {
    PRINT("%s\n",VER_LIB);
    PRINT("%s\n",VER_MESH_LIB);
  }
  CH57X_BLEInit( );
  HAL_Init(  );
	bt_mesh_lib_init();
	App_Init();
	Main_Circulation();
}

/******************************** endfile @ main ******************************/
