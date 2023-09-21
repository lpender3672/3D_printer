#include <Arduino.h>
#include "stepper.h"


class gantry {
private:
    stepper& R;
    stepper& L;

public:

    float x;
    float y;

    gantry(stepper& _R, stepper& _L) : R(_R), L(_L) {
        x = 0;
        y = 0;
    }
    
    void translate(float dx, float dy);

    void move_line(float dx, float dy);

    void move_arc(float cx, float cy, float r, float theta);

    void Bresenham_Step(stepper& stepper1, long steps1, stepper& stepper2, long steps2);
    void home(long step_delay);
    void combined_single_step(long step_delay);
    void enable();
    void disable();
};

