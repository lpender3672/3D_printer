
#include <Arduino.h>
#include "modified_pid.h"

#define MAX_BUF 64
char buffer[MAX_BUF];
int   sofar;

#define THERMISTOR_RESISTANCE 100000.0 // ohms

modified_PID hotend(A1, 44);
unsigned long previous_millis;
float setpoint;

void setup() {
    Serial.begin(115200);
    Serial.println("Thermistor calibration tool");
    
    hotend.set_PID(30.0, 10.0, 0.0);

    pinMode(45, OUTPUT);
    digitalWrite(45, LOW);

    // setpoint is in ohms now
    setpoint = 1000.0;
    hotend.set_setpoint(setpoint); // 100k ohm thermistor should be room temp

    previous_millis = 0;

}

void PID_update() {
  unsigned long now = millis();
  unsigned long dt = now - previous_millis;

  // PID updates
  hotend.update(dt);

  previous_millis = now;
}

void print_mean_resistance(){
    float mean = 0;
    int n = 10;
    for (int i = 0; i < n; i++) {
        mean += hotend.get_resistance();
        delay(100);
    }
    mean /= n;
    Serial.print("R = ");
    Serial.println(mean); // print the average resistance
}

void loop() 
{
    PID_update();

    while(Serial.available() > 0) {  // if something is available
    char c = Serial.read();  // get it
    // Serial.print(c);  // repeat it back so I know you got the message
    if(sofar<MAX_BUF-1) buffer[sofar++]=c;  // store it
    if((c=='\n') || (c == '\r')) {
        // entire message received
        buffer[sofar]=0;  // end the buffer so string functions work right
        

        if (buffer[0] == 'N') {
            setpoint *= 0.75;
            Serial.print("Moving to next setpoint");
            Serial.println(setpoint);

            Serial.print("currently R = ");
            Serial.println(hotend.get_resistance());
            
            hotend.set_setpoint(setpoint);
            // hotend.wait_for_setpoint();

            // take 10 readings and average them
            print_mean_resistance();

        } else if (buffer[0] == 'P') {
            print_mean_resistance();
        }
        
        sofar=0;

      }
    }
    delay(100);
}
