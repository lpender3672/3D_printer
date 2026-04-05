const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;

char messageFromPC[buffSize] = {0};

int yval;
int xval;

bool charInString(char cha, char* str) {

  for (int i = 0; i < strlen(str); i++) {
     if (str[i] == cha) {
      return true;
     }
  }
  return false;
}


bool getDataFromPC() {

    // receive data from PC and save it into inputBuffer
    
  if (Serial.available() > 0) {

    char x = Serial.read();

      // the order of these IF clauses is significant
      
    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();
      return true;
    }
    
    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) { 
      bytesRecvd = 0; 
      readInProgress = true;
    }
    return false;
  }
}


 
void parseData() {

    // split the data into its parts
    
  char * strtokIndx; // this is used by strtok() as an index
  
  strtokIndx = strtok(inputBuffer,",");      // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC

  int i, count;
  for (i=0, count=0; strtokIndx[i]; i++) count += (strtokIndx[i] == ',');

  
  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  yval = atoi(strtokIndx);     // convert this part to an integer
  
  strtokIndx = strtok(NULL, ","); 
  xval = atoi(strtokIndx);  


  xval = count;
}







void setup() {
 Serial.begin(115200);
 Serial.setTimeout(1);

 Serial.println("<ready>");
}

void loop() {
 while (!getDataFromPC());

 if (newDataFromPC) {

    newDataFromPC = false;


    
    Serial.print("<");
    Serial.print(charInString("e","hello"));
    Serial.print(">");
 }
 

}
