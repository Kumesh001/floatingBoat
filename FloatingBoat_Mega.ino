#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "ds3231.h"
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define BUFF_MAX 128

uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;
unsigned long prev, interval = 30000;    // i.e one minute  
unsigned long prev_1, interval_1 = 30000;

double distanceKm;

double latitude=0.0;
double longitude=0.0;

volatile double latitude_old_state=0.0;
volatile double longitude_old_state=0.0;

volatile uint32_t dateFromGPS;
volatile uint32_t timeFromGPS;
volatile double speedFromGPS;
volatile double altitudeFromGPS;
volatile double courseTo;


int SD_status=-1;
int Internet_status=-1;
int gps_Status=-1;
unsigned int itemCounter=-1;

int turbiditySensor = A0;  //connect cell to analog pin 0 with 10k pull down resistor to ground
int ledPin = 13;           //Connect LED to run pin 13 in series with appropriate resistor
int pinCS = 53;            // Chipselect pin

static const uint32_t GPSBaud = 9600;
static const uint32_t GSMBaud = 19200;
volatile TinyGPSPlus gps;
volatile File myFile;

// time Variables
struct ts t;
char buff[BUFF_MAX];

double ErrorMessage=0.0;

#define DS1307_I2C_ADDRESS 0x68

void setup()
{
  // Intialising the Serial Communication
  Serial.begin(9600);
  while(!Serial){
    ;
  }
 
  // Intialising the other components on the board
  intialise_gps();
  intialise_gsm(); 
  intialise_clock();
  intialise_sd_card();
  intialise_Sensors();
  //connectToInternet();
}

void intialise_gps(){
   Serial.println("Initializing GPS Module...");
   Serial1.begin(GPSBaud);
   Serial.println("Done");
   delay(2000);
}

void intialise_gsm(){
  Serial.println("Initializing GSM Module...");
  Serial2.begin(GSMBaud);
  Serial2.print("AT");
  Serial2.println();
  ShowSerialData();
  Serial.println("Done");
  delay(2000);
}

void intialise_clock(){
  Serial.println("Initializing RTC Module...");
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);
  Serial.println("Done");
  delay(500);
}

void intialise_sd_card(){
  Serial.println("Initializing SD card Module...");
  pinMode(pinCS,OUTPUT);
  //digitalWrite(pinCS,LOW); 
  if (!SD.begin(pinCS)) {
     Serial.println("SD initialization failed!");
     SD_status=0;
  }else{
    Serial.println("Done.");
    SD_status=1;
    delay(300);
  }
  Serial.print("SD Card Status is: ");
  Serial.println(SD_status);
}

void intialise_Sensors(){
   pinMode (ledPin, OUTPUT);  //Set pin to outut
   pinMode (turbiditySensor, INPUT);
}

void connectToInternet(){
  connectGPRS();
}
void connectGPRS()
{
  Serial2.println("AT+CGATT=1");
  delay(1000);
  ShowSerialData();
  Serial2.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  delay(1000);
  ShowSerialData();
  Serial2.println("AT+SAPBR=3,1,\"APN\",\"www\"");//instead of www use your SIM card APN
  delay(1000);
  ShowSerialData();
  Serial2.println("AT+SAPBR=1,1");
  delay(5000);
  ShowSerialData();
  Serial2.println("AT+SAPBR=2,1");
  delay(5000);
  ShowSerialData();
}

void SendMessage(String message)
{
  Serial.println("Wait Sending the Message");
  Serial2.println("AT+CMGF=1\r");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  Serial2.println("AT+CMGS=\"+919315693639\"\r"); // Replace x with mobile number
  delay(1000);
  Serial2.println(message);// The SMS text you want to send
  delay(100);
  Serial2.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  Serial.println("Sent");
}

void ShowSerialData()
{
  while(Serial2.available()!=0){
    Serial.println("response from the GSM module");
    Serial.write(Serial2.read());
  }
}

