#ifndef __MOTOR_DRV_H__
#define __MOTOR_DRV_H__

#include "stm32f1xx_hal.h"
#include "hardware_config.h"   // 你的引脚宏在这里

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {          // 开始定义一个枚举(enum)，并准备起一个新类型名
    MOTOR_DIR_STOP = 0, // 枚举值1：停止，显式指定为0
    MOTOR_DIR_FWD,      // 枚举值2：正转，默认就是1
    MOTOR_DIR_REV       // 枚举值3：反转，默认就是2
} motor_dir_t;          // 给这个枚举类型取名 motor_dir_t，以后当“方向类型”用


/* 初始化：把所有电机默认停掉 */
void motor_drv_init(void);

/* 设置方向：正转/反转/停止 */
void motor_drv_set_dir(motor_id_t id, motor_dir_t dir);

/* 方便上层用的快捷函数 */
void motor_drv_forward(motor_id_t id);
void motor_drv_reverse(motor_id_t id);
void motor_drv_stop(motor_id_t id);

/* 霍尔计数（6路，每电机1路） */
void motor_drv_hall_init(void);
uint32_t motor_drv_hall_get_count(motor_id_t id);
void motor_drv_hall_clear(motor_id_t id);
void motor_drv_hall_clear_all(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_DRV_H__ */