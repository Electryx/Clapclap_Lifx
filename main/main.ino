/*  Clapclap_Lifx v0.4
 *  
 *  Detect number of claps with some filters to avoid random detection problems
 *  
 *  Changelog : Adjust threshold in runtime (don't need to change microphone potentiometer anymore)
 *  
 *  TBD : Add wireless communication
 *  
 *  Components : Arduino Nano || Microphone KY-038 || Buzzer || Blue LED
 *  
 *  Date : 09 November 2016
 *  by Florent Dupont
 */


// --------------- Declarations ---------------
int led = 3;

int digitalMic = 2;
int analogMic = A0;   // usefull !

int buzzer = 6;



// Stuff
double detectionTimeLimit = 500;    // Time before stopping between-clap listening
double clapDurationLimit = 1000;    // Time before stopping between-clap listening
bool overtime = false;

bool DEBUGMODE = true;

int n = 0;

int savedValues[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int threshold = 1000;               // instantiate to unreachable value while mean is not ready
// --------------------------------------------


// -------------- Initialization --------------
void setup()
{
  // Initializing Serial
  Serial.begin(115200);

  // Setting up I/O
  pinMode(analogMic, INPUT);
  pinMode(digitalMic, INPUT);
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
        analogWrite(led, 90);
        delay(100);
        analogWrite(led, 0);
        delay(100);
      }
    }
    else if(n >= 5)
    {
      buzzNotOk();
    }
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


// --------------- Function library ---------------
int clapCounter(int m)
{
  printlnMessage(m);
  double detectionTime = millis();
  while(millis() - detectionTime < detectionTimeLimit)
  {
    if(clapClean())
    {
      m++;
      if(m >= 5)
        return m;
      m = clapCounter(m);
    }
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
  analogWrite(led, 90);
  tone(buzzer, 300, 100);
  delay(200);
  tone(buzzer, 300, 100);
  delay(200);
  tone(buzzer, 300, 100);
  delay(200);
  analogWrite(led, 0);
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
  int mean = 0;                         // mean reset
  mean += savedValues[0];               // oldest value save
  for(int i=1 ; i<10 ; i++)
  {
    savedValues[i-1] = savedValues[i];  // list shift
    mean += savedValues[i];             // add newest values to mean
  }
  mean = mean/10;                       // calculate mean
  savedValues[9] = analogRead(A0);      // save newest value at array's last position
  
  if(savedValues[0] != -1)              // if 10 values or more are saved, begin threshold algorithm
  {
    printMessage("Threshold = ");
    printlnMessage(threshold);
    threshold = mean + 2;               // threshold refresh
  }
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
