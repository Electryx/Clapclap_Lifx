int led = 3;

int digitalMic = 2;
int analogMic = A0;

bool mode = false;
int intensity = 75;

void setup() 
{
  Serial.begin(9600);
  pinMode(analogMic, INPUT);
  pinMode(digitalMic, INPUT);
  pinMode(led, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(2), interruptFunction, RISING);
  //attachInterrupt(digitalPinToInterrupt(2), interruptFunction, LOW);
}


void loop()
{
    if(mode)
    {
      mode = 0;
      long beginTime = millis();
    
      while(intensity >= 0)
      {
        analogWrite(led, intensity);
        intensity--;
        asyncDelay(5);
      }
    }
}

void interruptFunction()
{
  Serial.println("signal !");
  intensity = 75;
  analogWrite(led, intensity);
  mode = true;
}

void asyncDelay(long msDelay)
{
  long beginTimeAsync = millis();
  while(millis() - beginTimeAsync <= msDelay)
  {}
}

