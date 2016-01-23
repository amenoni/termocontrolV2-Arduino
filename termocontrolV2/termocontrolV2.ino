#include <Mailbox.h>
#include <Process.h>
#include "config.h"
#include "ledMgr.h"

//Global Variables

int MAXTEMP;

int MODE = -1;
//operation modes REPOSE MODE = 0, WAITING = 1, PREPARE USAGE = 2
int HEATER_ON = 0; //0 = off 1 = on
int TARGET_TEMP;
float PERCENTAGE_TEMP_FOR_READY = 0.9; //when this temperature is reached we turn the heater off
boolean USAGE_READY = false;
const int USENOTSTARTED = 0;
const int INUSE = 1;
const int USEFINISHED = 2;

//linux side usage event constants
const int USAGE_STARTED = 0;
const int USAGE_FINISHED = 1;

//Global Variable to store the temperature value to avoid delays
double TEMP = 0;
//temperature last reading maxmiun valid time in seconds
int tempValidTimeSec = 30;
//to detect if a usage has been started or not we must check temperature changes, if it drops an usage has started if afeter the usage has been started the temperature rises again the usage has been finished
int InUseSensingTempTimeSec = 60;
//if the temperature droped in the last sensing interval is higer than this value we have been detected an usage. Value is % of the last sensed temperature
int MaxTempDropForUseDetectedPercent = -5;
//after we detect an usage we start looking a usage finished, to detect it we spect to temperature start rising again, if the temperature rises more than this percent from the last sensing we have detected an usage finish
int MaxTempUpForDetectUseFinishedPercent = 5;


//-----------------------------

void resetVariables(){
  USAGE_READY = false;
 
}

double lastCheckedTime = 0;

int getTimeDifInSecs(double time){
  //double diference = millis() - time;
  double diference = millis() - time;
  int intdiff = diference / 1000;
  return abs(intdiff); //return the absolute number so we dont return a negative number when millis count is reseted
}
  
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
    Console.print("ADC: "); 
    Console.print(RawADC); 
    Console.print("/1024");                           // Print out RAW ADC Number
    Console.print(", vcc: ");
    Console.print(vcc,2);
    Console.print(", pad: ");
    Console.print(pad/1000,3);
    Console.print(" Kohms, Volts: "); 
    Console.print(((RawADC*vcc)/1024.0),3);   
    Console.print(", Resistance: "); 
    Console.print(Resistance);
    Console.print(" ohms, ");
    // END- Remove these lines for the function not to display anything
    Console.print("Temperature= ");
    Console.println(TEMP);
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
    #if DEBUG_MODE
    Console.println("Heater ON"); 
    #endif //ECHO_TO_SERIA
  }else{
    digitalWrite(heaterRelay,0);
    HEATER_ON = 0;
    #if DEBUG_MODE
    Console.println("Heater OFF"); 
    #endif //ECHO_TO_SERIA
  
  }
}

//-----------------------


double lastCheckedTimeUsageTemp;
double lastInUseCheckedTemp;

int CurrentUseStatus = USENOTSTARTED;
double deltat = 0;
boolean ussageloged = false;

