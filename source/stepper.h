#include "Arduino.h"

class stepper {

private:
    byte step_pin;
    byte direction_pin;
    byte enable_pin;
    byte limit_pin;

    volatile long current_steps;
    volatile double position;
    volatile double speed;
    volatile double acceleration;

    volatile double conversion_factor;
    volatile bool conversion_factor_set;

    volatile double microsteps_per_revolution;
    volatile bool motor_config_set;

    const long initial_step_delay = 100;

public:

    stepper();

    void begin(byte _step_pin, byte _direction_pin, byte _enable_pin, byte _limit_pin);

    byte get_step_pin() { return step_pin; }
    byte get_direction_pin() { return direction_pin; }
    byte get_enable_pin() { return enable_pin; }
    byte get_limit_pin() { return limit_pin; }

    void set_conversion_factor(double _conversion_factor);
    void set_motor_config(double microsteps, double steps_per_revolution, double gear_ratio);
    void set_speed(double _speed);
    void set_acceleration(double _acceleration);

    void step(double step_delay);
    void zero_position();
    void translate(double distance);
};
