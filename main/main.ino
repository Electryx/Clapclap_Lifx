/*  Clapclap_Lifx v0.5
 *  
 *  Detect number of claps with some filters to avoid random detection problems
 *  
 *  Changelog : Optimized threshold adjustement
 *  
 *  TBD : Add wireless communication
 *        Debug the debug mode
 *  
 *  Components : Arduino Nano || Microphone KY-038 || Buzzer || Blue LED
 *  
 *  Date : 04 January 2017
 *  by Florent Dupont
 */


// --------------- Declarations ---------------
int led = 13;
int analogMic = A0;
int buzzer = 6;


// Stuff
#define detectionTimeLimit 500      // Time before stopping between-clap listening
#define clapDurationLimit 1000      // Time before stopping between-clap listening
bool overtime = false;

#define DEBUGMODE false             // WARNING : setting to true cause serial communication to slow algorithm and breaks detection

int n = 0;

int savedValues[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int count = 0;
int mean = 0;
int threshold = 1000;               // instantiate to unreachable value while mean is not ready
// --------------------------------------------


// -------------- Initialization --------------
void setup()
{
  // Initializing Serial
  Serial.begin(9600);

  // Setting up I/O
  pinMode(analogMic, INPUT);
  pinMode(led, OUTPUT);

  pinMode(buzzer, OUTPUT);

  printlnMessage("--------------------- Start ---------------------");
}
// --------------------------------------------


// --------------- Loop function --------------
void loop()
{
  if(clapClean())         // if clap is recognized
  {
    n = clapCounter(1);   // begin clap count

    printMessage("Valeur de n : ");
    printlnMessage(n);
  }

  
  if(!overtime)       // if no overtime detection
  {
    if(n == 3 && n < 5)
    {
      buzzOk();

      for(n ; n>0 ; n--)    // LED blinking
      {
        digitalWrite(led, HIGH);
        delay(100);
        digitalWrite(led, LOW);
        delay(100);
      }
    }
    else if(n >= 5)
      buzzNotOk();
  }
  else                // if overtime detection
  {
    printlnMessage("OVERTIME !");
    buzzNotOk();
  }

  // reset
  overtime = false;
  n = 0;
}
// --------------------------------------------


// -------------- Methods library -------------
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
  tone(buzzer, 500, 100);
  delay(200);
  tone(buzzer, 1000, 200);
  delay(400);
}

void buzzNotOk()
{
  digitalWrite(led, HIGH);
  tone(buzzer, 300, 100);
  delay(200);
  tone(buzzer, 300, 100);
  delay(200);
  tone(buzzer, 300, 100);
  delay(200);
  digitalWrite(led, LOW);
}


int clapClean()
{
  if(getMicroValue())
  {
    double beginTime = millis();      // detection time
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
  if (savedValues[9] == -1)                 // while array isn't full, can't reach threshold
  {
    savedValues[count] = analogRead(A0);
    mean += savedValues[count];
  }
  else
  {
    mean -= savedValues[count];             // switch oldest value with the new one
    savedValues[count] = analogRead(A0);
    mean += savedValues[count];

    printMessage("Threshold = ");
    printlnMessage(threshold);
    threshold = (mean/10) + 2;               // threshold refresh
  }

  if(count < 9)
    count++;
  else
    count = 0;
}



void printMessage(String message)
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
