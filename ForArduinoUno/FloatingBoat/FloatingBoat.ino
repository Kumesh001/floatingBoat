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
unsigned long prev, interval = 5000;

double latitude=0.0;
double longitude=0.0;

double latitude_old_state=0.0;
double longitude_old_state=0.0;

int SD_status=-1;
int GPS_status=-1;

int pinCS = 10; 
static const int RXPin = 7, TXPin = 6;

int photocellPin = A1;    //connect cell to analog pin 0 with 10k pull down resistor to ground
int ledPin = 13;  //Connect LED to run pin 13 in series with appropriate resistor

SoftwareSerial ss(RXPin, TXPin);     // RX, TX

void parse_cmd(char *cmd, int cmdsize);

static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
File myFile;

#define DS1307_I2C_ADDRESS 0x68

void setup()
{
  // Intialising the Serial Communication
  Serial.begin(9600);
  Serial.println("Initializing GPS...");
 
  intialise_gps();
  //SPI_Initialise();
  intialise_clock();
  intialise_sd_card();
  intialise_gsm(); 
  intialise_Sensors();
  //SD_status=1;
}
void intialise_Sensors(){
   pinMode (ledPin, OUTPUT);  //Set pin to outut
   pinMode (photocellPin, INPUT);
}

void intialise_sd_card(){
  Serial.println("Initializing SD card...");
  pinMode(pinCS,OUTPUT);
  //digitalWrite(pinCS,LOW); 
  if (!SD.begin(pinCS)) {
     Serial.println("SD initialization failed!");
     SD_status=0;
  }else{
    Serial.println("Done.");
    SD_status=1;
    delay(100);
  }
  Serial.print("SD Card Status is :");
  Serial.println(SD_status);
}

void intialise_clock(){
  Serial.println("Initializing Clock...");
  //digitalWrite(10,HIGH);
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  memset(recv, 0, BUFF_MAX);
  Serial.println("Done");
}

void intialise_gps(){
   Serial.println("Initializing GPS...");
   ss.begin(GPSBaud);
   Serial.println("Done");
}

void intialise_gsm(){
  Serial.println("Initializing GSM Module...");
  Serial.println("Done");
}

int getTurbidityData(){
 float val = analogRead(photocellPin);  //create variable to take in analog reading from cell
 
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

int displayInfo()
{
  
  if (gps.location.isValid())
  {
        latitude=gps.location.lat();
        longitude=gps.location.lng();
        // Print the states on the Serial monitor
        Serial.print(F("Location: ")); 
        Serial.print(F(" Latitude is: "));
        Serial.print(latitude,6);
        Serial.print(F(" Longitude is: "));
        Serial.println(longitude,6);
        return 1;
  }
  else
  { Serial.print(F("Location: ")); 
    Serial.print(F("INVALID"));
    Serial.println();
    return 0;
  }  
}

void loop()
{
  while (ss.available() > 0)
  {
     struct ts t;
     char buff[BUFF_MAX];
     DS3231_get(&t);
     snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
     Serial.print(" The Time is: ");
     Serial.println(buff);
    
    if (gps.encode(ss.read()))
    {
      //Serial.println("Here ?");
      GPS_status=displayInfo();
    }
    // update the Value in Sd card
    if(SD_status==1)
    { 
     if(longitude_old_state!=longitude || latitude_old_state!=latitude){
        // Opening the File
        int result=getTurbidityData();
        myFile = SD.open("GPS.txt", FILE_WRITE);
        if (myFile) {
           Serial.println("Writing to GPS.txt...");
           myFile.print("Take-1");
           myFile.print(latitude,6);
           myFile.print(",");
           myFile.print(longitude,6);
           myFile.print(",");
           myFile.print(buff);
           myFile.close();
           Serial.println("done.");
        }else {
            // if the file didn't open, print an error:
          Serial.println("error opening GPS.txt");
        }
        
        // Update the States
        latitude_old_state=latitude;
        longitude_old_state=longitude;
                
      }
   }else{
     // If SD Card if not available then store the data in the local storage of the device
     Serial.println("SD Card Intialisation Failed");
    }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}
// Below is the code for setting the time
/*
void setTime(char *cmd, int cmdsize)
{
   uint8_t i;
    uint8_t reg_val;
    char buff[BUFF_MAX];
    struct ts t;

    //snprintf(buff, BUFF_MAX, "cmd was '%s' %d\n", cmd, cmdsize);
    //Serial.print(buff);

    // TssmmhhWDDMMYYYY aka set time
    if (cmd[0] == 84 && cmdsize == 16) {
        //T355720619112011
        t.sec = inp2toi(cmd, 1);
        t.min = inp2toi(cmd, 3);
        t.hour = inp2toi(cmd, 5);
        t.wday = cmd[7] - 48;
        t.mday = inp2toi(cmd, 8);
        t.mon = inp2toi(cmd, 10);
        t.year = inp2toi(cmd, 12) * 100 + inp2toi(cmd, 14);
        //t.year=2018;
        DS3231_set(t);
        Serial.println("OK");
    } else if (cmd[0] == 49 && cmdsize == 1) {  // "1" get alarm 1
        DS3231_get_a1(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 50 && cmdsize == 1) {  // "2" get alarm 1
        DS3231_get_a2(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 51 && cmdsize == 1) {  // "3" get aging register
        Serial.print("aging reg is ");
        Serial.println(DS3231_get_aging(), DEC);
    } else if (cmd[0] == 65 && cmdsize == 9) {  // "A" set alarm 1
        DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
        //ASSMMHHDD
        for (i = 0; i < 4; i++) {
            time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // ss, mm, hh, dd
        }
        uint8_t flags[5] = { 0, 0, 0, 0, 0 };
        DS3231_set_a1(time[0], time[1], time[2], time[3], flags);
        DS3231_get_a1(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 66 && cmdsize == 7) {  // "B" Set Alarm 2
        DS3231_set_creg(DS3231_INTCN | DS3231_A2IE);
        //BMMHHDD
        for (i = 0; i < 4; i++) {
            time[i] = (cmd[2 * i + 1] - 48) * 10 + cmd[2 * i + 2] - 48; // mm, hh, dd
        }
        uint8_t flags[5] = { 0, 0, 0, 0 };
        DS3231_set_a2(time[0], time[1], time[2], flags);
        DS3231_get_a2(&buff[0], 59);
        Serial.println(buff);
    } else if (cmd[0] == 67 && cmdsize == 1) {  // "C" - get temperature register
        Serial.print("temperature reg is ");
        Serial.println(DS3231_get_treg(), DEC);
    } else if (cmd[0] == 68 && cmdsize == 1) {  // "D" - reset status register alarm flags
        reg_val = DS3231_get_sreg();
        reg_val &= B11111100;
        DS3231_set_sreg(reg_val);
    } else if (cmd[0] == 70 && cmdsize == 1) {  // "F" - custom fct
        reg_val = DS3231_get_addr(0x5);
        Serial.print("orig ");
        Serial.print(reg_val,DEC);
        Serial.print("month is ");
        Serial.println(bcdtodec(reg_val & 0x1F),DEC);
    } else if (cmd[0] == 71 && cmdsize == 1) {  // "G" - set aging status register
        DS3231_set_aging(0);
    } else if (cmd[0] == 83 && cmdsize == 1) {  // "S" - get status register
        Serial.print("status reg is ");
        Serial.println(DS3231_get_sreg(), DEC);
    } else {
        Serial.print("unknown command prefix ");
        Serial.println(cmd[0]);
        Serial.println(cmd[0], DEC);
    }
}
*/

