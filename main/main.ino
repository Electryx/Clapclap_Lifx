/*  Clapclap_Lifx v0.2
 *  
 *  Detect number of claps with some filters to avoid random detection problems
 *  
 *  Changelog : Give up interrupts method because of too much strange problems
 *              Re-written with common methods, works well
 *  
 *  TBD : Better filters
 *  
 *  Components : Microphone KY-038 || Buzzer || Blue LED (cause it is classy)
 *  
 *  Date : 10 June 2016
 *  by Florent Dupont
 */


// Pins assignation
int led = 3;

int digitalMic = 2;
int analogMic = A0;   // useless ?

int buzzer = 6;



// Stuff
double detectionTimeLimit = 500;    // Time before stopping between-clap listening
double clapDurationLimit = 1000;    // Time before stopping between-clap listening
bool overtime = false;

int n = 0;


// --------------- Initialization ---------------
void setup()
{
  // Initializing Serial
  Serial.begin(9600);

  // Setting up I/O
  pinMode(analogMic, INPUT);
  pinMode(digitalMic, INPUT);
  pinMode(led, OUTPUT);

  pinMode(buzzer, OUTPUT);

  Serial.println("--------------------- Debut du programme ---------------------");
}


// --------------- Loop function ---------------
void loop()
{
  
  if(clapClean())
  {
    n = clapCounter(1);

    Serial.print("Valeur de n : ");
    Serial.println(n);
  }

  if(!overtime)
  {
    if(n == 2)
    {
      buzzOk();
    }

    for(n ; n>0 ; n--)
    {
      analogWrite(led, 90);
      delay(100);
      analogWrite(led, 0);
      delay(100);
    }
  }
  else
  {
    Serial.println("OVERTIME !");
    analogWrite(led, 90);
    delay(clapDurationLimit);
    analogWrite(led, 0);
  }

  // reset
  overtime = false;
  n == 0;
}


// --------------- Other functions ---------------
int clapCounter(int m)
{
  Serial.println(m);
  double detectionTime = millis();
  while(millis() - detectionTime < detectionTimeLimit)
  {
    if(clapClean())
    {
      m++;
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


int clapClean()
{
  if(digitalRead(2))
  {
    double beginTime = millis();  // temps de détection
    int mode = 1;
    while(mode == 1)
    {
      double beginTimeInside = millis();  
      while(millis() - beginTimeInside <= 50)
      {
        if(millis() - beginTime > clapDurationLimit)
        {
          mode = 0;
          overtime = true;
          return false;
        }
        if(digitalRead(2))
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
