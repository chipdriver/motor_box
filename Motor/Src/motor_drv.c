#include "motor_drv.h"
#include "stm32f103xe.h"

/******************************************************************************
 *                              私有类型定义
 ******************************************************************************/

/**
 * @brief 电机GPIO引脚配置结构体
 * @note  封装单个电机的正转/反转控制引脚信息
 *        便于通过motor_map映射表管理多个电机
 */
typedef struct 
{
    GPIO_TypeDef *fwd_port;   ///< 正转控制引脚的GPIO端口
    uint16_t      fwd_pin;    ///< 正转控制引脚编号
    GPIO_TypeDef *rev_port;   ///< 反转控制引脚的GPIO端口
    uint16_t      rev_pin;    ///< 反转控制引脚编号
} motor_gpio_t;

/******************************************************************************
 *                              私有变量定义
 ******************************************************************************/

/**
 * @brief 电机GPIO映射表
 * @note  存储所有电机的引脚配置，通过motor_id_t索引访问
 */
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
 * @brief 霍尔传感器脉冲计数数组
 * @note  每个电机对应一个计数器，在中断中累加
 */
static volatile uint32_t s_hall_cnt[MOTOR_NUM] = {0};

/******************************************************************************
 *                           电机方向控制函数
 ******************************************************************************/

/**
 * @brief  电机驱动初始化
 * @note   将所有电机设置为停止状态（正反转引脚均拉低）
 * @retval None
 */
