#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"
SdFat SD;

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
   SPI.begin(); // wake up the SPI bus.
  //SPI.setBitOrder(MSBFIRST);
  Serial.println("Initializing SD card...");
  //pinMode(4,OUTPUT);
  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
 /* myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, Check the data Again");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }*/

  // re-open the file for reading:
  myFile = SD.open("umesh.txt");
  if (myFile) {
    Serial.println("Result is");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening umesh.txt");
  }
}

void loop() {
  // nothing happens after setup
}


