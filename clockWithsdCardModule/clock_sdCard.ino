#include <SPI.h>
#include<Wire.h>
#include "SdFat.h"
#include "ds3231.h"

#define BUFF_MAX 128
#define SD_CS_PIN 53

uint8_t time[8];
char recv[BUFF_MAX];
unsigned int recv_size = 0;
unsigned long prev, interval = 1000;

SdFat SD;
volatile File myFile;
volatile int i=0;

int SD_cardStatus=0;

struct ts t;
volatile char buff[BUFF_MAX];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing SD card...");
   
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

   Wire.begin();
   DS3231_init(DS3231_INTCN);
   memset(recv, 0, BUFF_MAX);
}

void loop() {
 myFile=SD.open("gpsFile.txt",FILE_WRITE);
 Serial.print("myFile Output is : ");
 Serial.println(myFile);
 if(myFile)
 { 
    Serial.println("Writing into the File");
    myFile.print(i);
    myFile.print(',');
    i=i+1;
    myFile.close();
 }else{
  Serial.println("Error Opening the File");
 }
 delay(1000);
 showTime();
 delay(1000);
}
void showTime()
{
   DS3231_get(&t); //Get time
   Serial.println("GET time");
   Serial.print(t.mday);
   Serial.print(t.year);
   Serial.print("  ");
   Serial.print(t.hour);
}

