#include <cstdint>
#include <stdexcept> 
#include <iostream>
#include <linux/can.h>


class GM6020 
{
    uint8_t ID;//电机ID
    uint16_t fb_can_id;//电机反馈报文ID
    uint16_t ctl_can_id_cur;//电机电流模控制ID
    uint16_t ctl_can_id_vol;//电机电压模式控制ID

    int16_t voltage;//设定电压
    double voltage_provide;//供电电压 -25000 - 25000
    int16_t current;//设定电流
    int16_t current_fact;//反馈实际电流 -16384 - 16384 -3A - 3A
    uint16_t angle;//设定机械角度
    uint16_t angle_last;
    uint16_t angle_fact;//反馈机械角度 0 - 8191
    int16_t rpm;//设定转速 0- 360 rpm
    int16_t rpm_fact;//反馈速度
    double rpm_pre;//设定转速，算法版本高精度
    double rpm_pre_fact;//由反馈机械角度计算得到的实际转速；
    int circle;
    int angle2rpm_buf;
    /*待解决：丢包导致计算结果不精确问题*/
    uint8_t temp;//反馈温度

    const int ctl_Hz = 1000;//控制速度

    GM6020(uint8_t ID_,double vol_pro)
    {
        if(ID_ > 8 || ID_ < 0)
        {
           throw std::runtime_error("unvalid GM6020ID"); //电机id错误
        }

        ID = ID_;
        fb_can_id = 0x204 + ID;

        if(ID == 1 || ID == 2 || ID == 3 || ID == 4)
        {
            ctl_can_id_vol = 0x1FF;
            ctl_can_id_cur = 0x1FE;
        }
        else if(ID == 5 || ID == 6 || ID == 7 )
        {

            ctl_can_id_vol = 0x2FF;
            ctl_can_id_cur = 0x2FE;
        }//写入对应反馈信息
        
        voltage = 0;
        voltage_provide = voltage_provide;
        current = 0;
        current_fact = 0;
        angle = 0;
        angle_last = 0;
        angle_fact = 0;
        rpm = 0;
        rpm_fact = 0;
        circle = 0;
        temp = 0;//初始化
    }

    //数据读取
    // 直接读取值
    uint8_t get_ID() const { return ID; }
    int16_t get_voltage() const { return voltage; }
    double get_voltage_pro() const { return voltage_provide; }
    int16_t get_current() const { return current; }
    int16_t get_current_fact() const { return current_fact; }
    uint16_t get_angel() const { return angle; }
    uint16_t get_angle_last() const { return angle_last; }
    uint16_t get_angel_fact() const { return angle_fact; }
    int16_t get_rpm() const { return rpm; }
    int16_t get_rpm_fact() const { return rpm_fact; }
    double get_rpm_pre() const {return rpm_pre;}
    double get_rpm_pre_fact() const {return rpm_pre_fact;}
    uint8_t get_temp() const { return temp; }
    int get_circle() const { return circle; }

    // 外界读取，直接获取地址
    const uint8_t* get_ID_ptr() const { return &ID; }
    const int16_t* get_voltage_ptr() const { return &voltage; }
    const double* get_voltage_pro_ptr() const { return &voltage_provide; }
    const int16_t* get_current_ptr() const { return &current; }
    const int16_t* get_current_fact_ptr() const { return &current_fact; }
    const uint16_t* get_angel_ptr() const { return &angle; }
    const uint16_t* get_angel_last_ptr() const { return &angle_last; }
    const uint16_t* get_angel_fact_ptr() const { return &angle_fact; }
    const int16_t* get_rpm_ptr() const { return &rpm; }
    const int16_t* get_rpm_fact_ptr() const { return &rpm_fact; }
    const double* get_rpm_pre_ptr() const { return &rpm_pre;}
    const double* get_rpm_pre_fact_ptr() const { return &rpm_pre_fact;}
    const uint8_t* get_temp_ptr() const { return &temp; }
    
    //设定数据写入
    void set_voltage_RAW(int16_t vol){voltage = vol;}
    void set_voltage_24v(double vol){voltage = static_cast<int>(vol*25000/24);}
    void set_voltage_percent(double per){voltage = static_cast<int>(per*25000);}
    void set_voltage_real(double vol)
    {
        if(abs(vol) > voltage_provide)
        {
            voltage =25000;
            return;
        }
        voltage = static_cast<int>(vol*25000/voltage_provide); 
    }

    void set_current_RAW(int16_t cur){current = cur;}
    void set_current_percent(double per){current = static_cast<int16_t>(per*16384);}
    void set_current_real(double real){current = static_cast<int16_t>(real*16384/3);}

    void set_angle_RAW(uint16_t ang){angle = ang;}
    void set_angle_degree(float degree){angle = static_cast<uint16_t>(degree/360*8191);}
    void set_angle_percent(double per){angle = static_cast<uint16_t>(per*8191);}

    void set_angle_fact_RAW(uint16_t ang) { angle_fact = ang; }
    void set_angle_fact_degree(float degree){angle_fact = static_cast<uint16_t>(degree/360*8191);}
    void set_angle_fact_percent(double per){angle_fact = static_cast<uint16_t>(per*8191);}

    //RPM 一般不直接调用，GM6020以整数形式返回a转速，精度不适用于控制，只是适合读取
    void set_rpm_RAW(int16_t val){rpm = val;}
    void set_rpm_fact(int16_t val){rpm_fact = val;} 

    void set_rpm_pre_RAW(double val){rpm_pre = val;}
    void set_rpm_pre_fact_RAW(double val){rpm_pre_fact = val;}

    void set_temp(int8_t val){temp = val;}

    //can报文解码
    int data_set(struct can_frame& fb_frame)
    {
        if (fb_frame.can_id != fb_can_id)
            return 0;
        angle_fact = (fb_frame.data[0] << 8) | fb_frame.data[1];
        rpm_fact = (fb_frame.data[2] << 8) | fb_frame.data[3];
        current_fact = (fb_frame.data[4] << 8) | fb_frame.data[5];
        temp = fb_frame.data[6];


        //计算精确的rpm值&过零检测

        if (rpm_fact >= 0)
        {
            angle2rpm_buf = (angle_fact - angle_last + 8191);
            if(angle2rpm_buf > 8191)
            {
                circle++;
                rpm_pre_fact = (angle2rpm_buf%8191)/ 8191;
            }
        }
        else if (rpm_fact < 0)
        {
            rpm_pre_fact = (angle_last - angle_fact + 8191);
            if(rpm_pre_fact > 8191)
            { 
                circle--;
                rpm_pre_fact = (angle2rpm_buf%8191)/ 8191;
            }
        }
    }
    
    // 速度环和位置环实现 
    struct PID_PID_node
    {
        
    };
    
};