/*  Clapclap_Lifx v0.3
 *  
 *  Detect number of claps with some filters to avoid random detection problems
 *  
 *  Changelog : Limit claps number to 5 to avoid constant detection bug
 *              Finally use analogic output for better detection stability
 *              Add Debug mode with Serial debugging messages, possibility to disable it
 *  
 *  TBD : Add wireless communication
 *  
 *  Components : Microphone KY-038 || Buzzer || Blue LED (cause it is classy)
 *  
 *  Date : 13 October 2016
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

  printlnMessage("--------------------- Debut du programme ---------------------");
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
    double beginTime = millis();      // temps de détection
    int mode = 1;
    while(mode == 1)
    {
      double beginTimeInside = millis();  
      
      // overtime detection algorithm
      while(millis() - beginTimeInside <= 50)           // while sound is detected within 50 milliseconds
      {
        if(millis() - beginTime > clapDurationLimit)    // if it is during for more than 500 milliseconds, stop clap detection
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
  return (analogRead(A0) > 545);
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
