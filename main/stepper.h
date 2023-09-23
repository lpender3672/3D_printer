#include <Arduino.h>

class stepper {
private:
    byte step_pin;
    byte direction_pin;
    byte enable_pin;
    byte limit_pin;

    long current_steps;
    float min_interval; // microseconds
    float interval_increment; // microseconds
    const float initial_step_delay = 100; // microseconds

public:
    stepper(byte _step_pin, byte _direction_pin, bool _default_direction, byte _enable_pin, byte _limit_pin, float _max_position = 300.0);

    void begin();

    float microsteps_pu;
    float position;
    float max_position = 300;
    bool default_direction;

    byte get_step_pin() { return step_pin; }
    byte get_direction_pin() { return direction_pin; }
    byte get_enable_pin() { return enable_pin; }
    byte get_limit_pin() { return limit_pin; }

    void set_motor_config(float microsteps, float steps_per_revolution, float mechanical_ratio);
    void set_min_interval(float _min_interval);
    void set_interval_increment(float _interval_increment);

    float get_min_interval() { return min_interval; }
    float get_interval_increment() { return interval_increment; }

    void set_direction(bool direction);
    void zero_position();
    void disable();
    void enable();
    void single_step(float step_delay);

    void step(long steps);
    void translate(float dx);
};
