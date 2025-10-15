#include <cstdint>
#include <stdexcept> 
#include <iostream>


class GM6020 
{
    uint8_t ID;//电机ID
    uint16_t fb_can_id;//电机反馈报文ID
    uint16_t ctl_can_id_cur;//电机电流模控制ID
    uint16_t ctl_can_id_vol;//电机电压模式控制ID

    int16_t voltage;//设定电压
    double voltage_provide;//供电电压
    int16_t current;//设定电流
    int16_t current_fact;//反馈实际电流
    uint16_t angle_m;//反馈机械角度
    uint16_t rpm;//反馈速度
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
        angle_m = 0;
        rpm = 0;
        temp = 0;//初始化
    }

    //数据读取
    const uint8_t* get_ID(){return const_cast<const uint8_t*>(&ID);}
    const int16_t* get_voltage(){return const_cast<const int16_t*>(&voltage);}
    const double* get_voltage_pro(){return const_cast<const double*>(&voltage_provide);}
    const int16_t* get_current(){return const_cast<const int16_t*>(&current);}
    const int16_t* get_current_fact(){return const_cast<const int16_t*>(&current_fact);}
    const uint16_t* get_angel(){return const_cast<const uint16_t*>(&angle_m);}
    const uint16_t* get_rpm(){return const_cast<const uint16_t*>(&rpm);}
    const uint8_t* get_temp(){return const_cast<const uint8_t*>(&temp);}

    //数据写入
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
    void set_current_fact(int cur_fact){current_fact = cur_fact;}
    

     
    struct PID_PID_node
    {

    };
    
};