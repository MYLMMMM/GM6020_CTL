/**
 * 使用过程中发现的问题：
 * PID参数不便于修改 ： 改为外置结构体传入
 * 输入输出参数不一致时无法使用 
 */
#pragma once
#include <type_traits>

struct PID_para
{
    double Kp;
    double Ki;
    double Kd;    
};

template <typename T>
class PIDController {
    static_assert(std::is_arithmetic<T>::value, "T must be a numeric type");

private:
    // 输入/输出
    T* setpoint;    //输入
    T* current_value;  //当前值
    T* output;         // 输出

    // PID 参数
    double Kp = 0;
    double Ki = 0;
    double Kd = 0;

    // 控制频率 (Hz)
    T frequency = 1000;   

    // 内部状态
    T previous_error = 0;
    T integral = 0;

    // 积分限幅
    T integral_min = 0;
    T integral_max = 0;
    bool use_integral_limit = false;

public:

    // 构造函数
    PIDController(T* setpoint,T* current_value,T* output)
        : setpoint(setpoint), current_value(current_value), output(output) {}

    // 参数访问
    void setKp(double value) { Kp = value; }
    void setKi(double value) { Ki = value; }
    void setKd(double value) { Kd = value; }

    double getKp() const { return Kp; }
    double getKi() const { return Ki; }
    double getKd() const { return Kd; }

    // 频率访问
    void setFrequency(T hz) { 
        if (hz > 0) frequency = hz; 
    }
    T getFrequency() const { return frequency; }
    T getDt() const { return 1.0 / frequency; }

    // 状态访问
    void reset() {
        previous_error = 0;
        integral = 0;
    }
    T getError() const { return setpoint - current_value; }
    T getIntegral() const { return integral; }

    // 积分限幅
    void setIntegralLimit(T min_val, T max_val) {
        integral_min = min_val;
        integral_max = max_val;
        use_integral_limit = true;
    }

    // PID计算 
    void trriger(bool ifcaculate) {
        if(ifcaculate == false)
        {
            *output = *setpoint;
        }
        else
        {
            T dt = 1.0 / frequency;
            T error = setpoint - current_value;

            // 积分
            integral += error * dt;
            if (use_integral_limit) {
                if (integral > integral_max) integral = integral_max;
                if (integral < integral_min) integral = integral_min;
            }

            // 微分
            T derivative = (error - previous_error) / dt;

            // 输出计算
            output = Kp * error + Ki * integral + Kd * derivative;

            // 更新状态
            previous_error = error;
        }
    }
};
