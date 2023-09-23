#include <Arduino.h>
#include "temp_pid.h"
#include "gantry.h"

#define MAX_BUF 64
char buffer[MAX_BUF];

// pin assignments
stepper X(2, 5, true, 8, 9);
stepper Y(3, 6, true, 8, 10);
stepper Z(4, 7, false, 8, 11, 260);
stepper E(12, 13, false, 8, 11);

gantry XY_gantry(X, Y, E);

temp_PID nozzle(A1, 44); // Input on A1,  output on pin 44 and set 45 to LOW
temp_PID bed(A2, 46); // Input on pin A2,  output on pin 46 and set 47 to LOW

bool mode_abs = true;
int   sofar;

long previous_millis;

void setup()
{
    Serial.begin(115200);
    Serial.println("3Dprinter");

    nozzle.set_PID(3, 0.01, 0);
    pinMode(45, OUTPUT);
    digitalWrite(45, LOW);
    nozzle.set_setpoint(0.0);

    bed.set_PID(3, 0, 0);
    pinMode(47, OUTPUT);
    digitalWrite(47, LOW);
    bed.set_setpoint(0.0);

    X.begin();
    X.set_motor_config(32, 200, 1);
    X.set_min_interval(20);
    X.set_interval_increment(0.05);

    Y.begin();
    Y.set_motor_config(32, 200, 1);
    Y.set_min_interval(20);
    Y.set_interval_increment(0.05);

    Z.begin();
    Z.set_motor_config(32, 200, 8);
    Z.set_min_interval(50);
    Z.set_interval_increment(0.0);

    E.begin();
    E.set_motor_config(32, 200, 8);
    E.set_min_interval(20);
    E.set_interval_increment(0.0);

    // XY_gantry.home(80);
    // Z_home(100);

    //XY_gantry.move_line(100.0, 50.0);

    previous_millis = millis();
}


/*  cartesian arc input to polar arc input

  float dx = px - cx; 
  float dy = py - cy;
  float radius=sqrt(dx*dx+dy*dy);

  // find angle of arc (sweep)
  float angle1=atan3(dy,dx);
  float angle2=atan3(y-cy,x-cx);
  float theta=angle2-angle1;

*/

void Z_home(float step_delay)
{
    Z.set_direction(0);
    while (digitalRead(Z.get_limit_pin()) == LOW)
    {
        Z.single_step(step_delay);
    }
    delay(step_delay);
    Z.set_direction(1);
    while (digitalRead(Z.get_limit_pin()) == HIGH)
    {
        Z.single_step(step_delay);
    }
    Z.zero_position();
    delay(step_delay);
}


/*

void commands() {
  Serial.println(F("G00 [X(steps)] [Y(steps)] [F(feedrate)]; - line"));
  Serial.println(F("G01 [X(steps)] [Y(steps)] [F(feedrate)]; - line"));
  Serial.println(F("G02 [X(steps)] [Y(steps)] [I(steps)] [J(steps)] [F(feedrate)]; - clockwise arc"));
  Serial.println(F("G03 [X(steps)] [Y(steps)] [I(steps)] [J(steps)] [F(feedrate)]; - counter-clockwise arc"));
  Serial.println(F("G04 P[seconds]; - delay"));
  Serial.println(F("G90; - absolute mode"));
  Serial.println(F("G91; - relative mode"));
  Serial.println(F("G92 [X(steps)] [Y(steps)]; - change logical position"));
  Serial.println(F("M18; - disable motors"));
  Serial.println(F("M100; - this help message"));
  Serial.println(F("M114; - report position and feedrate"));
  Serial.println(F("All commands must end with a newline."));
}

*/
void where() {
  Serial.print("X");
  Serial.println(XY_gantry.x);
  Serial.print("Y");
  Serial.println(XY_gantry.y);
} 

float parseNumber(char code, float default_value = 0.0) {
  char *ptr=buffer;  // start at the beginning of buffer
  while((long)ptr > 1 && (*ptr) && (long)ptr < (long)buffer+sofar) {  // walk to the end
    if(*ptr==code) {  // if you find code on your walk,
      return atof(ptr+1);  // convert the digits that follow into a float and return it
    }
    ptr=strchr(ptr,' ')+1;  // take a step from here to the letter after the next space
  }
  return default_value;  // end reached, nothing found, return default value
}