int UseStatus(){
 double t = millis();
 
 //clean all the variables if the data is old
 if(getTimeDifInSecs(lastCheckedTimeUsageTemp) >= (InUseSensingTempTimeSec)* 3){
    #if DEBUG_MODE
    Console.println("Cleaning in use method variables");
    #endif //ECHO_TO_SERIA
    CurrentUseStatus = USENOTSTARTED;
    deltat = 0;
    lastInUseCheckedTemp = 0;
    ussageloged = false;
 }


 //if the temperature is no longer valid
 if (getTimeDifInSecs(lastCheckedTimeUsageTemp) >=  InUseSensingTempTimeSec ){
     Console.println("Checking In use temp");
     
   if(lastInUseCheckedTemp == 0){
     lastInUseCheckedTemp = getTemp();
     lastCheckedTimeUsageTemp = t;
   }else{
    deltat = ((getTemp() * 100) / lastInUseCheckedTemp) - 100;
    //after we calculate the deltaT we set the historical temp for the next run
    lastInUseCheckedTemp = getTemp();
    lastCheckedTimeUsageTemp = t;

     #if DEBUG_MODE
    Console.println("Delta T = " + String(deltat));
    #endif //ECHO_TO_SERIA
    if(CurrentUseStatus == USENOTSTARTED ){
      if(deltat <= MaxTempDropForUseDetectedPercent){
             #if DEBUG_MODE
                Console.println("Use start detected");
             #endif //ECHO_TO_SERIA
        CurrentUseStatus = INUSE;
        if (ussageloged == false){
          logUsageEvent();
          ussageloged = true;
        }
          
        return INUSE;
      }
    }else if(CurrentUseStatus == INUSE){
      if(deltat >= MaxTempUpForDetectUseFinishedPercent){
             #if DEBUG_MODE
                Console.println("Use finished detected");
             #endif //ECHO_TO_SERIA
        CurrentUseStatus = USEFINISHED;
        logUsageEvent();     
        CurrentUseStatus = USENOTSTARTED; //reset the variable for the next usage
        return USEFINISHED;
      }
    }else{
      return CurrentUseStatus;
    }

   }


 }else{
   return CurrentUseStatus;
 }


}

void logUsageEvent(){
    Process p;
    p.begin("/mnt/sda1/arduino/registerUsageLog.py");
    if(CurrentUseStatus == INUSE){
      p.addParameter(String(USAGE_STARTED));  
    }else if(CurrentUseStatus == USEFINISHED){
      p.addParameter(String(USAGE_FINISHED));  
    }
    p.run();
    #if DEBUG_MODE
    Console.println("Log usage event triggered ");
    #endif
}


void setup() {
  Bridge.begin();
  Console.begin();     
  
  //pin config
  pinMode(heaterRelay, OUTPUT);
  digitalWrite(heaterRelay,LOW);
  pinMode(RedLed, OUTPUT);
  pinMode(BlueLed, OUTPUT);
  pinMode(GreenLed, OUTPUT);
  
  #if DEBUG_MODE
    while(!Console){      
      if(digitalRead(RedLed)==HIGH){
        SetLed(NIL);
      }else{
        SetLed(RED);
      }
      delay(500);
  }
  SetLed(NIL);
  Console.println("Debug Mode on");
  #endif //end Debug mode


  
}

