#include "heat_out_drv.h"
#include "stm32f103xe.h"

/******************************************************************************
 *                              函数声明
 ******************************************************************************/
static void gpio_config_output_pp_50m(GPIO_TypeDef* port, uint16_t pin);
static void afio_swj_disable_once(void);
/***************************************************************
 * 关键配置区
 ***************************************************************/

/**
 * @brief 热控输出有效电平定义
 * 1 表示：MCU 输出高电平=打开外部高边开关（加热通电）
 * 0 表示：MCU 输出低电平=关闭外部高边开关（断电）
 */
#define HEAT_OUT_ACTIVE_HIGH  1


/**
 * @brief 每一路热控输出对应的GPIO端口/引脚映射表
 *
 * 映射来源于 hardware_config.h：
 * HEAT_CTRL1: PB8
 * HEAT_CTRL2: PB9
 * HEAT_CTRL3: PB10
 * HEAT_CTRL4: PB11
 * HEAT_CTRL5: PC9
 * HEAT_CTRL6: PA13  (SWDIO)
 * HEAT_CTRL7: PA14  (SWCLK)
 * HEAT_CTRL8: PD2
 */

typedef struct {
    GPIO_TypeDef* port;    //GPIOA~GPIOD
    uint16_t        pin;     //GPIO_Pin_0~GPIO_Pin_15
}heat_out_map_t;

static const heat_out_map_t s_heat_map[HEAT_OUT_NUM] = {
    {HEAT_CTRL1_PORT, HEAT_CTRL1_PIN}, // HEAT_OUT1
    {HEAT_CTRL2_PORT, HEAT_CTRL2_PIN}, // HEAT_OUT2
    {HEAT_CTRL3_PORT, HEAT_CTRL3_PIN}, // HEAT_OUT3
    {HEAT_CTRL4_PORT, HEAT_CTRL4_PIN}, // HEAT_OUT4
    {HEAT_CTRL5_PORT, HEAT_CTRL5_PIN}, // HEAT_OUT5
    {HEAT_CTRL6_PORT, HEAT_CTRL6_PIN}, // HEAT_OUT6
    {HEAT_CTRL7_PORT, HEAT_CTRL7_PIN}, // HEAT_OUT7
    {HEAT_CTRL8_PORT, HEAT_CTRL8_PIN}  // HEAT_OUT8
};

/***************************************************************
 * 内部工具函数：把单个GPIO配置为 推挽输出 50MHz（寄存器方式）
 ***************************************************************/

/**
 * STM32F103 GPIO配置说明：
 * - 每个GPIO口有两组配置寄存器：CRL(配置0~7号脚) 和 CRH(配置8~15号脚)
 * - 每个引脚对应4bit配置：
 *   MODE[1:0] + CNF[1:0]
 *
 * 推挽输出50MHz：
 *   MODE = 11 (50MHz输出)
 *   CNF  = 00 (推挽)
 *   => 4bit = 0b0011 = 0x3
*/
static void gpio_config_output_pp_50m(GPIO_TypeDef* port, uint16_t pin)
{
    //1）打开AFIO时钟
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    //2）打开对应GPIO口时钟
    if(port == GPIOA) RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    if(port == GPIOB) RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    if(port == GPIOC) RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    if(port == GPIOD) RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;

    // 3) 计算 pin 对应的编号（0~15）
    uint8_t pin_num = 0;
    uint16_t t = pin;
    while((t >>= 1) != 0) pin_num++;

    // 4) 选择CRL还是CRH
    volatile uint32_t *cr = (pin_num < 8) ? &port->CRL : &port->CRH;

    // 5）找到该引脚4bit字段的偏移
    uint8_t shift = (pin_num % 8) * 4;

    //6）先清除原配置
    *cr &= ~(0xF << shift);

    //7）设置为 推挽输出 50MHz
    *cr |= (0x03 << shift);

}

/**
 * @brief 关闭SWJ接口（JTAG+SWD），只保留SWD
 * 注意：调用此函数会关闭PA13/PA14引脚的SWD功能 
 * @note 调用时机：只需调用一次，建议在系统初始化时调用
 */
static void afio_swj_disable_once(void)
{
    //1）打开AFIO时钟（IO初始化时已经做了）

    //2）关闭JTAG，保留SWD
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
}

/***************************************************************
 * 对外接口：设置某一路热控输出开/关
 ***************************************************************/
void heat_out_set(heat_out_ch_t ch, uint8_t on)
{
      /**
     * STM32F1 GPIO置位/复位寄存器：
     * - BSRR：写1到低16位 => 置位（输出高）
     * - BRR ：写1到对应位   => 复位（输出低）
     *
     * 这样写的好处是原子操作，不会影响同口其他位。
     */
    GPIO_TypeDef *port = s_heat_map[ch].port;
    uint16_t pin = s_heat_map[ch].pin;
    #if HEAT_OUT_ACTIVE_HIGH
        // 高电平有效：on=1 输出高，on=0 输出低
        if(on) port->BSRR = pin;   // 置高
        else   port->BRR  = pin;   // 置低
    #else
        // 低电平有效：on=1 输出低，on=0 输出高
        if(on) port->BRR  = pin;   // 置低
        else   port->BSRR = pin;   // 置高
    #endif

}
/***************************************************************
 * 对外接口：初始化8路热控输出
 ***************************************************************/
void heat_out_init_register(void)
{
     /**
     * ⚠⚠⚠ PA13/PA14 释放说明 ⚠⚠⚠
     *
     * PA13 = SWDIO, PA14 = SWCLK
     * MCU上电后默认用于 SWD/JTAG 调试接口。
     * 如果要当普通GPIO使用，必须禁用 SWJ（JTAG+SWD）。
     *
     * 禁用后：
     * - 你将无法再用 STLink/SWD 调试！
     * - 所以开发阶段建议先不启用这句，
     *   或者临时不用 HEAT6/HEAT7。
     *
     * 量产固件/已不需要SWD调试时再打开。
     */

    afio_swj_disable_once();

    //逐路配置输出 & 默认关闭
    for(int i = 0; i<HEAT_OUT_NUM; i++)
    {
        gpio_config_output_pp_50m(s_heat_map[i].port, s_heat_map[i].pin);
        
        //默认关闭，防止上电误加热
        heat_out_set((heat_out_ch_t)i, 0);
    }
     
}