//Led pins
const int RedLed = 10;
const int GreenLed = 11;
const int BlueLed = 12;

//led colors
const int NIL = 0;
const int GREEN = 1;
const int RED = 2;
const int BLUE = 3;

void SetLed(int color){
  switch (color){
   case NIL:
      digitalWrite(RedLed, LOW);
      digitalWrite(BlueLed, LOW);
      digitalWrite(GreenLed, LOW);
      #if DEBUG_MODE
        Serial.println("LED Set NIL"); 
      #endif //ECHO_TO_SERIA  
      break; 
   case GREEN:
      digitalWrite(RedLed, LOW);
      digitalWrite(BlueLed, LOW);
      digitalWrite(GreenLed, HIGH);
      #if DEBUG_MODE
        Serial.println("LED Set GREEN"); 
      #endif //ECHO_TO_SERIA
      break; 
   case RED:
      digitalWrite(RedLed, HIGH);
      digitalWrite(BlueLed, LOW);
      digitalWrite(GreenLed, LOW);
      #if DEBUG_MODE
        Serial.println("LED Set RED"); 
      #endif //ECHO_TO_SERIA
      break; 
   case BLUE:
      digitalWrite(RedLed, LOW);
      digitalWrite(BlueLed, HIGH);
      digitalWrite(GreenLed, LOW);
      #if DEBUG_MODE
        Serial.println("LED Set BLUE"); 
      #endif //ECHO_TO_SERIA
      break; 
  }
}

