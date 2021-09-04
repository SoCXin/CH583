/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2019/11/05
 * Description        : ����ӻ�Ӧ��������������ϵͳ��ʼ��
 *******************************************************************************/

/******************************************************************************/
/* ͷ�ļ����� */
#include "CH58x_common.h"
#include "OTA.h"

/* ��¼��ǰ��Image */
unsigned char CurrImageFlag = 0xff;

/* flash��������ʱ�洢 */
__attribute__((aligned(8))) UINT8 block_buf[16];


#define    jumpApp  ((  void  (*)  ( void ))  ((int*)IMAGE_A_START_ADD))



/*********************************************************************
 * GLOBAL TYPEDEFS
 */

/*******************************************************************************
* Function Name  : SwitchImageFlag
* Description    : �л�dataflash���ImageFlag
* Input          : new_flag���л���ImageFlag
* Output         : none
* Return         : none
*******************************************************************************/
void SwitchImageFlag(UINT8 new_flag)
{
  UINT16 i;
  UINT32  ver_flag;

  /* ��ȡ��һ�� */
  EEPROM_READ( OTA_DATAFLASH_ADD, (PUINT32) &block_buf[0], 4 );

  /* ������һ�� */
  EEPROM_ERASE(OTA_DATAFLASH_ADD,EEPROM_PAGE_SIZE);

  /* ����Image��Ϣ */
  block_buf[0] = new_flag;

  /* ���DataFlash */
  EEPROM_WRITE(OTA_DATAFLASH_ADD, (PUINT32) &block_buf[0], 4);
}

/*******************************************************************************
 * Function Name  : jump_APP
 * Description    :
 * Input          : None
 * Return         : None
 *******************************************************************************/
void jump_APP( void )
{
  if( CurrImageFlag==IMAGE_IAP_FLAG )
  {
    __attribute__((aligned(8))) UINT8 flash_Data[1024];
    UINT8 i;
    FLASH_ROM_LOCK( 0 );                    // ����flash
    FLASH_ROM_ERASE( IMAGE_A_START_ADD, IMAGE_A_SIZE );
    for(i=0; i<IMAGE_A_SIZE/1024; i++)
    {
      FLASH_ROM_READ( IMAGE_B_START_ADD+(i*1024), flash_Data, 1024 );
      FLASH_ROM_WRITE(IMAGE_A_START_ADD+(i*1024),flash_Data,1024);
    }
    SwitchImageFlag( IMAGE_A_FLAG );
    // ���ٱ��ݴ���
    FLASH_ROM_ERASE( IMAGE_B_START_ADD, IMAGE_A_SIZE );
  }
  jumpApp();
}

/*******************************************************************************
 * Function Name  : ReadImageFlag
 * Description    : ��ȡ��ǰ�ĳ����Image��־��DataFlash���Ϊ�գ���Ĭ����ImageA
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ReadImageFlag( void )
{
  OTADataFlashInfo_t p_image_flash;

  EEPROM_READ( OTA_DATAFLASH_ADD, &p_image_flash, 4 );
  CurrImageFlag = p_image_flash.ImageFlag;

  /* �����һ��ִ�У�����û�и��¹����Ժ���º��ڲ���DataFlash */
  if ( (CurrImageFlag != IMAGE_A_FLAG) && (CurrImageFlag != IMAGE_B_FLAG) && (CurrImageFlag != IMAGE_IAP_FLAG) )
  {
    CurrImageFlag = IMAGE_A_FLAG;
  }

  PRINT( "Image Flag %02x\n", CurrImageFlag );
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
#if (defined (DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
  PWR_DCDCCfg( ENABLE );
#endif
  SetSysClock( CLK_SOURCE_PLL_60MHz );
#if (defined (HAL_SLEEP)) && (HAL_SLEEP == TRUE)
  GPIOA_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PU );
  GPIOB_ModeCfg( GPIO_Pin_All, GPIO_ModeIN_PU );
#endif
#ifdef DEBUG
  GPIOA_SetBits( bTXD1 );
  GPIOA_ModeCfg( bTXD1, GPIO_ModeOut_PP_5mA );
  UART1_DefInit();
#endif   
  FLASH_ROM_LOCK(0);
  ReadImageFlag();
//  while(1);
  jump_APP();
}

/******************************** endfile @ main ******************************/
