#include <Arduino.h>
#include "stepper.h"


class gantry {
private:
    stepper& R;
    stepper& L;
    stepper& E;

public:

    float x;
    float y;

    gantry(stepper& _R, stepper& _L, stepper& _E) : R(_R), L(_L), E(_E) {
        x = 0;
        y = 0;
    }

    void set_min_interval(float min_interval);
    
    void translate(float dx, float dy, float de = 0.0);

    void move_line(float dx, float dy, float de = 0.0);

    void move_arc(float cx, float cy, float r, float theta);

    void Bresenham_Step(stepper& stepper1, long steps1, stepper& stepper2, long steps2, long Esteps);

    void home(long step_delay);

    void combined_single_step(long step_delay);

    void enable();

    void disable();
};

