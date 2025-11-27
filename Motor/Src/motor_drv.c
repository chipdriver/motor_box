#include "motor_drv.h"
#include "stm32f103xe.h"

/**
 * @note 用结构体把一台电机的正/反转引脚(端口+pin)打包起来，便于做 motor_map 映射表。
 *       以后上层只传电机ID调用 forward/reverse/stop，硬件换脚时只改宏/表，驱动和业务不用动。
 */
typedef struct 
{
    GPIO_TypeDef *fwd_port;   // 正转控制引脚所在的GPIO端口（比如 GPIOA / GPIOB）
    uint16_t      fwd_pin;    // 正转控制引脚的pin号（比如 GPIO_PIN_12）
    GPIO_TypeDef *rev_port;   // 反转控制引脚所在的GPIO端口
    uint16_t      rev_pin;    // 反转控制引脚的pin号
} motor_gpio_t;              // 结构体类型名：表示“一台电机的正反转引脚配置”

static const motor_gpio_t motor_map[MOTOR_NUM] = 
{
    {MOTOR1_FWD_PORT, MOTOR1_FWD_PIN, MOTOR1_REV_PORT, MOTOR1_REV_PIN},
    {MOTOR2_FWD_PORT, MOTOR2_FWD_PIN, MOTOR2_REV_PORT, MOTOR2_REV_PIN},
    {MOTOR3_FWD_PORT, MOTOR3_FWD_PIN, MOTOR3_REV_PORT, MOTOR3_REV_PIN},
    {MOTOR4_FWD_PORT, MOTOR4_FWD_PIN, MOTOR4_REV_PORT, MOTOR4_REV_PIN},
    {MOTOR5_FWD_PORT, MOTOR5_FWD_PIN, MOTOR5_REV_PORT, MOTOR5_REV_PIN},
    {MOTOR6_FWD_PORT, MOTOR6_FWD_PIN, MOTOR6_REV_PORT, MOTOR6_REV_PIN},
};

/**
 * @brief 初始化： 所有电机停止
 * @note 控制电机是两根线（控制线）
 *       一根 FWD：拉高=要求正转
         一根 REV：拉高=要求反转
         停止：两根都拉低（常见做法）
 */
void motor_drv_init(void)
{
    for(int i= 0; i < MOTOR_NUM; i++)
    {
        HAL_GPIO_WritePin(motor_map[i].fwd_port, motor_map[i].fwd_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor_map[i].rev_port, motor_map[i].rev_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief 设置方向：带互斥保护（绝不允许正反同时开）
 */
void motor_drv_set_dir(motor_id_t id, motor_dir_t dir)
{
    if ( id >= MOTOR_NUM ) return;  // 越界保护:如果传进来的电机ID不存在，就直接退出。

    switch(dir) // 根据 dir 的不同取值，走不同分支来控制电机。
    {
        case MOTOR_DIR_STOP:
            HAL_GPIO_WritePin(motor_map[id].fwd_port, motor_map[id].fwd_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor_map[id].rev_port, motor_map[id].rev_pin, GPIO_PIN_RESET);
            break;
        case MOTOR_DIR_FWD: // 正转：先保证反转关掉，再打开正转，避免正反同时开。
            HAL_GPIO_WritePin(motor_map[id].rev_port, motor_map[id].rev_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor_map[id].fwd_port, motor_map[id].fwd_pin, GPIO_PIN_SET);
            break;
        case MOTOR_DIR_REV: // 反转：先保证正转关掉，再打开反转，避免正反同时开。
            HAL_GPIO_WritePin(motor_map[id].fwd_port, motor_map[id].fwd_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor_map[id].rev_port, motor_map[id].rev_pin, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

/**
 * @brief 正转
 */
void motor_drv_forward(motor_id_t id)
{
    motor_drv_set_dir(id, MOTOR_DIR_FWD);
}

/**
 * @brief 反转
 */
void motor_drv_reverse(motor_id_t id)
{
    motor_drv_set_dir(id, MOTOR_DIR_REV);
}

/**
 * @brief 停止
 */
void motor_drv_stop(motor_id_t id)
{
    motor_drv_set_dir(id, MOTOR_DIR_STOP);
}

/* ================= 霍尔计数（6路） ================= */
static volatile uint32_t s_hall_cnt[MOTOR_NUM] = { 0 }; //6个电机各自的脉冲计数

/**
 * @brief 霍尔初始化
 */
void motor_drv_hall_init(void)
{
    for(int i = 0; i < MOTOR_NUM ; i++)
    {
        s_hall_cnt[i] = 0;
    }
}

/**
 * @brief 获取指定电机的霍尔计数值
 */
uint32_t motor_drv_hall_get_count(motor_id_t id)
{
    if (id >= MOTOR_NUM) return 0; //越界保护
    return s_hall_cnt[id];  //返回指定电机的计数
}

/**
 * @brief 清零指定电机的霍尔计数值
 */
void motor_drv_hall_clear(motor_id_t id)
{
    if (id >= MOTOR_NUM) return; //越界保护
    s_hall_cnt[id] = 0;  //清零指定电机的计数
}

/**
 * @brief 全部电机的霍尔计数清零
 */
void motor_drv_hall_clear_all(void)
{
    for (int i = 0; i < MOTOR_NUM; i++)
    {
        s_hall_cnt[i] = 0;   // 全部清零
    }
} 

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MOTOR1_HALL_IN_PIN) s_hall_cnt[MOTOR1]++;
    else if (GPIO_Pin == MOTOR2_HALL_IN_PIN) s_hall_cnt[MOTOR2]++;
    else if (GPIO_Pin == MOTOR3_HALL_IN_PIN) s_hall_cnt[MOTOR3]++;
    else if (GPIO_Pin == MOTOR4_HALL_IN_PIN) s_hall_cnt[MOTOR4]++;
    else if (GPIO_Pin == MOTOR5_HALL_IN_PIN) s_hall_cnt[MOTOR5]++;
    else if (GPIO_Pin == MOTOR6_HALL_IN_PIN) s_hall_cnt[MOTOR6]++;
}