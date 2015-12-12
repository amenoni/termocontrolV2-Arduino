//---------------Heater Relay config
#define heaterRelay 13 //pin to control de heater relay
int heaterOn = 1; //0 = off 1 = on
//--------------------------------------

//--------------Others Config-----------
#define btnOk 49
#define btnNext 51
#define btnBack 47
#define DEBUG_MODE 1  //echo data to serial port 1= true 0 = false 
//------------------------------

//-----------Temperature sensor config

#define ThermistorPIN 0                 // Analog Pin 0

float vcc = 4.96;                       // only used for display purposes, if used
                                        // set to the measured Vcc.
float pad = 9740;                       // balance/pad resistor value, set this to
                                        // the measured resistance of your pad resistor
float thermr = 10000;                   // thermistor nominal resistance

//----------------------------------
