int led = D7;


void yeah(const char *event, const char *data) 
{
   // Blinking skjer her.
}



void setup()
{
   pinMode(led, OUTPUT);
   
   Particle.subscribe("yeah", yeah, MY_DEVICES);
   digitalWrite(led, LOW);
}


void loop()
{
   //ledToggle(0, 0);
}


