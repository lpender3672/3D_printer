#include <Servo.h>

#define enable 0x8
#define RDir 0x5
#define RStep 0x2

#define LDir 0x6
#define LStep 0x3

#define xLim 0x9
#define yLim 0xa

#define zServo 12

#define MAX_BUF        (64)
#define BAUD           (57600)

#define MM_PER_SEGMENT (1)
#define ARC_CW          (1)
#define ARC_CCW         (-1)

char  buffer[MAX_BUF];  // where we store the message until we get a newline
int   sofar;            // how much is in the buffer
char mode_abs=1;
float fr =     0;

float kx = 6400/40;
float ky = 6400/40;

float X = 0;
float Y = 0;

float px = 0;
float py = 0;

Servo Z;

void stepForward(int time) {
      digitalWrite(RStep, HIGH);
      digitalWrite(LStep, HIGH);
      delayMicroseconds(time);
      digitalWrite(RStep, LOW);
      digitalWrite(LStep, LOW);
      delayMicroseconds(time);
}

void pause(long ms) {
  delay(ms/1000);
  delayMicroseconds(ms%1000);  // delayMicroseconds doesn't work for values > ~16k.
}

void output(const char *code,float val) {
  Serial.print(code);
  Serial.println(val);
}

void position(float npx,float npy) {
  // here is a good place to add sanity tests
  px=npx;
  py=npy;
}

void where() {
  output("X",px);
  output("Y",py);
  output("F",fr);
  Serial.println(mode_abs?"ABS":"REL");
} 

void home(int time) {

  digitalWrite(RDir, LOW);
  digitalWrite(LDir, HIGH);
  while (digitalRead(xLim) == HIGH) {
      stepForward(time);
  }
  digitalWrite(RDir, HIGH);
  digitalWrite(LDir, LOW);
  
  while (digitalRead(xLim) == LOW) {
      stepForward(time);
  }
  delay(time);
  
  digitalWrite(RDir, HIGH);
  digitalWrite(LDir, HIGH);
  while (digitalRead(yLim) == HIGH) {
      stepForward(time);
  }
  digitalWrite(RDir, LOW);
  digitalWrite(LDir, LOW);
  while (digitalRead(yLim) == LOW) {
      stepForward(time);
  }
}

void line(float x, float y, int delay = 40) {

   if ( x > 300 || x < 0 || y > 280 || y < 0) {
      //digitalWrite(enable, HIGH);
   }
  
   long Rsteps = 0;
   long Lsteps = 0;

   long xS = kx * ( x - px );
   Rsteps += xS;
   Lsteps -= xS;
  
   long yS = ky * ( y - py );
   Rsteps -= yS;
   Lsteps -= yS;
   
   digitalWrite(RDir, Rsteps < 0 ? LOW : HIGH);
   digitalWrite(LDir, Lsteps < 0 ? LOW : HIGH);

   Rsteps = abs(Rsteps);
   Lsteps = abs(Lsteps);

   // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
  
   long i;
   long over = 0;
   
   if (Rsteps > Lsteps) {
      
      for (i = 0; i < Rsteps; i++) {
        digitalWrite(RStep, HIGH);
        delayMicroseconds(delay);
        digitalWrite(RStep, LOW);
        delayMicroseconds(delay);

        over += Lsteps;

        if (over >= Rsteps) {
          over -= Rsteps;
          
          digitalWrite(LStep, HIGH);
          delayMicroseconds(delay);
          digitalWrite(LStep, LOW);
          delayMicroseconds(delay);
        }
      } 
      
      
   } else {

      for (i = 0; i < Lsteps; i++) {
         digitalWrite(LStep, HIGH);
         delayMicroseconds(delay);
         digitalWrite(LStep, LOW);
         delayMicroseconds(delay);
         over += Rsteps;

         if (over >= Lsteps) {
            over -= Lsteps;
            digitalWrite(RStep, HIGH);
            delayMicroseconds(delay);
            digitalWrite(RStep, LOW);
            delayMicroseconds(delay);
         }
      }
   }

   px = x;
   py = y;
}

float atan3(float dy,float dx) {
  float a = atan2(dy,dx);
  if(a<0) a = (PI*2.0)+a;
  return a;
}

void arc(float cx,float cy,float x,float y,float dir) {
  // get radius
  float dx = px - cx;
  float dy = py - cy;
  float radius=sqrt(dx*dx+dy*dy);

  // find angle of arc (sweep)
  float angle1=atan3(dy,dx);
  float angle2=atan3(y-cy,x-cx);
  float theta=angle2-angle1;
  
  if(dir>0 && theta<0) angle2+=2*PI;
  else if(dir<0 && theta>0) angle1+=2*PI;
  
  theta=angle2-angle1;
  
  // get length of arc
  
  float len = abs(theta) * radius;

  int i, segments = ceil( len * MM_PER_SEGMENT );
 
  float nx, ny, angle3, scale;

  for(i=0;i<segments;++i) {
    // interpolate around the arc
    scale = ((float)i)/((float)segments);
    
    angle3 = ( theta * scale ) + angle1;
    nx = cx + cos(angle3) * radius;
    ny = cy + sin(angle3) * radius;
    
    line(nx,ny);

    Serial.println(nx);
    Serial.println(ny);
    Serial.println();
  }
  
  line(x,y);
  
}


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


