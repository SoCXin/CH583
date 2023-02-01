/********************************** (C) COPYRIGHT *******************************
 * File Name          : app_trans_process.h
 * Author             : WCH
 * Version            : V1.1
 * Date               : 2022/03/31
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

#ifndef app_trans_process_H
#define app_trans_process_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define LED_PIN    GPIO_Pin_18

/******************************************************************************/
/**
 * @brief   ��ȡled״̬
 *
 * @param   led_pin - ����
 *
 * @return  led״̬
 */
BOOL read_led_state(uint32_t led_pin);

/**
 * @brief   ����led״̬
 *
 * @param   led_pin - ����
 * @param   on      - ״̬
 */
void set_led_state(uint32_t led_pin, BOOL on);

/**
 * @brief   ��תled״̬
 *
 * @param   led_pin - ����
 */
void toggle_led_state(uint32_t led_pin);

/**
 * @brief   ����trans����
 *
 * @param   pValue      - ����ָ��.
 *          len         - ���ݳ���.
 *          src_Addr    - ������Դ��ַ.
 *          dst_Addr    - ����Ŀ�ĵ�ַ.
 */
extern void app_trans_process(uint8_t *pValue, uint8_t len, uint16_t src_Addr, uint16_t dst_Addr);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
