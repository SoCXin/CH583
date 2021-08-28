/********************************** (C) COPYRIGHT *******************************
 * File Name          : Main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description 		    : ��ʱ������
 *******************************************************************************/

#include "CH58x_common.h"

__attribute__((aligned(4)))   UINT32 CapBuf[100];
UINT8V capFlag = 0;

void DebugInit( void )
{
  GPIOA_SetBits( GPIO_Pin_9 );
  GPIOA_ModeCfg( GPIO_Pin_8, GPIO_ModeIN_PU );
  GPIOA_ModeCfg( GPIO_Pin_9, GPIO_ModeOut_PP_5mA );
  UART1_DefInit();
}

int main()
{
  UINT8 i;

  SetSysClock( CLK_SOURCE_PLL_60MHz );

  /* ���ô��ڵ��� */
  DebugInit();
  PRINT( "Start @ChipID=%02X\n", R8_CHIP_ID );
  
#if 1       /* ��ʱ��0���趨100ms��ʱ������IO�����ƣ� PB15-LED */

  GPIOB_SetBits( GPIO_Pin_15 );
  GPIOB_ModeCfg( GPIO_Pin_15, GPIO_ModeOut_PP_5mA );

  TMR0_TimerInit( FREQ_SYS / 10 );                  // ���ö�ʱʱ�� 100ms
  TMR0_ITCfg( ENABLE, TMR0_3_IT_CYC_END );          // �����ж�
  PFIC_EnableIRQ( TMR0_IRQn );
#endif 

#if 1       /* ��ʱ��3��PWM��� */

  GPIOB_ResetBits( GPIO_Pin_22 );            // ����PWM�� PB22
  GPIOB_ModeCfg( GPIO_Pin_22, GPIO_ModeOut_PP_5mA );

  TMR3_PWMInit( High_Level, PWM_Times_1 );
  TMR3_PWMCycleCfg( 6000 );        // ���� 100us
  TMR3_Disable();
  TMR3_PWMActDataWidth( 3000 );              // ռ�ձ� 50%, �޸�ռ�ձȱ�����ʱ�رն�ʱ��
  TMR3_Enable();

#endif   

#if 1       /* ��ʱ��1��CAP��׽�� */
  PWR_UnitModCfg( DISABLE, UNIT_SYS_LSE );     // ע���������LSE�������ţ�Ҫ��֤�رղ���ʹ����������
  GPIOA_ResetBits( GPIO_Pin_10 );             // ����PWM�� PA10
  GPIOA_ModeCfg( GPIO_Pin_10, GPIO_ModeIN_PU );

  TMR1_CapInit( Edge_To_Edge );
  TMR1_CAPTimeoutCfg( 0xFFFFFFFF );    // ���ò�׽��ʱʱ��
  TMR1_DMACfg( ENABLE, ( UINT16 ) ( UINT32 ) &CapBuf[0], ( UINT16 ) ( UINT32 ) &CapBuf[100], Mode_Single );
  TMR1_ITCfg( ENABLE, TMR1_2_IT_DMA_END );          // ����DMA����ж�
  PFIC_EnableIRQ( TMR1_IRQn );

  while( capFlag == 0 )
    ;
  capFlag = 0;
  for ( i = 0; i < 100; i++ )
  {
    printf( "%08ld ", CapBuf[i] & 0x1ffffff );      // bit26 ���λ��ʾ �ߵ�ƽ���ǵ͵�ƽ
  }
  printf( "\n" );

#endif

#if 1       /* ��ʱ��2���������� */
  GPIOB_ModeCfg( GPIO_Pin_11, GPIO_ModeIN_PD );
  GPIOPinRemap( ENABLE, RB_PIN_TMR2 );

  TMR2_EXTSingleCounterInit( FallEdge_To_FallEdge );
  TMR2_CountOverflowCfg( 1000 );  // ���ü�������1000

/* ������������жϣ�����1000�����ڽ����ж� */
  TMR2_ClearITFlag( TMR0_3_IT_CYC_END );
  PFIC_EnableIRQ(TMR2_IRQn);
  TMR2_ITCfg( ENABLE, TMR0_3_IT_CYC_END);

  do
  {
    /* 1s��ӡһ�ε�ǰ����ֵ�������������Ƶ�ʽϸߣ����ܺܿ�����������Ҫ��ʵ������޸� */
    mDelaymS(1000);
    printf("=%ld \n", TMR2_GetCurrentCount());
  }while(1);

#endif

  while( 1 )
    ;
}

__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler( void )        // TMR0 ��ʱ�ж�
{
  if ( TMR0_GetITFlag( TMR0_3_IT_CYC_END ) )
  {
    printf( "d\n" );
    TMR0_ClearITFlag( TMR0_3_IT_CYC_END );      // ����жϱ�־
    GPIOB_InverseBits( GPIO_Pin_15 );
  }
}

__INTERRUPT
__HIGH_CODE
void TMR1_IRQHandler( void )        // TMR1 ��ʱ�ж�
{
  if ( TMR1_GetITFlag( TMR1_2_IT_DMA_END ) )
  {
    TMR1_ITCfg( DISABLE, TMR1_2_IT_DMA_END );       // ʹ�õ���DMA����+�жϣ�ע����ɺ�رմ��ж�ʹ�ܣ������һֱ�ϱ��жϡ�
    TMR1_ClearITFlag( TMR1_2_IT_DMA_END );      // ����жϱ�־
    capFlag = 1;
    printf( "*\n" );
  }
}

__INTERRUPT
__HIGH_CODE
void TMR2_IRQHandler(void)
{
   if( TMR2_GetITFlag(TMR0_3_IT_CYC_END) )
   {
       TMR2_ClearITFlag( TMR0_3_IT_CYC_END );
       /* ������������Ӳ���Զ����㣬���¿�ʼ���� */
       /* �û������������Ҫ�Ĵ��� */
   }

}

