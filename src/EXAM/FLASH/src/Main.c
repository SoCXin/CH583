/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description 		    : FALSH¶ÁÐ´Àý³Ì
 *******************************************************************************/

#include "CH58x_common.h"

UINT8 TestBuf[1024];

void DebugInit( void )
{
  GPIOA_SetBits( GPIO_Pin_9 );
  GPIOA_ModeCfg( GPIO_Pin_8, GPIO_ModeIN_PU );
  GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );
  UART1_DefInit();
}

int main()
{
  UINT16 i;
  UINT8 s;

  SetSysClock( CLK_SOURCE_PLL_60MHz );

  /* ÅäÖÃ´®¿Úµ÷ÊÔ */
  DebugInit();
  PRINT( "Start @ChipID=%02X\n", R8_CHIP_ID );

#if 1 // ¶ÁÐ´Data-Flash

  FLASH_ROM_LOCK(0);
  PRINT( "EEPROM_READ...\n" );
  EEPROM_READ( 0, TestBuf, 500 );
  for ( i = 0; i < 500; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

  s = EEPROM_ERASE( 0, EEPROM_BLOCK_SIZE );
  PRINT( "EEPROM_ERASE=%02x\n", s );
  PRINT( "EEPROM_READ...\n" );
  EEPROM_READ( 0, TestBuf, 500 );
  for ( i = 0; i < 500; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

  for ( i = 0; i < 500; i++ )
    TestBuf[i] = 0x0 + i;
  s = EEPROM_WRITE( 0, TestBuf, 500 );
  PRINT( "EEPROM_WRITE=%02x\n", s );
  PRINT( "EEPROM_READ...\n" );
  EEPROM_READ( 0, TestBuf, 500 );
  for ( i = 0; i < 500; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

#endif

#if 1   // »ñÈ¡Î¨Ò»ID£¬MACµØÖ·£¬¶ÁÐ´Flash-ROM

  PRINT( "GET_UNIQUE_ID...\n" );
  GET_UNIQUE_ID( TestBuf );
  for ( i = 0; i < 8; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

  PRINT( "FlashMACADDRESS...\n" );
  GetMACAddress( TestBuf );
  for ( i = 0; i < 6; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

  PRINT( "FLASH_READ 20k...\n" );
  FLASH_ROM_READ( 20 * 1024, TestBuf, 128 );
  for ( i = 0; i < 128; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

  s = FLASH_ROM_ERASE( 20 * 1024, 4096 );
  PRINT( "FLASH_ROM_ERASE=%02x\n", s );
  PRINT( "FLASH_READ 20k...\n" );
  FLASH_ROM_READ( 20 * 1024, TestBuf, 128 );
  for ( i = 0; i < 128; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );

  for ( i = 0; i < 128; i++ )
  {
    TestBuf[i] = 0x70 + i;
  }
  s = FLASH_ROM_WRITE( 20 * 1024, TestBuf, 128 );
  PRINT( "FlashWrite=%02x\n", s );
  s = FLASH_ROM_VERIFY( 20 * 1024, TestBuf, 128 );
  PRINT( "FlashVerify=%02x\n", s );

  PRINT( "FLASH_READ...\n" );
  FLASH_ROM_READ( 20 * 1024, TestBuf, 128 );
  for ( i = 0; i < 128; i++ )
  {
    PRINT( "%02x ", TestBuf[i] );
  }
  PRINT( "\n" );


#endif

  while( 1 )
    ;
}

