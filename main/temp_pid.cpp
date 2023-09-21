#include <Arduino.h>
#include "temp_pid.h"

#define c1 1.009249522e-03
#define c2 2.378405444e-04
#define c3 2.019202697e-07


temp_PID::temp_PID(byte _pwm_pin, byte _thermistor_pin) {

    this->pwm_heater_pin = _pwm_pin;
    this->analog_thermistor_pin = _thermistor_pin;
    
    this->setpoint = 0;
    this->error = 0;
    this->error_sum = 0;
    this->error_last = 0;
}

float temp_PID::get_temp() {

    float f = analogRead(analog_thermistor_pin) / 1023.0;
    float R = Resistor * f / (1 - f); // ohm
    float logR = log(R);
    // Steinhart–Hart equation
    return (1.0 / (c1 + c2*logR + c3*logR*logR*logR)) - 273.15 + 50;
}

void temp_PID::set_PID(float Kp, float Ki, float Kd) {
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
}

void temp_PID::set_setpoint(float setpoint) {
    this->setpoint = setpoint;
    this->error_sum = 0;
}

void temp_PID::update(float dt) {
    /*
    *  PID controller
    Inputs:
        feedback: measured value between 0 and 1
        dt: time step in seconds
    */
    this->error = this->setpoint - get_temp();
    this->error_sum += this->error * dt;
    float error_diff = (this->error - this->error_last) / dt;
    this->control = this->Kp * this->error + this->Ki * this->error_sum + this->Kd * error_diff;
    this->error_last = this->error;


}

float temp_PID::get_control() {
    return this->control;
}

float temp_PID::get_error() {
    return this->error;
}

float temp_PID::get_error_sum() {
    return this->error_sum;
}

float temp_PID::get_error_last() {
    return this->error_last;
}

float temp_PID::get_setpoint() {
    return this->setpoint;
}
