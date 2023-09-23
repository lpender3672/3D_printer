#include <Arduino.h>
#include "temp_pid.h"

/*
float c1 = 1.009249522e-03;
float c2 = 2.378405444e-04;
float c3 = 2.019202697e-07;
*/

float c1 = 1.1384e-3;
float c2 = 2.3245e-4;
float c3 = 9.489e-8;


temp_PID::temp_PID(byte _thermistor_pin, byte _pwm_pin) {

    this->pwm_heater_pin = _pwm_pin;
    this->analog_thermistor_pin = _thermistor_pin;

    pinMode(this->pwm_heater_pin, OUTPUT);
    pinMode(this->analog_thermistor_pin, INPUT);
    
    this->setpoint = 0;
    this->error = 0;
    this->error_sum = 0;
    this->error_last = 0;
}

float temp_PID::get_temp() {

    float f = analogRead(analog_thermistor_pin) / 1023.0;
    float R = Resistor * ( 1.0 / f - 1.0); // ohm
    //Serial.println(R);
    float logR = log(R);
    // Steinhart–Hart equation
    return (1.0 / (c1 + c2*logR + c3*logR*logR*logR)) - 273.15 + 60;

    // why is this not working?

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

    if (this->setpoint == 0) {
        this->error_sum = 0;
        // this is to prevent the integral term from accumulating error when the setpoint is 0
    }

    this->error = this->setpoint - get_temp();
    this->error_sum += this->error * dt;
    float error_diff = (this->error - this->error_last) / dt;
    this->control = this->Kp * this->error + this->Ki * this->error_sum + this->Kd * error_diff;
    this->error_last = this->error;

    // set PWM

    if (this->control > 255) {
        this->control = 255;
    }
    else if (this->control < 0) {
        this->control = 0;
    }

    analogWrite(this->pwm_heater_pin, this->control);
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