int getTurbidityData(){
 float val = analogRead(turbiditySensor);  //create variable to take in analog reading from cell
 
 digitalWrite(13, HIGH);
 float ardval = val*0.00488758553;  //arduino value units 

 float r1 = (50000/ardval)-10000; //R1 value when using Ohm's law
 
 float I = ardval/r1; //value of I which we are solving for
 float NTUval = I*70000;  //200 = units in NTU
 delay(1000);


 Serial.print("NTUval ");
 Serial.println(NTUval);
 delay(1000);
 return NTUval;
}

int getPhValue()
{

  return 0;
}

int displayInfo()
{
  if (gps.location.isValid())
  {
      if(gps.location.isUpdated())
      {
        Serial.print("Location Updated");

        latitude=gps.location.lat();
        longitude=gps.location.lng();
        dateFromGPS=gps.date.value();
        timeFromGPS=gps.time.value();
        speedFromGPS=gps.speed.kmph();
        altitudeFromGPS=gps.altitude.kilometers();

        //courseTo =gps.courseTo(gps.location.lat(),gps.location.lng(),latitude_old_state,longitude_old_state);
        distanceKm =gps.distanceBetween(gps.location.lat(),gps.location.lng(),latitude_old_state,longitude_old_state) / 1000.0;
        
        // print the location on the screen;
        Serial.print(F("Location: ")); 
        Serial.print(F(" Latitude is: "));
        Serial.print(latitude,6);
        Serial.print(F(" Longitude is: "));
        Serial.println(longitude,6);
        
        if(distanceKm<0.04 && itemCounter<20)  // If the distance between the intial point and the change point is less than 40 mt. then dont update it
        {
          itemCounter++;
          return 1;        // distance is less then the set so save only the 20 entrys
        }else if(distanceKm>=0.04){
          itemCounter=-1;
          return 1;
        }
        itemCounter=-1;                  // Here we will trigger our motors to trun on push the boat forward
        return 2;
      }
      return 0;
  }
  else
  { 
    Serial.print(F("Location: ")); 
    Serial.print(F("INVALID"));
    Serial.println();
    return 3;
  }  
}

char getRealTime(){
     DS3231_get(&t);
     snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
     Serial.print(" The Time is: ");
     Serial.println(buff);
     return buff;
}

// making the post request
void  makePostRequest(char data[])
{
  Serial.print("Data Received :");
  Serial.println(data);
  Serial.println("POST Successfull");

}

JsonObject createJsonObject()
{
  
}