void loop() {
String message;
// Mailbox control
if (Mailbox.messageAvailable()){
  Mailbox.readMessage(message);
  Console.println("Message = " +message);
  
  String action = message.substring(0,message.indexOf(" "));
  Console.println("Action: " +  action);  
  String command;
  String command_parameters;
  if(action == "updateTemp"){
    Process p;
    p.begin("/mnt/sda1/arduino/updateTemp.py");
    p.addParameter(String(TEMP));
    p.run();
    Console.println("New temperature seted");
  }else if(action == "heater"){
    command = message.substring(message.indexOf(" ")+1, message.length());
    Console.println("Command do: " + command);
    if(command == "on"){
      HeaterSwitch(true);
    }else{
      HeaterSwitch(false);
    }
  }else if(action == "mode"){
      message = message.substring(message.indexOf(" ")+1, message.length());
      int first_command_parameter = message.indexOf(" ");
      command = message.substring(0,first_command_parameter);
      #if DEBUG_MODE
        Console.println("command=" + command);
      #endif
      if(command == "repose"){
        resetVariables();
        MODE = 0;
      }else if(command == "waiting_temp"){
         resetVariables();
         MODE = 1;
         command_parameters = message.substring(first_command_parameter + 1, message.length());
         TARGET_TEMP = command_parameters.toInt();
         #if DEBUG_MODE
         Console.println("TARGET_TEMP= " + String(TARGET_TEMP));
         #endif
      }else if(command == "prepare_usage"){
         resetVariables();
         MODE = 2;
         command_parameters = message.substring(first_command_parameter + 1, message.length());
         TARGET_TEMP = command_parameters.toInt();
         #if DEBUG_MODE
         Console.println("Pepare usage TARGET_TEMP= " + String(TARGET_TEMP));
         #endif
        
      }
  }else if(action == "config"){
         message = message.substring(message.indexOf(" ")+1, message.length());
         int first_command_parameter = message.indexOf(" ");
         command = message.substring(0,first_command_parameter);
         if (command == "PERCENTAGE_TEMP_FOR_READY"){
            command_parameters = message.substring(first_command_parameter + 1, message.length());
            PERCENTAGE_TEMP_FOR_READY = command_parameters.toFloat();
            #if DEBUG_MODE
              Console.println("Config setted PERCENTAGE_TEMP_FOR_READY= " + String(PERCENTAGE_TEMP_FOR_READY));
            #endif
         }else if (command == "tempValidTimeSec"){
            command_parameters = message.substring(first_command_parameter + 1, message.length());
            tempValidTimeSec = command_parameters.toInt();
            #if DEBUG_MODE
              Console.println("Config setted tempValidTimeSec= " + String(tempValidTimeSec));
            #endif
         }else if (command == "InUseSensingTempTimeSec"){
            command_parameters = message.substring(first_command_parameter + 1, message.length());
            InUseSensingTempTimeSec = command_parameters.toInt();
            #if DEBUG_MODE
              Console.println("Config setted InUseSensingTempTimeSec= " + String(InUseSensingTempTimeSec));
            #endif
         }else if (command == "MaxTempDropForUseDetectedPercent"){
            command_parameters = message.substring(first_command_parameter + 1, message.length());
            MaxTempDropForUseDetectedPercent = command_parameters.toInt();
            #if DEBUG_MODE
              Console.println("Config setted MaxTempDropForUseDetectedPercent= " + String(MaxTempDropForUseDetectedPercent));
            #endif
         }else if (command == "MaxTempUpForDetectUseFinishedPercent"){
            command_parameters = message.substring(first_command_parameter + 1, message.length());
            MaxTempUpForDetectUseFinishedPercent = command_parameters.toInt();
            #if DEBUG_MODE
              Console.println("Config setted MaxTempUpForDetectUseFinishedPercent= " + String(MaxTempUpForDetectUseFinishedPercent));
            #endif
         }
         
      }
  Console.println("Waiting for new message");
}
//-------END Mailbox Control -------------------

//-----Mode control-----------
switch(MODE){
  case -1:
    if(HEATER_ON == 0){
      HeaterSwitch(true);
      #if DEBUG_MODE
        Console.println("Mode -1 heater on");
      #endif 
    }
    break;
  case 0: //REPOSE MODE
    if(HEATER_ON == 1){
      HeaterSwitch(false);
      #if DEBUG_MODE
        Console.println("Mode REPOSE heater OFF");
      #endif 
    }
    break;
  case 1: //Waiting temp mode
    if(getTemp() < TARGET_TEMP * PERCENTAGE_TEMP_FOR_READY){
      HeaterSwitch(true);
      #if DEBUG_MODE
        Console.println("Mode WAITING TEMP Heater On Current temp: " + String(getTemp()) );
      #endif 
    }else{
      HeaterSwitch(false);
      Console.println("Mode WAITING TEMP Heater off Current temp: " + String(getTemp()) );
    }
    break;
  case 2: //Prepare usage mode
    if(getTemp() < TARGET_TEMP * PERCENTAGE_TEMP_FOR_READY){
      HeaterSwitch(true);
      USAGE_READY = false;
      #if DEBUG_MODE
        Console.println("Mode PREPARE USAGE Heater On Current temp: " + String(getTemp()) );
      #endif 
    }else{
      USAGE_READY = true;
      HeaterSwitch(false);
      Console.println("Mode PREPARE USAGE Heater off Current temp: " + String(getTemp()) );
    }
    break;
  
}

//----------------------------
//---UI controll ---------
  if(HEATER_ON == 0){
    if(USAGE_READY == true){
      SetLed(GREEN);  
    }else{
      SetLed(BLUE);  
    }
    
  }else{
    SetLed(RED);   
  }
  
//-------------------------

//-------Detect an usage ----------

UseStatus();


//---------------------------------


  
  delay(1000);
  

 
}
