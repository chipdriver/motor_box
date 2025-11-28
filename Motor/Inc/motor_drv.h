#ifndef __MOTOR_DRV_H__
#define __MOTOR_DRV_H__

#include "stm32f1xx_hal.h"
#include "hardware_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *                              类型定义
 ******************************************************************************/

/**
 * @brief 电机运行方向枚举
 * @note  用于控制电机的运行状态
 */
typedef enum 
{
    MOTOR_DIR_STOP = 0,  ///< 停止状态
    MOTOR_DIR_FWD,       ///< 正转
    MOTOR_DIR_REV        ///< 反转
} motor_dir_t;

/******************************************************************************
 *                           电机控制函数声明
 ******************************************************************************/

/**
 * @brief  电机驱动初始化
 * @note   将所有电机设置为停止状态
 * @retval None
 */
void motor_drv_init(void);

/**
 * @brief  设置电机运行方向
 * @param  id: 电机ID
 * @param  dir: 运行方向 (MOTOR_DIR_STOP/FWD/REV)
 * @retval None
 */
void motor_drv_set_dir(motor_id_t id, motor_dir_t dir);

/**
 * @brief  电机正转
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_forward(motor_id_t id);

/**
 * @brief  电机反转
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_reverse(motor_id_t id);

/**
 * @brief  电机停止
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_stop(motor_id_t id);

/******************************************************************************
 *                          霍尔传感器函数声明
 ******************************************************************************/

/**
 * @brief  霍尔传感器计数器初始化
 * @note   将所有电机的霍尔计数清零
 * @retval None
 */
void motor_drv_hall_init(void);

/**
 * @brief  获取指定电机的霍尔计数值
 * @param  id: 电机ID
 * @retval 霍尔脉冲计数值
 */
uint32_t motor_drv_hall_get_count(motor_id_t id);

/**
 * @brief  清零指定电机的霍尔计数值
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_hall_clear(motor_id_t id);

/**
 * @brief  清零所有电机的霍尔计数值
 * @retval None
 */
void motor_drv_hall_clear_all(void);

void motor_drv_hall_enable(void);          // 打开霍尔模块
void motor_drv_hall_disable(void);         // 关闭霍尔模块
uint8_t motor_drv_hall_is_enabled(void);   // 查询当前开关状态


/******************************************************************************
 *                          电机采集电流函数声明
 ******************************************************************************/
void motor_drv_current_init(void);            // 电机电流采样初始化（ADC/通道/校准等）
uint16_t motor_drv_get_current_raw(motor_id_t id);  // 获取指定电机电流ADC原始值
float motor_drv_get_current_A(motor_id_t id);       // 获取指定电机电流值（单位A）

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_DRV_H__ */