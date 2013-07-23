#include <Time.h>
#include <Wire.h> 

// Display Green "Attach Battery"  message
// Once attached, change screen to red and start test
// Measure energy released from battery and log to USB

#define REDLITE       3
#define GREENLITE     5
#define BLUELITE      6
#define ONE_WIRE_BUS  4
#define V_LOAD_PIN    A0
#define V_LOAD_PIN1   A1
#define R_LOAD        10
#define FINAL_VOLTAGE 0.2

// TODO initialize the library with the numbers of the interface pins 

float joules = 0;
float voltage = 0;
float temp = 0;
float joules1 = 0;
float voltage1 = 0;
float temp1 = 0;
uint8_t hours = 0;
uint8_t mins = 0;
uint8_t lastSecond;
bool batteryAttached = false;
bool testComplete = false;
time_t startTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(V_LOAD_PIN, INPUT);
  pinMode(V_LOAD_PIN1, INPUT);
  time_t t = now(); 
  lastSecond = second(t);
}

void loop() {
  if (batteryAttached) {
    if (testComplete) {
      Serial.println("Test complete");
      delay(10000);
    } else {
      time_t t = now()-startTime; 
      uint8_t sec = second(t);
      if (sec != lastSecond) {
        lastSecond = sec;
        hours = hour(t);
        mins = minute(t);
        voltage = 5.0 * ((float) analogRead(V_LOAD_PIN)) / 1024.0;
        voltage1 = 5.0 * ((float) analogRead(V_LOAD_PIN1)) / 1024.0;
        float current = voltage / R_LOAD;
        float current1 = voltage1 / R_LOAD;
        joules += voltage * current;
        joules1 += voltage1 * current1;
        float wh = joules / 3600;
        float wh1 = joules1 / 3600;

        //TODO Add lcd commands for jy-lkm1638
        
        Serial.print(t);
        Serial.print(",");
        Serial.print(voltage);
        Serial.print(",");
        Serial.print(current);
        Serial.print(",");
        Serial.print(joules);
        Serial.print(",");
        Serial.print(wh);
        Serial.print(", ");
        Serial.print(voltage1);
        Serial.print(",");
        Serial.print(current1);
        Serial.print(",");
        Serial.print(joules1);
        Serial.print(",");
        Serial.print(wh1);
        Serial.println();
        if (voltage < FINAL_VOLTAGE) {
          testComplete = true;
        }
      }
    }
  } else {
    voltage = 5.0 * ((float) analogRead(V_LOAD_PIN)) / 1024.0;
    voltage1  = 5.0 * ((float) analogRead(V_LOAD_PIN1)) / 1024.0;
    if (voltage > 0.02) {
      startTime = now(); 
      batteryAttached = true;
      Serial.println("time,voltage,current,joules,temp");
    }      
  }
}