void processCommand() {
    float px = XY_gantry.x;
    float py = XY_gantry.y;
    float pz = Z.position;
    float pe = E.position;
    //float pz = Z.position;

    int cmd = parseNumber('G', -1);
    switch(cmd) {
    case  0:
    case  1: { // line

      float speed = parseNumber('F');
      float dx = parseNumber('X', mode_abs?px:0) - (mode_abs?px:0);
      float dy  = parseNumber('Y', mode_abs?py:0) - (mode_abs?py:0);
      float dz = parseNumber('Z', mode_abs?pz:0) - (mode_abs?pz:0);
      float de = parseNumber('E', mode_abs?pe:0) - (mode_abs?pe:0);

      if (speed > 0) {
          float distance = sqrt(dx*dx + dy*dy + dz*dz + de*de);
          long steps = distance * 6400 / 40;
          float time = 60 * distance / speed; // speed in mm/min
          float step_delay = time / steps * 1000000; // microseconds

      }

      Z.translate(dz);
      //E.translate(de);

      XY_gantry.move_line(dx, dy, de);
      break;
      }

    case 2: { // arc clockwise
      float I = parseNumber('I') - (mode_abs?px:0);
      float J = parseNumber('J') - (mode_abs?py:0);
      float tX = parseNumber('X') - (mode_abs?px:0);
      float tY = parseNumber('Y') - (mode_abs?py:0);
      break;

    }
    case 3: {  // arc counter clockwise
      float I = parseNumber('I') - (mode_abs?px:0);
      float J = parseNumber('J') - (mode_abs?py:0);
      float tX = parseNumber('X') - (mode_abs?px:0);
      float tY = parseNumber('Y') - (mode_abs?py:0);
      break;

    }
    case  4: { // sleep
      long duration = parseNumber('P');
      delay(duration);
      break;
    }

    case 28: {
      XY_gantry.home(80);
      Z_home(100);
      break;
    }
    case 82:
    case 90: 
      mode_abs = true;
    break;  // absolute mode

    case 91:
      mode_abs = false; 
      break;  // relative mode

    case 92:  // set logical position
      XY_gantry.x = parseNumber('X');
      XY_gantry.y = parseNumber('Y');
      break;

    default:
      break;
    }

    cmd = parseNumber('M');
    switch(cmd) {
    case 17: {// enable motors
    XY_gantry.enable();
    break;
    }

    case 18: {  // disable motors
    XY_gantry.disable();
    break;
    }

    case 100: {
    Serial.println("3Dprinter");
    break;
    }

    case 114: {
    where();
    break;
    }

    case 105: { // report temperatures
      Serial.print("Nozzle T:");
      Serial.println(nozzle.get_temp());

      Serial.print("Bed T:");
      Serial.println(bed.get_temp());
      break;
    }

    case 106: { // set fan speed
      float fan_speed = parseNumber('S');
      // set fan speed
      break;
    }

    case 104:
    case 109: { // nozzle temp
      float nozzle_temp = parseNumber('S') / 2.0;
      nozzle.set_setpoint(nozzle_temp);
      // wait for nozzle to reach stable temp
      while (nozzle.get_temp() < nozzle_temp) {
        PID_update();
      }
    
      break;
    }

    case 140:
    case 190: { // bed temp
      float bed_temp = parseNumber('S') / 2.0;
      bed.set_setpoint(bed_temp);
      // wait for bed to reach stable temp
      while (bed.get_temp() < bed_temp) {
        PID_update();
      }

      break;
    }

    default: break;
    }
}

void PID_update() {
  long now = millis();
  float dt = (now - previous_millis) / 1000.0;

  // PID updates
  bed.update(dt);
  nozzle.update(dt);

  previous_millis = now;
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
      //Serial.print(F("\r\n"));  // echo a return character for humans
      processCommand();  // do something with the command
      Serial.println("OK");
      sofar=0;

      //delay(100);
      }
    PID_update();
    }
    
}