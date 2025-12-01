#ifndef THERMISTOR_TEMPERATURE_DRIVER_H
#define THERMISTOR_TEMPERATURE_DRIVER_H

#include <stdint.h>
#include <math.h>
/**
 * @brief 把模数转换器(ADC)采样值转换成摄氏温度
 * @param analog_to_digital_converter_value  12位ADC采样原始值（0~4095）
 * @return 摄氏温度
 *         如果采样值异常（接近0或接近满量程），返回极端值给上层做故障判断
 */
float convert_analog_to_digital_converter_value_to_temperature_celsius(uint16_t analog_to_digital_converter_value);

#endif // THERMISTOR_TEMPERATURE_DRIVER_H