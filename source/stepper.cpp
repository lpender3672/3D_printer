#include "stepper.h"
#include "Arduino.h"

stepper::stepper()
{

}

void stepper::begin(byte _step_pin, byte _direction_pin, byte _enable_pin, byte _limit_pin)
{
    step_pin = _step_pin;
    direction_pin = _direction_pin;
    enable_pin = _enable_pin;
    limit_pin = _limit_pin;
    conversion_factor = 1;
    conversion_factor_set = false;
    motor_config_set = false;

    pinMode(step_pin, OUTPUT);
    pinMode(direction_pin, OUTPUT);
    pinMode(enable_pin, OUTPUT);
    pinMode(limit_pin, INPUT);
}

void stepper::set_conversion_factor(double _conversion_factor)
{
    conversion_factor = _conversion_factor; // mm per revolution
    conversion_factor_set = true;
}
void stepper::set_motor_config(double microsteps, double steps_per_revolution, double gear_ratio)
{
    microsteps_per_revolution = microsteps * steps_per_revolution * gear_ratio;
    motor_config_set = true;
}
void stepper::set_speed(double _speed)
{
    speed = _speed;
}
void stepper::set_acceleration(double _acceleration)
{
    acceleration = _acceleration;
}

void stepper::step(double step_delay)
{
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(step_delay);
    digitalWrite(step_pin, LOW);
    delayMicroseconds(step_delay);
}

void stepper::zero_position()
{
    position = 0;
}

void stepper::translate(double distance)
{

    if (!motor_config_set)
    {
        Serial.println("Motor config not set");
        return;
    }

    if (!conversion_factor_set)
    {
        Serial.println("Warning: Conversion factor not set");
    }

    position += distance;
    if (distance > 0)
    {
        digitalWrite(direction_pin, 1);
    }
    else 
    {
        digitalWrite(direction_pin, 0);
    }

    double distance_per_step = conversion_factor / microsteps_per_revolution;
    long steps = abs(distance) / distance_per_step;
    long half_steps = steps / 2;
    long step_delay = initial_step_delay;
    double step_delay_incriment = distance_per_step / acceleration;
    long min_step_delay = pow(10, 6 ) * distance_per_step / speed;

    Serial.println(min_step_delay);

    unsigned long i = 0;

    while (digitalRead(limit_pin) == 0 && step_delay > min_step_delay && i < half_steps) 
    {
        step(step_delay);
        step_delay -= step_delay_incriment;
        i++;
    }

    if (step_delay < min_step_delay) 
    {
        step_delay = min_step_delay;
    } 

    unsigned long acceleration_steps = i;
    unsigned long max_speed_steps = steps - acceleration_steps;

    while (digitalRead(limit_pin) == 0 && i < max_speed_steps) 
    {
        step(step_delay);
        i++;
    }

    while (digitalRead(limit_pin) == 0 && i < steps) 
    {
        step(step_delay);
        step_delay += step_delay_incriment;
        i++;
    }
    Serial.println("FINISHED");

}