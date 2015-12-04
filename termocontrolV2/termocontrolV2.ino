#include <Mailbox.h>
#include <Process.h>
#include "config.h"
#include "ledMgr.h"

//Global Variables

int MAXTEMP;
//Current Mode of operation 0=NORMAL, 1=REPOSE 2=SETTINGS
int MODE;
int HEATER_ON = 0; //0 = off 1 = on
//-----------------------------

  
//Temperature sensor ---------------------- 
double getTemp(){
  long Resistance;  
  int RawADC = analogRead(ThermistorPIN);
  Resistance=pad*((1024.0 / RawADC) - 1); 
  TEMP = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  TEMP = 1 / (0.001129148 + (0.000234125 * TEMP) + (0.0000000876741 * TEMP * TEMP * TEMP));
  TEMP = TEMP - 273.15;  // Convert Kelvin to Celsius                      

   #if DEBUG_MODE
     
    // BEGIN- Remove these lines for the function not to display anything
    Serial.print("ADC: "); 
    Serial.print(RawADC); 
    Serial.print("/1024");                           // Print out RAW ADC Number
    Serial.print(", vcc: ");
    Serial.print(vcc,2);
    Serial.print(", pad: ");
    Serial.print(pad/1000,3);
    Serial.print(" Kohms, Volts: "); 
    Serial.print(((RawADC*vcc)/1024.0),3);   
    Serial.print(", Resistance: "); 
    Serial.print(Resistance);
    Serial.print(" ohms, ");
    // END- Remove these lines for the function not to display anything
    Serial.print("Temperature= ");
    Serial.println(TEMP);
   #endif //end debug mode
   
   //Update MAXTEMP Value
   if (TEMP > MAXTEMP){
     MAXTEMP = TEMP;
   } 
   return TEMP;  
}

//-----------------------
//Heater switch control --------------
void HeaterSwitch (boolean isOn){
  if(isOn == true){
    digitalWrite(heaterRelay,1);
    HEATER_ON = 1;
    #if ECHO_TO_SERIAL
    Serial.println("Heater ON"); 
    #endif //ECHO_TO_SERIA
  }else{
    digitalWrite(heaterRelay,0);
    HEATER_ON = 0;
    #if ECHO_TO_SERIAL
    Serial.println("Heater OFF"); 
    #endif //ECHO_TO_SERIA
  
  }
}

//-----------------------



void setup() {
  Bridge.begin();
  Serial.begin(9600);     
  
  //pin config
  pinMode(heaterRelay, OUTPUT);
  digitalWrite(heaterRelay,LOW);
  pinMode(RedLed, OUTPUT);
  pinMode(BlueLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  
  #if DEBUG_MODE
    while(!Serial){      
      if(digitalRead(RedLed)==HIGH){
        SetLed(NIL);
      }else{
        SetLed(RED);
      }
      delay(500);
  }
  SetLed(NIL);
  Serial.println("Debug Mode on");
  #endif //end Debug mode


  
}

void loop() {
String message;
// Mailbox control
if (Mailbox.messageAvailable()){
  // read all the messages present in the queue
  while (Mailbox.messageAvailable()){
    Mailbox.readMessage(message);
    Serial.println(message);
  }
  String action = message.substring(0,message.indexOf(" "));
  Serial.println("Action: " +  action);  
  String command;
  if(action == "updateTemp"){
    Process p;
    p.begin("/mnt/sda1/arduino/updateTemp.py");
    p.addParameter(String(getTemp()));
    p.run();
    Serial.println("New temperature seted");
  }else if(action == "heater"){
    command = message.substring(message.indexOf(" ")+1, message.length());
    Serial.println("Command: " + command);
    if(command == "on"){
      HeaterSwitch(true);
    }else{
      HeaterSwitch(false);
    }
  }else if(action == "mode"){
    //changes between modes
  }
  Serial.println("Waiting for new message");
}
//-------END Mailbox Control -------------------

//-----Modes control-----------



//---UI controll ---------
  if(HEATER_ON == 0){
    SetLed(BLUE);
  }else{
    SetLed(RED);
  }//TODO: Set the led green when the usage is ready

//-------------------------
  
  delay(1000);
  

 
}
