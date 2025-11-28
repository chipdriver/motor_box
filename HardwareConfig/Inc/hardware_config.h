#ifndef __HARDWARE_CONFIG_H__
#define __HARDWARE_CONFIG_H__

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ================================================================
 *                     硬件资源统计
 * ================================================================
 * STM32F103RCT6 (LQFP64) 实际可用GPIO:
 *   PA: 16个 (PA0-PA15)
 *   PB: 16个 (PB0-PB15, 其中PB3/PB4是JTAG)
 *   PC: 16个 (PC0-PC15)
 *   PD: 2个  (PD0/PD1, 通常用于HSE晶振；若不使用HSE可作GPIO)
 *
 * 总需求(按“霍尔每电机1路输入”修正版):
 *   - 电机控制IO: 12个输出 + 6个霍尔输入 = 18个
 *   - 霍尔输出: 0（硬件确认不用）
 *   - 热控输出: 8个输出
 *   - ADC: 12路
 *   - PWM: 2路
 *   - 通信: RS485或CAN 二选一
 *
 * 引脚分配策略（可先按此写软件，后续硬件定稿改宏）:
 *   PA0-PA7:  ADC + PWM + 通信
 *   PB口:     电机控制 + 热控制 + 预留
 *   PC口:     霍尔输入 + ADC + 电机控制 + 热控
 *   PD2:      预留热控补充脚（仅在“不用外部HSE”且封装实际引出时可用）
 * ================================================================ */


/* ================================================================
 *                  逻辑资源枚举
 * ================================================================ */
#define MOTOR_NUM    6
#define FAN_NUM      2
#define HEAT_NUM     8

typedef enum { MOTOR1=0, MOTOR2, MOTOR3, MOTOR4, MOTOR5, MOTOR6 } motor_id_t;
typedef enum { FAN1=0, FAN2 } fan_id_t;
typedef enum { HEAT1=0, HEAT2, HEAT3, HEAT4, HEAT5, HEAT6, HEAT7, HEAT8 } heat_id_t;


/* ================================================================
 *                  通信接口选择（二选一）
 * ================================================================ */
#define USE_RS485_COMM
// #define USE_CAN_COMM

#ifdef USE_CAN_COMM
    #define COMM_CAN            CAN1
    // CAN需要remap到PB8/PB9
#endif

#ifdef USE_RS485_COMM
    #define COMM_UART           USART1
    #define RS485_TX_PIN        GPIO_PIN_9    // PA9
    #define RS485_TX_PORT       GPIOA
    #define RS485_RX_PIN        GPIO_PIN_10   // PA10
    #define RS485_RX_PORT       GPIOA
    #define RS485_DE_PIN        GPIO_PIN_8    // PA8
    #define RS485_DE_PORT       GPIOA
#endif


/* ================================================================
 *                     ADC 通道规划
 * ================================================================
 *  CH0~CH5  : 电机电流  PA0~PA5   (6路)
 *  CH8~CH9  : NTC温度   PB0~PB1   (2路)
 *  CH10~CH11: 热控电流  PC0~PC1   (2路)
 *  CH12~CH13: 风扇电流  PC2~PC3   (2路)
 * ================================================================ */


/* ================================================================
 *              模块1: 电机控制 (6个电机)
 * ================================================================
 * 引脚分配（霍尔仅输入，无霍尔输出）:
 *   - 电机正转: PB12-PB15, PC10-PC11 (6路)
 *   - 电机反转: PC12-PC15, PA11-PA12 (6路)
 *   - 霍尔输入: PA15, PC4-PC8 (6路)
 *   - 电机电流: PA0-PA5 ADC (6路)
 * ================================================================ */

/* ---------------- 电机1 ---------------- */
#define MOTOR1_FWD_PIN          GPIO_PIN_12
#define MOTOR1_FWD_PORT         GPIOB
#define MOTOR1_FWD_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define MOTOR1_REV_PIN          GPIO_PIN_12
#define MOTOR1_REV_PORT         GPIOC
#define MOTOR1_REV_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR1_HALL_IN_PIN      GPIO_PIN_15
#define MOTOR1_HALL_IN_PORT     GPIOA
#define MOTOR1_HALL_IN_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define MOTOR1_CURRENT_ADC_CHANNEL  ADC_CHANNEL_0   // PA0


/* ---------------- 电机2 ---------------- */
#define MOTOR2_FWD_PIN          GPIO_PIN_13
#define MOTOR2_FWD_PORT         GPIOB
#define MOTOR2_FWD_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define MOTOR2_REV_PIN          GPIO_PIN_13
#define MOTOR2_REV_PORT         GPIOC
#define MOTOR2_REV_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR2_HALL_IN_PIN      GPIO_PIN_4
#define MOTOR2_HALL_IN_PORT     GPIOC
#define MOTOR2_HALL_IN_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR2_CURRENT_ADC_CHANNEL  ADC_CHANNEL_1   // PA1


