#include <Arduino.h>

class temp_PID {
public:
    temp_PID(byte _thermistor_pin, byte _pwm_pin);

    void set_PID(float Kp, float Ki, float Kd);
    void set_setpoint(float setpoint);
    void update(unsigned long dt); // dt in milliseconds
    float get_control();
    float get_error();
    float get_error_sum();
    float get_error_last();
    float get_setpoint();
    
    float get_temp();
    void wait_for_setpoint();

private:

    byte pwm_heater_pin;
    byte analog_thermistor_pin;

    long Resistor = 100000; // Ohms

    float Kp;
    float Ki;
    float Kd;
    float setpoint;
    float error;
    float error_sum;
    float error_last;
    float control;

};
