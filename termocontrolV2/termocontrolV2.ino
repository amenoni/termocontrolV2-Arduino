#include <RTClib.h>
#include <Process.h>
#include "config.h"

//Global Variables

int MAXTEMP;
//Current Mode of operation 0=NORMAL, 1=REPOSE 2=SETTINGS
int MODE;

//-----------------------------


DateTime updateCurrentTime(){


//Busca la forma de actualizar la hora como esta en el tutorial cosa que despues puedas hacer RTC.now o algo asi
//La otra es usar time que se setea con  setTime(t);  y luego se llama con hour, day, etc

  
Process process;
DateTime datetime;
String timeString;
if (!process.running())  {
    process.begin("date");
    process.addParameter("+%s%z");
    process.run();
  }
while (process.available()>0){
  timeString = process.readString();
  
  String hemisfere = timeString.substring(10,11);
  int timezone = timeString.substring(11,13).toInt();
  long timestamp = 0;
  if(hemisfere == "-"){
    //TODO: ARMAR UN FUCKING SCRIPT QUE HAGA EN ESTO EN PYHTON Y TA!
    timestamp = timeString.substring(0,10) - (timezone * 60);
  }else{
    timestamp = timeString.substring(0,10) + (timezone * 60);
  }
  datetime = DateTime(timestamp);
  #if DEBUG_MODE
    Serial.println("Current timestamp= " + timeString);
    Serial.println("Current timestamp= " + String(timestamp));
    Serial.println(hemisfere);
    Serial.println(String(timezone));
    
    Serial.print("Current date= ");
    Serial.println(String(datetime.day())+"/"+datetime.month()+"/"+datetime.year()+" "+datetime.hour()+":"+datetime.minute());
  #endif //end debug_mode
}
 
  return datetime;
}
  
//last time the temperature was checked
DateTime tempCheckedTime;

double getTemp(){
  DateTime t = updateCurrentTime();
  delay(200);
 //if the temperature is no longer valid
 if ( t.unixtime() >= tempCheckedTime.unixtime() + tempValidTimeSec ){

  long Resistance;  
  int RawADC = analogRead(ThermistorPIN);
  Resistance=pad*((1024.0 / RawADC) - 1); 
  TEMP = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
  TEMP = 1 / (0.001129148 + (0.000234125 * TEMP) + (0.0000000876741 * TEMP * TEMP * TEMP));
  TEMP = TEMP - 273.15;  // Convert Kelvin to Celsius                      

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
   
   tempCheckedTime = t;
   #if DEBUG_MODE
     Serial.print("Temperature= ");
     Serial.println(TEMP);
   #endif //end debug mode
   
   //Update MAXTEMP Value
   if (TEMP > MAXTEMP){
     MAXTEMP = TEMP;
     //TODO
     //saveConfigFile();
   }
   
   
   return TEMP;  
 }else{
  return TEMP; 
 }
}

void setup() {
  Bridge.begin();
  Serial.begin(9600);     
  #if DEBUG_MODE
  while(!Serial);      
  Serial.println("Time Check");
  #endif //end Debug mode
}

void loop() {
  // put your main code here, to run repeatedly:
  getTemp();
  delay(10000);
}
