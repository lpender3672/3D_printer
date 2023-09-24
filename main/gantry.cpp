#include "gantry.h"

#define MAX_X (300)
#define MAX_Y (280)
#define MM_PER_SEGMENT (1)

void gantry::combined_single_step(long step_delay) {
    byte RStep = R.get_step_pin();
    byte LStep = L.get_step_pin();

    digitalWrite(RStep, HIGH);
    digitalWrite(LStep, HIGH);
    delayMicroseconds(step_delay);
    digitalWrite(RStep, LOW);
    digitalWrite(LStep, LOW);
    delayMicroseconds(step_delay);
}

void gantry::enable() {
    R.enable();
    L.enable();
}

void gantry::disable() {
    R.disable();
    L.disable();
}

void gantry::set_min_interval(float min_interval) {
    R.set_min_interval(min_interval);
    L.set_min_interval(min_interval);
}

void gantry::home(long step_delay) {
    byte RDir = R.get_direction_pin();
    byte LDir = L.get_direction_pin();

    byte xLim = R.get_limit_pin();
    byte yLim = L.get_limit_pin();

    x = 0;
    y = 0;

    // home X
    digitalWrite(RDir, LOW);
    digitalWrite(LDir, HIGH);
    while (digitalRead(xLim) == HIGH) {
        combined_single_step(step_delay);
    }
    digitalWrite(RDir, HIGH);
    digitalWrite(LDir, LOW);
    delay(step_delay);
    while (digitalRead(xLim) == LOW) {
        combined_single_step(step_delay);
    }
    delay(step_delay);

    // home Y
    digitalWrite(RDir, LOW);
    digitalWrite(LDir, LOW);
    delay(step_delay);
    while (digitalRead(yLim) == HIGH) {
        combined_single_step(step_delay);
    }
    digitalWrite(RDir, HIGH);
    digitalWrite(LDir, HIGH);
    while (digitalRead(yLim) == LOW) {
        combined_single_step(step_delay);
    }
    delay(step_delay);
}

void gantry::translate(float dx, float dy, float de = 0.0) {

    float kx = 6400/40;
    float ky = 6400/40;
    float ke = E.microsteps_pu;

    long Lsteps = 0;
    long Rsteps = 0;

    Rsteps += kx * dx;
    Lsteps -= kx * dx;

    Rsteps += ky * dy;
    Lsteps += ky * dy;

    long extruder_steps = ke * de;

    // Rsteps > Lsteps
    // 
    if (abs(Rsteps) > abs(Lsteps)) {
        Bresenham_Step(R, Rsteps, L, Lsteps, extruder_steps);
    } else {
        
        Bresenham_Step(L, Lsteps, R, Rsteps, extruder_steps);
    }

}

void gantry::Bresenham_Step(stepper& Astepper, long Asteps, stepper& Bstepper, long Bsteps, long Esteps){
    // abs Asteps > abs Bsteps required for proper operation

    digitalWrite(Astepper.get_direction_pin(), Asteps < 0 ? (not Astepper.default_direction) : Astepper.default_direction);
    digitalWrite(Bstepper.get_direction_pin(), Bsteps < 0 ? (not Bstepper.default_direction) : Bstepper.default_direction);
    digitalWrite(E.get_direction_pin(),  Esteps < 0 ? (not E.default_direction) : E.default_direction);

    unsigned long abs_Asteps = abs(Asteps);
    unsigned long abs_Bsteps = abs(Bsteps);
    unsigned long abs_Esteps = abs(Esteps);
    unsigned long abs_half_Asteps = abs_Asteps / 2;
    
    float step_delay = 200.0; // initial step delay

    unsigned long i = 0;
    long overB = 0;
    long overE = 0;

    float min_interval = Astepper.get_min_interval();
    float interval_incriment = Astepper.get_interval_increment();

    if (interval_incriment == 0.0) { 
        step_delay = min_interval; // basically if acceleration is disabled, use constant speed
    }

    while (step_delay > min_interval && i < abs_half_Asteps) 
    {
        Astepper.single_step(step_delay);
        overB += abs_Bsteps;
        if (overB >= abs_Asteps){
            overB -= abs_Asteps;
            Bstepper.single_step(step_delay);
        }

        overE += abs_Esteps;
        if (overE >= abs_Asteps){
            overE -= abs_Asteps;
            E.single_step(step_delay);
        }
        
        step_delay -= interval_incriment;
        i++;
    }

    if (step_delay < min_interval) 
    {
        step_delay = min_interval;
    } 

    unsigned long acceleration_steps = i;
    unsigned long max_speed_steps = abs_Asteps - acceleration_steps;

    while (i < max_speed_steps) 
    {
        Astepper.single_step(step_delay);
        overB += abs_Bsteps;
        if (overB >= abs_Asteps){
            overB -= abs_Asteps;
            Bstepper.single_step(step_delay);
        }

        overE += abs_Esteps;
        if (overE >= abs_Asteps){
            overE -= abs_Asteps;
            E.single_step(step_delay);
        }

        i++;
    }

    while (i < abs_Asteps) 
    {
        Astepper.single_step(step_delay);
        overB += abs_Bsteps;
        if (overB >= abs_Asteps){
            overB -= abs_Asteps;
            Bstepper.single_step(step_delay);
        }

        overE += abs_Esteps;
        if (overE >= abs_Asteps){
            overE -= abs_Asteps;
            E.single_step(step_delay);
        }

        step_delay += interval_incriment;
        i++;
    }
}

void gantry::move_line(float dx, float dy, float de = 0.0) {
    // boundary check
    if (x + dx > MAX_X || x + dx < 0) {
        return;
    }
    if (y + dy > MAX_Y || y + dy < 0) {
        return;
    }

    x+= dx;
    y+= dy;

    translate(dx, dy, de);
}

void gantry::move_arc(float cx, float cy, float radius ,float theta) {
  
  // get length of arc
  
  float len = abs(theta) * radius;

  int i, segments = ceil( len * MM_PER_SEGMENT );
 
  float nx, ny, angle3, scale;

  for (i=0; i<segments; i++) {
    // interpolate around the arc
    scale = ((float)i)/((float)segments);
    
    angle3 = ( theta * scale );
    nx = cx + cos(angle3) * radius;
    ny = cy + sin(angle3) * radius;
    
    move_line(nx,ny);

    Serial.println(nx);
    Serial.println(ny);
    Serial.println();
  }
  
  move_line(x, y);
  
}