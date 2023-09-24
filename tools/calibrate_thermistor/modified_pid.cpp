#include <Arduino.h>
#include "modified_pid.h"

/*
float c1 = 1.009249522e-03;
float c2 = 2.378405444e-04;
float c3 = 2.019202697e-07;
*/

float c1 = 1.1384e-3;
float c2 = 2.3245e-4;
float c3 = 9.489e-8;

float R_table[] = {100000.0, 10000.0}; // data from calibration
float T_table[] = {24.0, 200.0}; // data from calibration


float Log_Interpolate(float x, float x1, float x2, float y1, float y2) {
    return y1 + (y2 - y1) * (log(x) - log(x1)) / (log(x2) - log(x1));
}

float T_from_R(float R) {
    int i = 0;
    while (R_table[i] < R) {
        i++;
    }
    return Log_Interpolate(R, R_table[i-1], R_table[i], T_table[i-1], T_table[i]);
}

modified_PID::modified_PID(byte _thermistor_pin, byte _pwm_pin) {

    this->pwm_heater_pin = _pwm_pin;
    this->analog_thermistor_pin = _thermistor_pin;

    pinMode(this->pwm_heater_pin, OUTPUT);
    pinMode(this->analog_thermistor_pin, INPUT);
    
    this->setpoint = 0;
    this->error = 0;
    this->error_sum = 0;
    this->error_last = 0;
}

float modified_PID::get_resistance() {

    float f = analogRead(analog_thermistor_pin) / 1023.0;
    float R = Resistor * ( 1.0 / f - 1.0); // ohm
    return R;

}

void modified_PID::set_PID(float Kp, float Ki, float Kd) {
    this->Kp = Kp;
    this->Ki = Ki;
    this->Kd = Kd;
}

void modified_PID::set_setpoint(float setpoint) {
    this->setpoint = setpoint;
    this->error_sum = 0;
}

void modified_PID::wait_for_setpoint() {
    while (abs(get_resistance() - this->setpoint) / this->setpoint > 0.01) { // 1% tolerance
        delay(100);
        update(100);
    }
}

void modified_PID::update(unsigned long dt) {
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

    this->error = this->setpoint / get_resistance() - 1.0; // relative error
    this->error_sum += this->error * dt;
    float error_diff = (this->error - this->error_last) / (dt / 1000.0);
    this->control = this->Kp * this->error + this->Ki * this->error_sum + this->Kd * error_diff;
    this->error_last = this->error;

    this->control = - this->control; // NTC thermistor

    // set PWM

    if (this->control > 255) {
        this->control = 255;
    }
    else if (this->control < 0) {
        this->control = 0;
    }

    analogWrite(this->pwm_heater_pin, this->control);
}
