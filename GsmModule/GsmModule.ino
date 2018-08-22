String remoteNumber = "";  // the number you will call
char charbuffer[20];
char VodafoneNumber[]="9990538243";
char JioNumber[]="9315693639";

void setup()
{
 Serial.begin(9600);
 Serial2.begin(19200);
 //intialiseGSM();
 //connectToInternet();
 checkGsmStatus();
 //SendMessage("Hello umesh message from the GSM Module");
 delay(1000);
 callSomeone(JioNumber); // call someone
 //receiveCall();
}

void connectToInternet(){
  Serial.println("Connecting to Internet ...");
  Serial2.print("AT");
  Serial2.println();
  connectGPRS();
  //connectHTTP();
  //post();
}

void intialiseGSM()
{
  while(Serial2.available()!=0)
  {
    Serial2.println("Not Connected");
    delay(1000);
  }
  Serial.println("Intialised the GSM Module"); 
}

void checkGsmStatus()
{
 Serial.println("Checking the GSM Status");
 Serial2.println("AT"); 
 char response[200];
 for(int i = 0 ; Serial.available() > 0 && i<200 ; i++) {
   response[i++] = Serial.read();
 }
 Serial.print("The Result of the Commands");
 Serial.println(response);
 delay(1000);
}

void callSomeone(char number[])
{
 Serial.println("Wait Calling");
 Serial2.print("ATD");
 Serial2.print(number); // dial US (212) 8675309
 Serial2.println(";");
 delay(30000);      // wait for 30 seconds...
 Serial2.println("ATH"); // hang up
 delay(300);
 Serial.println("Call Ended");
}

void receiveCall()
{
  Serial.println("Wait.. Receiving Call");
  Serial2.println("ATA");
  delay(20000);
  Serial2.println("ATH"); // hang up
  Serial.println("call Ended");
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

void RecieveMessage()
{
  Serial2.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  delay(1000);
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

 }

 void ShowSerialData()
{
  while(Serial2.available()!=0)
  { 
    Serial.write(Serial2.read());
  }
}

void loop()
{
  
}
