#include "OneWire/OneWire.h"


char pubString[40];

int led = D7; 
int dsbPort = D0;

int periodSeconds = 30;

OneWire ds(dsbPort);

void setup() {
  pinMode(led, OUTPUT);
}

float readCelcius()
{
     byte i; 
     byte present = 0; 
     byte type_s; 
     byte data[12]; 
     byte addr[8]; 
     float celsius; 
    
     if ( !ds.search(addr)) 
     { 
       ds.reset_search(); 
       delay(250); 
       return -99; 
     } 
 
     if (OneWire::crc8(addr, 7) != addr[7]) 
     { 
         return -99; 
     } 

   switch (addr[0]) { 
     case 0x10: 
       Serial.println("  Chip = DS18S20");  // or old DS1820 
       type_s = 1; 
       break; 
     case 0x28: 
       Serial.println("  Chip = DS18B20"); 
       type_s = 0; 
       break; 
     case 0x22: 
       Serial.println("  Chip = DS1822"); 
       type_s = 0; 
       break; 
     case 0x26: 
       Serial.println("  Chip = DS2438"); 
       type_s = 2; 
       break; 
     default: 
       Serial.println("Device is not a DS18x20/DS1822/DS2438 device. Skipping..."); 
       return -99; 
   }  
 
   ds.reset(); 
   ds.select(addr);      // Just do one at a time for testing 
                         // change to skip if you already have a list of addresses 
                         // then loop through them below for reading 
                          
   ds.write(0x44);        // start conversion, with parasite power on at the end 
    
   delay(900);     // maybe 750ms is enough, maybe not, I'm shooting for 1 reading per second 
                     // prob should set to min reliable and check the timer for 1 second intervals 
                     // but that's a little fancy for test code 
                      
   // we might do a ds.depower() here, but the reset will take care of it. 
    
   present = ds.reset(); 
   ds.select(addr);     
   ds.write(0xBE, 0);         // Read Scratchpad 0 
 
   Serial.print("  Data = "); 
   Serial.print(present, HEX); 
   Serial.print(" "); 
   for ( i = 0; i < 9; i++) {           // we need 9 bytes 
     data[i] = ds.read(); 
     Serial.print(data[i], HEX); 
     Serial.print(" "); 
   } 
   Serial.print(" CRC="); 
   Serial.print(OneWire::crc8(data, 8), HEX); 
   Serial.println(); 
 
   // Convert the data to actual temperature 
   // because the result is a 16 bit signed integer, it should 
   // be stored to an "int16_t" type, which is always 16 bits 
   // even when compiled on a 32 bit processor. 
   int16_t raw = (data[1] << 8) | data[0]; 
   if (type_s) { 
     if (type_s==1) {    // DS18S20 
       raw = raw << 3; // 9 bit resolution default 
       if (data[7] == 0x10) { 
         // "count remain" gives full 12 bit resolution 
         raw = (raw & 0xFFF0) + 12 - data[6]; 
       } 
       celsius = (float)raw / 16.0; 
     }else{ // type_s==2 for DS2438 
       if (data[2] > 127) data[2]=0; 
       data[1] = data[1] >> 3; 
       celsius = (float)data[2] + ((float)data[1] * .03125); 
     } 
   } else {  // DS18B20 and DS1822 
     byte cfg = (data[4] & 0x60); 
     // at lower res, the low bits are undefined, so let's zero them 
     if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms 
     else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms 
     else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms 
     //// default is 12 bit resolution, 750 ms conversion time 
     celsius = (float)raw / 16.0; 
   } 
    return celsius;//2.0;
}

void blink(int times)
{
   if (times < 0)
      times = 1;
      
   for (int i = 0; i < times; i++)
   {
      digitalWrite(led, HIGH);
      delay(300);
      digitalWrite(led, LOW);
      delay(300); 
   }
}

void publish(float temp)
{
  sprintf(pubString, "%f", temp);
  Spark.publish("Temp_Office", pubString);
}

void wait()
{
  for (int i = 0; i < periodSeconds; i++)
  { 
      delay(1000);
  }   
}

float readCelciusNTimes(int times)
{
    float cels = -99;
    for (int i = 0; i < times; ++i)
       {
           cels = readCelcius();
           if (cels == -99)
              delay(3000);
            else
              return cels;
       }
}

void loop() {
  float tempC = readCelciusNTimes(5);
  
  publish(tempC);
  blink( (int)(tempC) );
 
  wait();
}

