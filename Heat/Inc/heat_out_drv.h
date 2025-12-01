#ifndef HEAT_OUT_DRV_H
#define HEAT_OUT_DRV_H

#include "stm32f103xe.h"
#include "hardware_config.h"
/******************************************************************************
 *                              宏定义
 ******************************************************************************/
/**
 * @brief 热控输出通道编号（共8路）
 * 这些通道对应 hardware_config.h 里 HEAT_CTRL1~8 的那8个13.5V高边控制口。
 *
 * 注意：
 * 1) 这里只是MCU侧的控制脚编号
 * 2) 这些脚输出的是 3.3V 高/低电平，用来控制外部高边开关（MOS/驱动芯片）
 */
typedef enum{
    HEAT_OUT1 = 0,
    HEAT_OUT2,
    HEAT_OUT3,
    HEAT_OUT4,
    HEAT_OUT5,
    HEAT_OUT6,
    HEAT_OUT7,
    HEAT_OUT8,
    HEAT_OUT_NUM
}heat_out_ch_t;
/******************************************************************************
 *                              函数声明
 ******************************************************************************/
void heat_out_init_register(void);
void heat_out_set(heat_out_ch_t ch, uint8_t on);

#endif // HEAT_OUT_DRV_H