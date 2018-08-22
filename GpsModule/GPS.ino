#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 7, TXPin = 6;
static const uint32_t GPSBaud = 9600;
// The TinyGPS++ object
TinyGPSPlus gps;

double latitude=0.0;
double longitude=0.0;

 double latitude_old_state=0.0;
 double longitude_old_state=0.0;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(GPSBaud);
}
void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
      displayInfo();
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}
void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    latitude=gps.location.lat();
    longitude=gps.location.lng();
    
    if(longitude_old_state!=longitude || latitude_old_state!=latitude){

         //float course=gps.course_to(latitude,longitude,latitude_old_state,longitude_old_state);
         //float distance=gps.location.distance(latitude,longitude,latitude_old_state,longitude_old_state);

         Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
         Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
         Serial.print("ALT=");  Serial.println(gps.altitude.meters());
         //Serial.print ("course is ");
         //Serial.println (course);
         //Serial.print("Distance is: ");
         //Serial.println(distance);
         
        latitude_old_state=latitude;
        longitude_old_state=longitude;
      }
  }
  else
  {
    Serial.print(F("INVALID"));
    
  }  
  Serial.println();
}