/* ---------------- 电机3 ---------------- */
#define MOTOR3_FWD_PIN          GPIO_PIN_14
#define MOTOR3_FWD_PORT         GPIOB
#define MOTOR3_FWD_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define MOTOR3_REV_PIN          GPIO_PIN_14
#define MOTOR3_REV_PORT         GPIOC
#define MOTOR3_REV_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR3_HALL_IN_PIN      GPIO_PIN_5
#define MOTOR3_HALL_IN_PORT     GPIOC
#define MOTOR3_HALL_IN_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR3_CURRENT_ADC_CHANNEL  ADC_CHANNEL_2   // PA2


/* ---------------- 电机4 ---------------- */
#define MOTOR4_FWD_PIN          GPIO_PIN_15
#define MOTOR4_FWD_PORT         GPIOB
#define MOTOR4_FWD_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define MOTOR4_REV_PIN          GPIO_PIN_15
#define MOTOR4_REV_PORT         GPIOC
#define MOTOR4_REV_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR4_HALL_IN_PIN      GPIO_PIN_6
#define MOTOR4_HALL_IN_PORT     GPIOC
#define MOTOR4_HALL_IN_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR4_CURRENT_ADC_CHANNEL  ADC_CHANNEL_3   // PA3


/* ---------------- 电机5 ---------------- */
#define MOTOR5_FWD_PIN          GPIO_PIN_10
#define MOTOR5_FWD_PORT         GPIOC
#define MOTOR5_FWD_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR5_REV_PIN          GPIO_PIN_11
#define MOTOR5_REV_PORT         GPIOA
#define MOTOR5_REV_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define MOTOR5_HALL_IN_PIN      GPIO_PIN_7
#define MOTOR5_HALL_IN_PORT     GPIOC
#define MOTOR5_HALL_IN_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR5_CURRENT_ADC_CHANNEL  ADC_CHANNEL_4   // PA4


/* ---------------- 电机6 ---------------- */
#define MOTOR6_FWD_PIN          GPIO_PIN_11
#define MOTOR6_FWD_PORT         GPIOC
#define MOTOR6_FWD_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR6_REV_PIN          GPIO_PIN_12
#define MOTOR6_REV_PORT         GPIOA
#define MOTOR6_REV_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define MOTOR6_HALL_IN_PIN      GPIO_PIN_8
#define MOTOR6_HALL_IN_PORT     GPIOC
#define MOTOR6_HALL_IN_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define MOTOR6_CURRENT_ADC_CHANNEL  ADC_CHANNEL_5   // PA5

/* ============ 霍尔公共使能/供电切换 IO（第7路） ============ */
/* 用 PD0（不使用HSE晶振时可用），如果用HSE就换 PD1 或别的空脚 */
#define HALL_EN_PIN          GPIO_PIN_0     // PD0
#define HALL_EN_PORT         GPIOD
#define HALL_EN_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()

#define HALL_EN_ACTIVE_LEVEL GPIO_PIN_SET  // 高有效；低有效就改 RESET


/* ================================================================
 *              模块2: 通风控制 (2路PWM + 2路ADC)
 * ================================================================ */

/* ---------------- 风扇1 PWM ---------------- */
#define FAN1_PWM_TIMER          TIM3
#define FAN1_PWM_CHANNEL        TIM_CHANNEL_1
#define FAN1_PWM_PIN            GPIO_PIN_6    // PA6 TIM3_CH1
#define FAN1_PWM_PORT           GPIOA
#define FAN1_PWM_CLK_ENABLE()   __HAL_RCC_TIM3_CLK_ENABLE()

#define FAN1_CURRENT_ADC_CHANNEL    ADC_CHANNEL_12  // PC2


/* ---------------- 风扇2 PWM ---------------- */
#define FAN2_PWM_TIMER          TIM3
#define FAN2_PWM_CHANNEL        TIM_CHANNEL_2
#define FAN2_PWM_PIN            GPIO_PIN_7    // PA7 TIM3_CH2
#define FAN2_PWM_PORT           GPIOA
#define FAN2_PWM_CLK_ENABLE()   __HAL_RCC_TIM3_CLK_ENABLE()

#define FAN2_CURRENT_ADC_CHANNEL    ADC_CHANNEL_13  // PC3


/* PWM参数 */
#define FAN_PWM_FREQUENCY       25000U
#define FAN_PWM_RESOLUTION      1000U


/* ================================================================
 *              模块3: 热控制 (8路IO + 4路ADC)
 * ================================================================
 * 热控IO分配:
 *   PB8-PB11 (4路) + PC9 (1路) + PA13-PA14 (2路) + PD2 (1路) = 8路
 * 注：PA13/PA14 为 SWD 脚，除非 IO 不够，否则后续建议挪走
 * ================================================================ */