void motor_drv_init(void)
{
    for (int i = 0; i < MOTOR_NUM; i++)
    {
        HAL_GPIO_WritePin(motor_map[i].fwd_port, motor_map[i].fwd_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor_map[i].rev_port, motor_map[i].rev_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief  设置电机运行方向
 * @param  id: 电机ID (MOTOR1 ~ MOTOR6)
 * @param  dir: 运行方向 (MOTOR_DIR_STOP/FWD/REV)
 * @note   包含互斥保护，确保正转和反转不会同时开启
 * @retval None
 */
void motor_drv_set_dir(motor_id_t id, motor_dir_t dir)
{
    if (id >= MOTOR_NUM) 
    {
        return;
    }

    switch (dir) 
    {
        case MOTOR_DIR_STOP:
            HAL_GPIO_WritePin(motor_map[id].fwd_port, motor_map[id].fwd_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor_map[id].rev_port, motor_map[id].rev_pin, GPIO_PIN_RESET);
            break;

        case MOTOR_DIR_FWD:
            HAL_GPIO_WritePin(motor_map[id].rev_port, motor_map[id].rev_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor_map[id].fwd_port, motor_map[id].fwd_pin, GPIO_PIN_SET);
            break;

        case MOTOR_DIR_REV:
            HAL_GPIO_WritePin(motor_map[id].fwd_port, motor_map[id].fwd_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor_map[id].rev_port, motor_map[id].rev_pin, GPIO_PIN_SET);
            break;

        default:
            break;
    }
}

/**
 * @brief  电机正转
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_forward(motor_id_t id)
{
    motor_drv_set_dir(id, MOTOR_DIR_FWD);
}

/**
 * @brief  电机反转
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_reverse(motor_id_t id)
{
    motor_drv_set_dir(id, MOTOR_DIR_REV);
}

/**
 * @brief  电机停止
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_stop(motor_id_t id)
{
    motor_drv_set_dir(id, MOTOR_DIR_STOP);
}

/******************************************************************************
 *                          霍尔传感器计数函数
 ******************************************************************************/

/**
 * @brief  霍尔传感器计数器初始化
 * @note   将所有电机的霍尔计数清零
 * @retval None
 */
void motor_drv_hall_init(void)
{
    for (int i = 0; i < MOTOR_NUM; i++)
    {
        s_hall_cnt[i] = 0;
    }
}

/**
 * @brief  获取指定电机的霍尔计数值
 * @param  id: 电机ID
 * @retval 该电机的霍尔脉冲计数值，越界返回0
 */
uint32_t motor_drv_hall_get_count(motor_id_t id)
{
    if (id >= MOTOR_NUM) 
    {
        return 0;
    }
    return s_hall_cnt[id];
}

/**
 * @brief  清零指定电机的霍尔计数值
 * @param  id: 电机ID
 * @retval None
 */
void motor_drv_hall_clear(motor_id_t id)
{
    if (id >= MOTOR_NUM) 
    {
        return;
    }
    s_hall_cnt[id] = 0;
}

/**
 * @brief  清零所有电机的霍尔计数值
 * @retval None
 */
void motor_drv_hall_clear_all(void)
{
    for (int i = 0; i < MOTOR_NUM; i++)
    {
        s_hall_cnt[i] = 0;
    }
}

/*===============================================================
 * 霍尔公共使能/供电切换（第7路IO）
 * @note 这根线是“霍尔模块的总开关”（供电/使能/选通），不参与计数中断。
 *       上层只需要调用 enable/disable，不关心具体引脚；后续换脚只改宏即可。
 *==============================================================*/

static uint8_t s_hall_enabled = 0;  // 记录当前霍尔是否已经使能


/**
 * @brief 打开霍尔模块公共使能（HALL_EN）输出。
 */
void motor_drv_hall_enable(void)
{
    HALL_EN_CLK_ENABLE();  // 打开 HALL_EN 所在 GPIO 口的时钟

    HAL_GPIO_WritePin(HALL_EN_PORT, HALL_EN_PIN, HALL_EN_ACTIVE_LEVEL); //把 HALL_EN 引脚拉到“有效电平

    s_hall_enabled = 1; //更新状态：已打开
}

/**
 * @brief 关闭霍尔模块公共使能（HALL_EN）输出。
 */
void motor_drv_hall_disable(void)
{
    HALL_EN_CLK_ENABLE();  // 打开 HALL_EN 所在 GPIO 口的时钟

    HAL_GPIO_WritePin(HALL_EN_PORT, HALL_EN_PIN, (HALL_EN_ACTIVE_LEVEL == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET); //把 HALL_EN 引脚拉到“无效电平

    s_hall_enabled = 0; //更新状态：已关闭
}

/**
 * @brief 获取当前霍尔模块公共使能状态。
 * @return 1=已使能(开)，0=未使能(关)。
 */
uint8_t motor_drv_hall_is_enabled(void)
{
    return s_hall_enabled;  // 返回当前是否使能
}


/******************************************************************************
 *                            中断回调函数
 ******************************************************************************/

/**
 * @brief  GPIO外部中断回调函数
 * @param  GPIO_Pin: 触发中断的GPIO引脚编号
 * @note   霍尔传感器上升沿触发，对应电机计数器自增
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MOTOR1_HALL_IN_PIN)
    {
        s_hall_cnt[MOTOR1]++;
    }
    else if (GPIO_Pin == MOTOR2_HALL_IN_PIN)
    {
        s_hall_cnt[MOTOR2]++;
    }
    else if (GPIO_Pin == MOTOR3_HALL_IN_PIN)
    {
        s_hall_cnt[MOTOR3]++;
    }
    else if (GPIO_Pin == MOTOR4_HALL_IN_PIN)
    {
        s_hall_cnt[MOTOR4]++;
    }
    else if (GPIO_Pin == MOTOR5_HALL_IN_PIN)
    {
        s_hall_cnt[MOTOR5]++;
    }
    else if (GPIO_Pin == MOTOR6_HALL_IN_PIN)
    {
        s_hall_cnt[MOTOR6]++;
    }
}

/******************************************************************************
 *                            电机采集电流函数
 ******************************************************************************/

extern ADC_HandleTypeDef hadc1;  // 声明ADC句柄

static uint16_t s_motor_adc_buf[MOTOR_NUM] = { 0 };//DMA缓冲，存放MOTOR_NUM路电机电流ADC原始值

#define R_SHUNT_OHM 0.01f // 分流电阻阻值，单位欧姆
#define AMP_GAIN    20.0f //电流采样放大倍数，示例：20倍
#define ADC_VREF    3.3f  // ADC参考电压，单位伏特
#define ADC_FULL_SCALE 4096.0f //ADC满量程值（12位ADC）

/**
 * @brief  启动电机电流采样（ADC1 扫描 + DMA 循环模式）
 * @param  None
 * @note   使用 ADC1 多通道扫描，将 MOTOR_NUM 路电流采样结果通过 DMA
 *         连续写入 s_motor_adc_buf[]，DMA 为 Circular 模式保证缓冲区实时更新。
 *         为什么用Circular模式？因为 Circular 模式能让 DMA 自动循环把 ADC 采样结果持续更新到缓冲区里，软件不用反复启动采样就能一直拿到最新电流值。
 * @retval None
 * 
 */
void motor_drv_current_init(void)
{
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)s_motor_adc_buf,MOTOR_NUM);
}

/**
 * @brief  读取指定电机的原始 ADC 采样值
 * @param  id: 电机编号/ID（对应 ADC 扫描通道顺序）
 * @note   返回值为 ADC 原始码，范围通常为 0~4095（12bit）。
 *         若 id 越界则返回 0。
 * @retval 指定电机的 ADC 原始值(uint16_t)
 */
uint16_t motor_drv_get_current_raw(motor_id_t id)
{
    if (id >= MOTOR_NUM) return 0;
    return s_motor_adc_buf[id];
}

/**
 * @brief  读取指定电机的实际电流值
 * @param  id: 电机编号/ID（对应 ADC 扫描通道顺序）
 * @note   换算公式：
 *           1) v_sense = (adc / ADC_FULL_SCALE) * ADC_VREF
 *           2) I = v_sense / (R_SHUNT_OHM * AMP_GAIN)
 *         其中 R_SHUNT_OHM、AMP_GAIN 为当前默认示例值，需按实际硬件修正。
 *         若 id 越界则返回 0.0f。
 * @retval 指定电机电流值，单位 A(float)
 */
float motor_drv_get_current_A(motor_id_t id)
{
    if (id >= MOTOR_NUM) return 0.0f;

    float adc = (float)s_motor_adc_buf[id];
    float v_sense = (adc / ADC_FULL_SCALE) * ADC_VREF;   // ADC→电压
    float current = v_sense / (R_SHUNT_OHM * AMP_GAIN);  // 电压→电流

    return current;
}


