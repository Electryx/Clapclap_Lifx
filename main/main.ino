/*  Clapclap_Lifx v0.1
 *  
 *  Detect one or two claps, do something if two claps are heard by microphone
 *  
 *  TBD : Filter if more than two claps
 *        Improve detection, gap between claps
 *  
 *  Components : Microphone KY-038 || Blue LED (cause it is classy)
 *  
 *  Date : 30 May 2016
 *  by Florent Dupont
 */


// Pins assignation
int led = 3;

int digitalMic = 2;
int analogMic = A0;


// Stuff
int intensity = 75;

bool activation1 = false;
bool activation2 = false;
bool acknowledgement = false;


// ---------------------------- SETUP ----------------------------
void setup() 
{
  // Initializing Serial
  Serial.begin(9600);

  // Setting up I/O
  pinMode(analogMic, INPUT);
  pinMode(digitalMic, INPUT);
  pinMode(led, OUTPUT);

  // Inizializing interrupts
  attachInterrupt(digitalPinToInterrupt(2), initializingInterrupt2, RISING);
  attachInterrupt(digitalPinToInterrupt(2), initializingInterrupt1, RISING);

  Serial.println("--------------------- Debut du programme ---------------------");
}


// ---------------------------- LOOP ----------------------------
void loop()
{
  // If first clap
  if(activation1 == true)
  {
    activation1 = false;
    Serial.println("1er signal !");         // ---TBR---
    long beginTime = millis();
    Serial.println("delay ...");            // ---TBR---
    asyncDelay(200);
    Serial.println("interrup 2 enabled");   // ---TBR---
    attachInterrupt(digitalPinToInterrupt(2), initializingInterrupt2, RISING);
    //attachInterrupt(digitalPinToInterrupt(2), secondActivation, RISING);

    Serial.println("wait for signal 2..."); // ---TBR---
    long checkPoint = millis();
    while(millis() - checkPoint <= 750)
    {
      // If second clap
      if(activation2 == true)
      {
        activation2 = false;
        Serial.println("2e signal !");      // ---TBR---
  
        for(int i = 0 ; i<3 ; i++)
        {
          intensity = 75;
          analogWrite(led, intensity);  
          long beginTime = millis();
          while(millis() - beginTime <= 500)
          {
            if(intensity >= 0)
            {
              analogWrite(led, intensity);
              intensity--;
              asyncDelay(5);
            }
          }
        }
        intensity = 75;
        analogWrite(led, intensity); 
      }
    }
    Serial.println("interrup 1 enabled");     // ---TBR---
    attachInterrupt(digitalPinToInterrupt(2), initializingInterrupt1, RISING);
    activation1, activation2 = false;
  }


  // Everytime, fade the LED
  while(intensity >= 0)
  {
    analogWrite(led, intensity);
    intensity--;
    asyncDelay(2);
  }
}


// ---------------------------- Interrupts methods ----------------------------
void initializingInterrupt1()
{
  detachInterrupt(digitalPinToInterrupt(2));    // detach both interrupts
  attachInterrupt(digitalPinToInterrupt(2), firstActivation, RISING);
}

void initializingInterrupt2()
{
  detachInterrupt(digitalPinToInterrupt(2));    // detach both interrupts
  attachInterrupt(digitalPinToInterrupt(2), secondActivation, RISING);
}

void firstActivation()
{
  Serial.println("interrup 1 disabled");        // ---TBR---
  detachInterrupt(digitalPinToInterrupt(2));    // detach interrupt 1
  activation1 = true;
  intensity = 125;
  analogWrite(led, intensity);
}

void secondActivation()
{
  Serial.println("interrup 2 disabled");        // ---TBR---
  detachInterrupt(digitalPinToInterrupt(2));    // detach interrupt 2
  activation2 = true;
  intensity = 75;
  analogWrite(led, intensity);  
}


// ---------------------------- Others methods ----------------------------
void asyncDelay(long msDelay)
{
  long beginTimeAsync = millis();
  while(millis() - beginTimeAsync <= msDelay)
  {}
}

