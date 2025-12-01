#include "thermistor_temperature_driver.h"

/******************************************************************
 * 模数转换器(ADC)参数（STM32F103为12位ADC）
 ******************************************************************/

 /*ADC 最大值：12位对应0~4095*/
 #define ANALOG_TO_DIGITAL_CONVERTER_MAX_VALUE         4095.0f
/*ADC 参考电压：一般为3.3V*/
#define ANALOG_TO_DIGITAL_CONVERTER_REFERENCE_VOLTAGE_VOLTS 3.3f

/******************************************************************
 * 热敏电阻（负温度系数热敏电阻）参数
 * 这些值与硬件真实参数对应
 ******************************************************************/

/* 25℃时热敏电阻阻值（常见10k） */
#define THERMISTOR_RESISTANCE_AT_25_CELSIUS_OHMS               10000.0f
/* 热敏电阻Beta系数（常见3950） */
#define THERMISTOR_BETA_COEFFICIENT                            3950.0f
/* 参考温度25℃对应的开尔文温度 */
#define THERMISTOR_REFERENCE_TEMPERATURE_KELVIN                298.15f
/* 分压用的固定上拉电阻阻值（常见10k） */
#define PULLUP_RESISTOR_OHMS                                  10000.0f

/******************************************************************
 * 分压结构选择
 *
 * 1：3.3V -> 固定上拉电阻 ->（采样节点）-> 热敏电阻 -> 地
 * 0：3.3V -> 热敏电阻 ->（采样节点）-> 固定上拉电阻 -> 地
 *
 ******************************************************************/
#define THERMISTOR_CONNECTED_TO_GROUND_AT_BOTTOM                  1

float convert_analog_to_digital_converter_value_to_temperature_celsius(uint16_t analog_to_digital_converter_value)
{
    /*1）边界保护：避免出现除0，负数取对数等数学错误*/
    if(analog_to_digital_converter_value <= 1)    return -273.15f; /* 采样接近0，可能是热敏电阻短路/异常 */
    if(analog_to_digital_converter_value >= (ANALOG_TO_DIGITAL_CONVERTER_MAX_VALUE - 1)) return 999.0f; /* 采样接近满量程，可能是热敏电阻断路/异常 */

    /*2）ADC原始值 -> 采样节点电压*/
    float sample_node_voltage_volts = (analog_to_digital_converter_value / ANALOG_TO_DIGITAL_CONVERTER_MAX_VALUE) * ANALOG_TO_DIGITAL_CONVERTER_REFERENCE_VOLTAGE_VOLTS;

    /*3）采样节点电压 -> 热敏电阻阻值 根据分压电路公式反推热敏电阻阻值*/
    float thermistor_resistance_ohms = 0.0f;

#if THERMISTOR_CONNECTED_TO_GROUND_AT_BOTTOM
    /*
     * 情况A：
     * 3.3V -- 固定上拉电阻 -- 采样点 -- 热敏电阻 -- GND
     *
     * 分压关系：
     * 采样点电压 = 参考电压 * R_热敏 / (R_上拉 + R_热敏)
     *
     * 变形得到：
     * R_热敏 = R_上拉 * 采样点电压 / (参考电压 - 采样点电压)
     */
     thermistor_resistance_ohms =
        PULLUP_RESISTOR_OHMS * sample_node_voltage_volts
        / (ANALOG_TO_DIGITAL_CONVERTER_REFERENCE_VOLTAGE_VOLTS - sample_node_voltage_volts);
#else
    /*
     * 情况B：
     * 3.3V -- 热敏电阻 -- 采样点 -- 固定上拉电阻 -- GND
     *
     * 分压关系：
     * 采样点电压 = 参考电压 * R_上拉 / (R_上拉 + R_热敏)
     *
     * 变形得到：
     * R_热敏 = R_上拉 * (参考电压 - 采样点电压) / 采样点电压
     */
    thermistor_resistance_ohms =
        PULLUP_RESISTOR_OHMS
        * (ANALOG_TO_DIGITAL_CONVERTER_REFERENCE_VOLTAGE_VOLTS - sample_node_voltage_volts)
        / sample_node_voltage_volts;

#endif
    /**************************************************************
     * 4) 热敏电阻阻值 -> 温度（Beta公式）
     *
     * 1/T = 1/T0 + (1/B) * ln(R/R0)
     *
     * T  ：当前温度（开尔文）
     * T0 ：参考温度（25℃=298.15K）
     * B  ：Beta系数
     * R  ：当前热敏电阻阻值
     * R0 ：25℃时热敏电阻阻值
     **************************************************************/
    float inverse_temperature_kelvin =
        (1.0f / THERMISTOR_REFERENCE_TEMPERATURE_KELVIN)
        + (1.0f / THERMISTOR_BETA_COEFFICIENT)
          * logf(thermistor_resistance_ohms / THERMISTOR_RESISTANCE_AT_25_CELSIUS_OHMS);

    float temperature_kelvin = 1.0f / inverse_temperature_kelvin;
    /*5) 开尔文温度 -> 摄氏温度*/
    return temperature_kelvin - 273.15f;
}