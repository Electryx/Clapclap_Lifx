/*  Clapclap_Lifx v1.1
 *  
 *  Detect number of claps to turn on/off a Lifx bulb
 *  
 *  Changelog : Add brighness options with 3/4/5 claps
 *              Code improvements
 *  
 *  TBD : Debug the debug mode
 *        Get time when ESP is wireless ready instead of waiting 5 seconds
 *        Flash ESP to set at 9600 bauds
 *  
 *  Components :  Arduino Nano || Breadboard Power Supply || ESP8266 ESP-01 || Microphone KY-038
 *                Voltage regulator 5V-3.3V || 2 PNP transistors || Buzzer || Blue LED
 *  
 *  Date : 11 January 2017
 *  by Florent Dupont
 */


#include <SoftwareSerial.h>


// ------------------------------ Declarations ------------------------------
#define DEBUGMODE false                  // WARNING : setting to true cause serial communication to slow algorithm and breaks detection
#define NOISYMODE false                  // to turn on/off buzzer sound

int led = 13;
int analogMic = A0;
int digitalMic = 12;
int buzzer = 9;

SoftwareSerial ESP(2, 3);               // initialize serial communication through pin 2/RX and 3/TX
#define baudrate 9600                   // SoftwareSerial is stable only at 9600 bauds on serial communication
int resetPin = 5;
bool bulbState = false;                 // bulb have to be set OFF when initializing soft

#define detectionTimeLimit 500          // Time before stopping between-clap listening
#define clapDurationLimit 1000          // Time before stopping between-clap listening
bool overtime = false;

int n = 0;