void loop()
{
  while (Serial1.available() > 0)
  {
     unsigned long now=millis();
    if((now - prev > interval))
    {
      Serial.println();
      if (gps.encode(Serial1.read()))
      {
         gps_Status=displayInfo();
      }else{
        gps_Status=3;
      }

      char realTime=getRealTime();                     // all the values
      int turbidity=getTurbidityData();
      int phValue=getPhValue();

     // update the Value in Sd car
     if(SD_status==1)
      {
      
      switch(gps_Status)
      {
        case 0: Serial.println("Location not changed!");
                break;
        case 1: // Opening the File
                myFile = SD.open("umesh.txt", FILE_WRITE);
                if (myFile) {
                   Serial.println("Writing to File...");
                    myFile.print(turbidity);
                    myFile.print(",");
                    myFile.print(latitude,6);
                    myFile.print(",");
                    myFile.print(longitude,6);
                    myFile.print(",");
                    myFile.print(dateFromGPS);
                    myFile.print(",");
                    myFile.print(timeFromGPS);
                    myFile.print(",");
                    myFile.print(altitudeFromGPS);
                    myFile.print(",");
                    myFile.print(speedFromGPS);
                    myFile.print(",");
                    myFile.print(buff);
                    myFile.print(",");
                    myFile.println("Okay");
                    myFile.close();
                    Serial.println("done.");
               }else {
                   SD_status=0;
                   Serial.println("error opening File");    // if the file didn't open, print an error:
               }

               // Update the States
              latitude_old_state=latitude;
              longitude_old_state=longitude;
               
               break;
        case 2: myFile = SD.open("umesh.txt", FILE_WRITE);
                if (myFile) {
                   Serial.println("Writing to File..");
                    myFile.print(turbidity);
                    myFile.print(",");
                    myFile.print(latitude,6);
                    myFile.print(",");
                    myFile.print(longitude,6);
                    myFile.print(",");
                    myFile.print(dateFromGPS);
                    myFile.print(",");
                    myFile.print(timeFromGPS);
                    myFile.print(",");
                    myFile.print(altitudeFromGPS);
                    myFile.print(",");
                    myFile.print(speedFromGPS);
                    myFile.print(",");
                    myFile.print(buff);
                    myFile.print(",");
                    myFile.println("Moving Very Slow or Stuck Somewhere");
                    myFile.close();
                    Serial.println("done.");
               }else {
                   SD_status=0;
                   Serial.println("error opening File");    // if the file didn't open, print an error:
               }
               // Update the States
              latitude_old_state=latitude;
              longitude_old_state=longitude;
              
            
              break;
        case 3:myFile = SD.open("umesh.txt", FILE_WRITE);
                if (myFile) {
                   Serial.println("Writing to File...");
                    myFile.print(turbidity);
                    myFile.print(",");
                    myFile.print(ErrorMessage);
                    myFile.print(",");
                    myFile.print(ErrorMessage);
                    myFile.print(",");
                    myFile.print(ErrorMessage);
                    myFile.print(",");
                    myFile.print(ErrorMessage);
                    myFile.print(",");
                    myFile.print(ErrorMessage);
                    myFile.print(",");
                    myFile.print(ErrorMessage);
                    myFile.print(",");
                    myFile.print(buff);
                    myFile.print(",");
                    myFile.println("INVALID LOCATION");
                    myFile.close();
                    Serial.println("done.");
                 }else {
                   SD_status=0;
                   Serial.println("error opening File");    // if the file didn't open, print an error:
                 }

                 
               break;
           default:Serial.println("Not a valid response");
      }
   }else if(SD_status==0){
      // try make the post request
          

     // If SD Card if not available then store the data in the local storage of the device
     Serial.println("SD Card Intialisation Failed");
     intialise_sd_card();
    }
    prev=millis();
  }else{
    Serial.print(".");
    //delay(50);
  }
}

// unable to fetch the information from the satelites
if(!Serial1.available())
{
  unsigned int now_1=millis();

  if((now_1-prev_1>interval_1))
  {
    char realTime=getRealTime();                     // all the values
    int turbidity=getTurbidityData();
    int phValue=getPhValue();
   if(SD_status==1)
    {
     myFile = SD.open("umesh.txt", FILE_WRITE);
             if (myFile) {
                Serial.println("Writing to File");
                myFile.print(turbidity);
                myFile.print(",");
                myFile.print(ErrorMessage);
                myFile.print(",");
                myFile.print(ErrorMessage);
                myFile.print(",");
                myFile.print(ErrorMessage);
                myFile.print(",");
                myFile.print(ErrorMessage);
                myFile.print(",");
                myFile.print(ErrorMessage);
                myFile.print(",");
                myFile.print(ErrorMessage);
                myFile.print(",");
                myFile.print(buff);
                myFile.print(",");
                myFile.println("INVALID LOCATION");
                myFile.close();
                Serial.println("done.");
             }else {
                   SD_status=0;
                   Serial.println("error opening File");    // if the file didn't open, print an error:
               }
      }else if(SD_status==0)
      {
         
          // If SD Card if not available then store the data in the local storage of the device
          Serial.println("SD Card Intialisation Failed");
          intialise_sd_card();
    }
    prev_1=millis();
  }else{
    Serial.print(".");
    //delay(50);
    intialise_gps();
  }
 }
}

