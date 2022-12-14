//Illegal logging and fire Detection and  monitoring system with the New Blynk app
  
//Include the library files
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#define Buzzer 9
#define relay D3
#define flame D0
#define tilt D4
 

/* -------------------------------------------------
   ------------------------------------------------*/
   
char auth[] = "ExsajpgP49skJFL_jkQNkZGM4g0OkK9t";//Enter your Blynk Auth token
char ssid[] = "PAVAN";//Enter your WIFI name
char pass[] = "12345678";//Enter your WIFI password

//**************************************************************

int Analog_In = A0; // Analog output of the sensor
int Digital_Input= D2;

char status;
WiFiClient client;
float t,h,db;
const int sampleWindow = 50;  
String apiKey = "K2YMB91MW2OZUBB0"; //ThingSpeak Write API Key
const char* server = "api.thingspeak.com";//Thingspeak API

unsigned int sample;
DHT dht(D1, DHT11);//(DHT sensor pin,sensor type)
BlynkTimer timer;


void setup() {
  Serial.begin(9600);
  pinMode (Analog_In, INPUT);
  pinMode (Digital_Input, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(flame, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();
  
 //Call the functions
 timer.setInterval(100L,Sound);
 timer.setInterval(100L, DHT11sensor);
 timer.setInterval(100L, flamesensor);
 timer.setInterval(100L, Tiltsensor);
}

void Sound()
{
  float  Analog;
  int Digital;
  unsigned int sample;
  const int sampleWindow = 50;  
  unsigned long startMillis= millis();                   // Start of sample window
  float peakToPeak = 0;                                  // peak-to-peak level
 
  unsigned int signalMax = 0;                            //minimum value
  unsigned int signalMin = 1024;                         //maximum value
 
                                                        // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);                             //get reading from microphone
      if (sample < 1024)                                  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;                    // max - min = peak-peak amplitude
   db = map(peakToPeak,20,900,49.5,90);             //calibrate for deciBels
   Blynk.virtualWrite(V2, db);                                                     //set text size to 2

                                                        
  //Current values are read out, converted to the voltage value...
  Analog =  analogRead (Analog_In)   *  (5.0 / 1023.0); 
  Digital = digitalRead (Digital_Input) ;
    
  //...  and issued at this point
  //Serial.print  ("Analog voltage value:");  Serial.print (Analog,  4) ;   Serial.print  ("V, ");
  //Serial.print ("Limit value:") ;
  int tcount = 0;
  int count = 0;
  while(tcount<10)
  {
  
    if  (count>=2) 
    {
        Serial.println("-----------------------------------------------------------------");
        Serial.print  ("Analog voltage value:");  Serial.print (Analog,  4) ;   Serial.print  ("V, ");
        Serial.print ("Limit value:") ;
        Serial.println (" reached ");
        
        Serial.print(db);                              
        Serial.println(" dB"); 
        Serial.println(signalMax);
        Serial.println(signalMin);
        Serial.println(peakToPeak);
        Blynk.logEvent("sound", "Warning! Chainsaw Sound detected");
        Blynk.email("bgm00ind@gmail.com", "Code Malli GREEN", "ChainSaw Sound Detected Click here https://thingspeak.com/channels/1769216");
        digitalWrite(Buzzer, HIGH);
        tone(Buzzer, 1000, 200);
    }
  
    if  (Analog>=1.9500) 
    {
        count++;
    }
    tcount++;
    delay(400);
  }
}


//Get the DHT11  Temperature and Humidity sensor values
void DHT11sensor() {
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
}

//Smoke Sensor


//Fire Sensor
void flamesensor() {
  bool value = digitalRead(flame );
    if (value == 0) 
    {
    Blynk.email("bgm00ind@gmail.com", "Code Malli RED", "Fire Detected Click here https://thingspeak.com/channels/1769216");
    Blynk.logEvent("fire", "Warning! Fire was detected");
    digitalWrite(relay, LOW);
    digitalWrite(Buzzer, HIGH);
    tone(Buzzer, 1000, 200);
    }
    else
    { 
      noTone(Buzzer);
    }
}

void Tiltsensor() {
  bool value = digitalRead(tilt);
    if (value == 0) 
    {
    Blynk.logEvent("fall", "Warning! Tree has Fallen");
    Blynk.email("bgm00ind@gmail.com", "Code Malli FALL", "Tree has Fallen Click here https://thingspeak.com/channels/1769216");
    digitalWrite(Buzzer, HIGH);
    tone(Buzzer, 1000, 200);
    }
    else
    { 
      noTone(Buzzer);
    }
}

//Get buttons values
BLYNK_WRITE(V3) {
 bool RelayOne = param.asInt();
  if (RelayOne == 1) {
    digitalWrite(relay, LOW);
  } else {
    digitalWrite(relay, HIGH);
  }
}

void loop() {
  //...............
  Blynk.run();//Run the Blynk library
  timer.run();//Run the Blynk timer
   if (client.connect(server, 80)) 
   {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(db);
    postStr += "\r\n\r\n\r\n\r\n\r\n";
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n\n\n\n");
    client.print(postStr);
    Serial.print("Temperature: ");
    Serial.println(t);
    Serial.print("Humidity: ");
    Serial.println(h);
  }
  client.stop();
  delay(1000);
}
