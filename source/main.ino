
#include "pid.h"
#include "stepper.h"


stepper z;

void setup()
{
    Serial.begin(9600);
    Serial.println("Hello World!");

    z.begin(2, 5, 8, 9);
    z.set_conversion_factor(8);
    z.set_motor_config(32, 200, 1);

    z.set_acceleration(20);
    z.set_speed(50);

    delay(1000);
    z.translate(-32);
}

void loop()
{
    if (digitalRead(10) == 1)
    {
        while(digitalRead(10) == 1)
        z.step(-32);
    }
    else if (digitalRead(11) == 1)
    {
        while(digitalRead(11) == 1)
        z.translate(32);
    }
}