int savedValues[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int count = 0;
int mean = 0;
int threshold = 1025;                   // instantiate to unreachable value while mean is not ready

// ASCII messages
const char power1[] = {49, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0, 255, 63, 172, 13, 0, 4, 0, 0};
const char power2[] = {49, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0, 255, 127, 172, 13, 0, 4, 0, 0};
const char power3[] = {49, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 102, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 172, 13, 0, 4, 0, 0};

const char turnON[] = {42, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 117, 0, 0, 0, 0, 255, 255, 4, 0, 0};
const char turnOFF[] = {42, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 117, 0, 0, 0, 0, 0, 0, 4, 0, 0};
// --------------------------------------------------------------------------


// ----------------------------- Initialization -----------------------------
void setup()
{
  if(DEBUGMODE) Serial.begin(115200);   // start serial communications at 115200 bauds, required because ESP restarts at 115200 bauds
  ESP.begin(115200);

  // Setting up I/O
  pinMode(resetPin, OUTPUT);            // controls ESP reset
  pinMode(led, OUTPUT);                 // indicative LED
  pinMode(analogMic, INPUT);
  pinMode(digitalMic, INPUT);
  pinMode(buzzer, OUTPUT);
  
  rebootESP();
  sendToESP("AT+CIOBAUD=9600", 1000);   // lower ESP serial communication baudrate for better stability, SoftwareSerial doesn't support 115200 bauds
  
  // change baudrates from 115200 to 9600 bauds
  ESP.flush();
  ESP.end();
  if(DEBUGMODE)
  {
    Serial.flush();                     // let last character be sent
    Serial.end ();                      // close serial
    Serial.begin(baudrate);
  }
  ESP.begin(baudrate);

  // AT Commands
  sendToESP("AT+CIPMUX=0", 1000);
  
  sendToESP("AT+CIPSTART=\"UDP\",\"192.168.1.16\",56700", 1000);    // create UDP communication with IP adress and port
  
  printlnMessage("ESP ready");
}
// --------------------------------------------------------------------------


// ------------------------------ Loop function -----------------------------
void loop()
{
  if(DEBUGMODE)
  {
    // listen for communication from the ESP8266 and then write it to the serial monitor
    if (ESP.available())
      Serial.write( ESP.read() );
  
    // listen for user input and send it to the ESP8266
    if (Serial.available())
      ESP.write( Serial.read() );
  }

    
  if(clapClean())         // if clap is recognized
  {
    n = clapCounter(1);   // begin clap count

    printMessage("Valeur de n : ");
    printlnMessage(n);
  }

  
  if(!overtime)           // if no overtime detection
  {
    switch(n)
    {
      case 2 :  executeOrder66();
                buzzOk();
                break;
      case 3 :  changeBrightness(1);
                buzzOk();
                break;
      case 4 :  changeBrightness(2);
                buzzOk();
                break;
      case 5 :  changeBrightness(3);
                buzzOk();
                break;
    }
    if(n > 5)
      buzzNotOk();
  }
  else                    // if overtime detection
  {
    printlnMessage("OVERTIME !");
    buzzNotOk();
  }

  // Microphone analysis reset
  overtime = false;
  n = 0;
}
// --------------------------------------------------------------------------


// ----------------------------- Methods library ----------------------------
int clapCounter(int m)
{
  printlnMessage(m);
  double detectionTime = millis();
  while(millis() - detectionTime < detectionTimeLimit)
    if(clapClean())
    {
      m++;
      if(m >= 5)
        return m;
      m = clapCounter(m);
    }
  return m;
}


void buzzOk()
{
  if(NOISYMODE)
  {
    tone(buzzer, 500, 50);
    delay(100);
    tone(buzzer, 1000, 100);
    delay(200);
  }
}

void buzzNotOk()
{
  bool bulbStateMem = (digitalRead(led) ? HIGH : LOW);    // save bulb state

  for(int i=0 ; i<4 ; i++)
  {
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    delay(50);
  }
  
  digitalWrite(led, (bulbStateMem ? HIGH : LOW));         // set back lifx led state
  
  if(NOISYMODE)
  {
    tone(buzzer, 300, 50);
    delay(100);
    tone(buzzer, 300, 50);
    delay(100);
    tone(buzzer, 300, 50);
    delay(100);
  }
}


int clapClean()
{
  if(getMicroValue())
  {
    double beginTime = millis();        // detection time
    int mode = 1;
    while(mode == 1)
    {
      double beginTimeInside = millis();  
      
      // overtime detection algorithm
      while(millis() - beginTimeInside <= 50)           // while sound is detected within 50 milliseconds
      {
        if(millis() - beginTime > clapDurationLimit)    // if sound last for more than 1000 milliseconds, stop clap detection
        {
          mode = 0;
          overtime = true;
          return false;
        }
        if(getMicroValue())
        {
          mode = 1;
          break;
        }
        mode = 0;
      }
    }
    return true;
  }
  else
    return false;
}


int getMicroValue()
{
  printlnMessage(analogRead(A0));

  adjustThreshold();

  return (analogRead(A0) > threshold);
}


void adjustThreshold()
{
  if (savedValues[9] == -1)             // while array isn't full, can't reach threshold
  {
    savedValues[count] = analogRead(A0);
    mean += savedValues[count];
  }
  else
  {
    mean -= savedValues[count];         // switch oldest value with the new one
    savedValues[count] = analogRead(A0);
    mean += savedValues[count];

    printMessage("Threshold = ");
    printlnMessage(threshold);
    threshold = (mean/10) + 2;          // threshold refresh
  }

  if(count < 9)
    count++;
  else
    count = 0;
}


void rebootESP()
{
  digitalWrite(5, LOW);                 // put ESP reset pin to 0V
  digitalWrite(13, LOW);
  delay(1000);
  digitalWrite(5, HIGH);                // reset ESP8266
  digitalWrite(13, HIGH);
  
  double timein = millis();
  while(millis() - timein < 5000)       // await ESP serial emptying during 5 sec
    while(ESP.available() > 0) 
      char trash = ESP.read();          // empty ESP serial buffer
  digitalWrite(13, LOW);
  
  char message[] = {'O','K'};
  char n = 0;
  ESP.println("AT");                    // send "AT" to ESP, need to answer "OK"
  
  timein = millis();
  while(millis() - timein < 1000)
    if (ESP.available())
      if(ESP.read() == message[n])
        if(n == 0)
          n++;
        else if(n == 1)
        {
          printlnMessage("ESP boot completed");
          return;                       // if completed, leave recursive function
        }
      else
        n = 0;
  
  printlnMessage("ESP boot failed");    // if ESP doesn't send "OK", retry reboot process until it sends OK
  rebootESP();
}


void sendToESP(String command, int timeout)
{
  ESP.println(command);
  char check[] = {'O','K'};
  char n = 0;
  char answer;
  double timein = millis();
  while(millis() - timein < timeout)
    if(ESP.available())
    {
      answer = ESP.read();
      printMessage(answer);
      if(answer == check[n])
        if(n == 0)
          n++;
        else if(n == 1)
        {
          printMessage("\r\n");
          delay(5);
          return;
        }
      else
        n = 0;
    }
}


void waitESPReaction(int timeout)
{
  double timein = millis();
  while(millis() - timein < timeout)
  if(DEBUGMODE)
    if(ESP.available())
      Serial.write( ESP.read() );
}


void waitESPAcknowledgement(short timeout)
{
  char check[] = {'O','K'};
  char n = 0;
  char answer;
  double timein = millis();
  while(millis() - timein < timeout)
    if(ESP.available())
    {
      answer = ESP.read();
      printMessage(answer);
      if(answer == check[n])
        if(n == 0)
          n++;
        else if(n == 1)
        {
          printMessage("\r\n");
          delay(5);
          return;
        }
      else
        n = 0;
    }
}


void changeBrightness(char power)
{
  sendToESP("AT+CIPSEND=49", 1000);

  switch(power)
  {
    case 1: for(int i=0 ; i<sizeof(power1) ; i++)
              ESP.print(power1[i]);
            break;
    case 2: for(int i=0 ; i<sizeof(power2) ; i++)
              ESP.print(power2[i]);
            break;
    case 3: for(int i=0 ; i<sizeof(power3) ; i++)
              ESP.print(power3[i]);
            break;
  }
  waitESPAcknowledgement(1024);
    
  if(!bulbState)        // if bulb is OFF, turn it ON
    executeOrder66();   
}


void executeOrder66()
{
  sendToESP("AT+CIPSEND=42", 1000);

  if(!bulbState)
  {
    for(int i=0 ; i<sizeof(turnON) ; i++)
      ESP.print(turnON[i]);
    bulbState = true;
    digitalWrite(led, HIGH);
  }
  else
  {
    for(int i = 0 ; i < sizeof(turnOFF) ; i++)
      ESP.print(turnOFF[i]);
    bulbState = false;
    digitalWrite(led, LOW);
  }
  waitESPAcknowledgement(1024);
}



void printMessage(String message)
{
  if(DEBUGMODE)
    Serial.print(message);
}

void printMessage(char message)
{
  if(DEBUGMODE)
    Serial.print(message);
}

void printlnMessage(String message)
{
  if(DEBUGMODE)
    Serial.println(message);
}

void printlnMessage(int message)
{
  if(DEBUGMODE)
    Serial.println(message);
}