void setup() {

  pinMode(xLim,INPUT_PULLUP);
  pinMode(yLim,INPUT_PULLUP);
  
  pinMode(RDir, OUTPUT);
  pinMode(LDir, OUTPUT);
  pinMode(RStep, OUTPUT);
  pinMode(LStep, OUTPUT);
  
  pinMode(enable, OUTPUT);
  digitalWrite(enable, LOW);


  Z.attach(zServo);
  
  Serial.begin(BAUD);

  

  delay(1000);

  Z.write(93);
  home(20);

  delay(1000);

  Serial.println("<ready>");
}

// someone elses
float parseNumber(char code,float val) {
  char *ptr=buffer;  // start at the beginning of buffer
  while((long)ptr > 1 && (*ptr) && (long)ptr < (long)buffer+sofar) {  // walk to the end
    if(*ptr==code) {  // if you find code on your walk,
      return atof(ptr+1);  // convert the digits that follow into a float and return it
    }
    ptr=strchr(ptr,' ')+1;  // take a step from here to the letter after the next space
  }
  return val;  // end reached, nothing found, return default val.
}

void processCommand() {
  int cmd = parseNumber('G',-1);
  switch(cmd) {
  case  0:
  case  1: { // line
    
    if ( parseNumber('Z',0 ) < 0 ) Z.write(133); // pen down
    else Z.write(93); // pen up
    
    line( parseNumber('X',(mode_abs?px:0)) + (mode_abs?0:px),
          parseNumber('Y',(mode_abs?py:0)) + (mode_abs?0:py) );
    
    }
  case 2: {
    float I = parseNumber('I',(mode_abs?px:0)) + (mode_abs?0:px);
    float J = parseNumber('J',(mode_abs?py:0)) + (mode_abs?0:py);
    float tX = parseNumber('X',(mode_abs?px:0)) + (mode_abs?0:px);
    float tY = parseNumber('Y',(mode_abs?py:0)) + (mode_abs?0:py);
    /*
    Serial.println(I);
    Serial.println(J);
    Serial.println(tX);
    Serial.println(tY);
    Serial.println(); */
    arc(I,
        J,
        tX,
        tY,
          ARC_CW);
      break;
  }
  case 3: {  // arc
    float I = parseNumber('I',(mode_abs?px:0)) + (mode_abs?0:px);
    float J = parseNumber('J',(mode_abs?py:0)) + (mode_abs?0:py);
    float tX = parseNumber('X',(mode_abs?px:0)) + (mode_abs?0:px);
    float tY = parseNumber('Y',(mode_abs?py:0)) + (mode_abs?0:py);
    /*
    Serial.println(I);
    Serial.println(J);
    Serial.println(tX);
    Serial.println(tY);
    Serial.println(); */
    arc(I,
        J,
        tX,
        tY,
          ARC_CCW);
      break;
    }
  case  4:  pause(parseNumber('P',0)*1000);  break;  // dwell
  case 90:  mode_abs=1;  break;  // absolute mode
  case 91:  mode_abs=0;  break;  // relative mode
  case 92:  // set logical position
    position( parseNumber('X',0),
              parseNumber('Y',0) );
    break;
  default:  break;
  }

  cmd = parseNumber('M',-1);
  switch(cmd) {
  case 17: // enable motors
    digitalWrite(enable, LOW);
    break;
  case 18:  // disable motors
    digitalWrite(enable, HIGH);
    break;
  case 100:  commands();  break;
  case 114:  where();  break;
  default:  break;
  }
}

void ready() {
  sofar=0;  // clear input buffer
  Serial.println("<ready>");
}
  
void loop() {
  // others code to listen for serial commands
  
  while(Serial.available() > 0) {  // if something is available
    char c=Serial.read();  // get it
    Serial.print(c);  // repeat it back so I know you got the message
    if(sofar<MAX_BUF-1) buffer[sofar++]=c;  // store it
    if((c=='\n') || (c == '\r')) {
      // entire message received
      buffer[sofar]=0;  // end the buffer so string functions work right
      //Serial.print(F("\r\n"));  // echo a return character for humans
      processCommand();  // do something with the command
      ready();
      delay(100);
    }
  }
  
  // z goes from 93 at top to 133 at bottom
  
  
}