/* ---------------- 热控输出 IO (8路) ---------------- */
#define HEAT_CTRL1_PIN          GPIO_PIN_8
#define HEAT_CTRL1_PORT         GPIOB
#define HEAT_CTRL1_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define HEAT_CTRL2_PIN          GPIO_PIN_9
#define HEAT_CTRL2_PORT         GPIOB
#define HEAT_CTRL2_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define HEAT_CTRL3_PIN          GPIO_PIN_10
#define HEAT_CTRL3_PORT         GPIOB
#define HEAT_CTRL3_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define HEAT_CTRL4_PIN          GPIO_PIN_11
#define HEAT_CTRL4_PORT         GPIOB
#define HEAT_CTRL4_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define HEAT_CTRL5_PIN          GPIO_PIN_9
#define HEAT_CTRL5_PORT         GPIOC
#define HEAT_CTRL5_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

#define HEAT_CTRL6_PIN          GPIO_PIN_13   // ⚠️ PA13=SWDIO 需禁SWD
#define HEAT_CTRL6_PORT         GPIOA
#define HEAT_CTRL6_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define HEAT_CTRL7_PIN          GPIO_PIN_14   // ⚠️ PA14=SWCLK 需禁SWD
#define HEAT_CTRL7_PORT         GPIOA
#define HEAT_CTRL7_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define HEAT_CTRL8_PIN          GPIO_PIN_2    // 仅在不用外部HSE且硬件引出时可用
#define HEAT_CTRL8_PORT         GPIOD
#define HEAT_CTRL8_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()


/* ---------------- NTC温度 ADC (2路) ---------------- */
#define NTC1_ADC_CHANNEL        ADC_CHANNEL_8   // PB0
#define NTC1_ADC_PIN            GPIO_PIN_0
#define NTC1_ADC_PORT           GPIOB

#define NTC2_ADC_CHANNEL        ADC_CHANNEL_9   // PB1
#define NTC2_ADC_PIN            GPIO_PIN_1
#define NTC2_ADC_PORT           GPIOB


/* ---------------- 热控供电电流 ADC (2路) ---------------- */
#define HEAT_CURRENT1_ADC_CHANNEL   ADC_CHANNEL_10  // PC0
#define HEAT_CURRENT1_ADC_PIN       GPIO_PIN_0
#define HEAT_CURRENT1_ADC_PORT      GPIOC

#define HEAT_CURRENT2_ADC_CHANNEL   ADC_CHANNEL_11  // PC1
#define HEAT_CURRENT2_ADC_PIN       GPIO_PIN_1
#define HEAT_CURRENT2_ADC_PORT      GPIOC


/* ================================================================
 *                      系统配置
 * ================================================================ */
#define SYSTEM_CLOCK_FREQ       72000000U
#define ADC_VREF                3.3f
#define ADC_RESOLUTION          4096U


/* ================================================================
 *             完整引脚分配表（当前假设，无霍尔输出）
 * ================================================================
 * PA0-PA5:   电机电流ADC (6路) ✓
 * PA6-PA7:   风扇PWM (2路) ✓
 * PA8-PA10:  RS485通信 (3路) ✓
 * PA11-PA12: 电机5/6反转 (2路) ✓
 * PA13-PA14: 热控6-7 (2路) ⚠️ 需禁SWD
 * PA15:      电机1霍尔输入 (1路) ✓
 *
 * PB0-PB1:   NTC温度ADC (2路) ✓
 * PB8-PB11:  热控1-4 (4路) ✓
 * PB12-PB15: 电机1-4正转 (4路) ✓
 * PB2-PB7:   预留GPIO（原霍尔输出位）✓
 *
 * PC0-PC1:   热控电流ADC (2路) ✓
 * PC2-PC3:   风扇电流ADC (2路) ✓
 * PC4-PC8:   电机2-6霍尔输入 (5路) ✓
 * PC9:       热控5 (1路) ✓
 * PC10-PC11: 电机5-6正转 (2路) ✓
 * PC12-PC15: 电机1-4反转 (4路) ✓
 *
 * PD2:       热控8 (1路) ✓（仅在不用外部HSE且硬件引出时）
 *
 * ⚠️ 注意事项:
 *   1. PB3/PB4 若后续使用，需在CubeMX中禁用JTAG，保留SWD
 *   2. PA13/PA14 若用作GPIO，需要禁SWD（下载后无法再调试）
 *   3. PD0/PD1 若使用外部HSE晶振则不可用作GPIO
 *   4. 如使用CAN，需remap到PB8/PB9（会占用热控1-2）
 * ================================================================ */

#endif /* __HARDWARE_CONFIG_H__ */
