#include "stepper.h"

stepper::stepper(byte _step_pin, byte _direction_pin, bool _default_direction, byte _enable_pin, byte _limit_pin, float _max_position)
{
    step_pin = _step_pin;
    direction_pin = _direction_pin;
    enable_pin = _enable_pin;
    limit_pin = _limit_pin;
    default_direction = _default_direction;

    microsteps_pu = 32 * 200 * 1;
    position = 0.0;
    max_position = _max_position;
}

void stepper::begin()
{
    

    pinMode(step_pin, OUTPUT);
    pinMode(direction_pin, OUTPUT);
    pinMode(enable_pin, OUTPUT);
    pinMode(limit_pin, INPUT_PULLUP);
}

void stepper::set_motor_config( float microsteps,
                                float steps_per_revolution,
                                float mechanical_ratio)
{
    microsteps_pu = microsteps * steps_per_revolution / mechanical_ratio;
}

void stepper::set_min_interval(float _min_interval)
{
    min_interval = _min_interval; // speed
}

void stepper::set_interval_increment(float _interval_increment)
{
    interval_increment = _interval_increment; // acceleration
}

void stepper::single_step(float step_delay)
{
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(step_delay);
    digitalWrite(step_pin, LOW);
    delayMicroseconds(step_delay);
}

void stepper::set_direction(bool direction)
{
    digitalWrite(direction_pin, direction);
}

void stepper::zero_position()
{
    position = 0;
}

void stepper::enable()
{
    digitalWrite(enable_pin, HIGH);
}

void stepper::disable()
{
    digitalWrite(enable_pin, LOW);
}

void stepper::step(long steps)
{
    if (steps > 0)
    {
        digitalWrite(direction_pin, default_direction);
    }
    else 
    {
        digitalWrite(direction_pin, (not default_direction));
    }

    unsigned long abs_steps = abs(steps);
    unsigned long abs_half_steps = abs_steps / 2;
    float step_delay = initial_step_delay;

    unsigned long i = 0;

    while (step_delay > min_interval && i < abs_half_steps) 
    {
        single_step(step_delay);
        step_delay -= interval_increment;
        i++;
    }

    if (step_delay < min_interval) 
    {
        step_delay = min_interval;
    } 

    unsigned long acceleration_steps = i;
    unsigned long max_speed_steps = abs_steps - acceleration_steps;

    while (i < max_speed_steps) 
    {
        single_step(min_interval);
        i++;
    }

    while (i < abs_steps) 
    {
        single_step(step_delay);
        step_delay += interval_increment;
        i++;
    }
}

void stepper::translate(float dx)
{

    Serial.print("position: ");
    Serial.println(position);
    Serial.print("dx: ");
    Serial.println(dx);

    // convert dx to steps
    if (position + dx > max_position || position + dx < 0) {
        return;
    }

    position += dx;
    
    step(dx * microsteps_pu);
}
