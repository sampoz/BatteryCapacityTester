#include <OneWire.h>
#include <DallasTemperature.h>
#include <Time.h>
#include <LiquidCrystal.h>
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
#define R_LOAD        5.5
#define FINAL_VOLTAGE 0.2

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

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
      updateDisplay();
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

        //TODO Add lcd commands for jy-lkm1638

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
      setBacklight(255, 0, 0, 255);
      Serial.println("time,voltage,current,joules,temp");
    }      
  }
}

void setBacklight(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
  // normalize the red LED - its brighter than the rest!
  r = map(r, 0, 255, 0, 100);
  g = map(g, 0, 255, 0, 150);
 
  r = map(r, 0, 255, 0, brightness);
  g = map(g, 0, 255, 0, brightness);
  b = map(b, 0, 255, 0, brightness);
 
  // common anode so invert!
  r = map(r, 0, 255, 255, 0);
  g = map(g, 0, 255, 255, 0);
  b = map(b, 0, 255, 255, 0);
  analogWrite(REDLITE, r);
  analogWrite(GREENLITE, g);
  analogWrite(BLUELITE, b);
}

// Update the LCD with the following format:
//
//  +----------------+
//  |99999.9J 99.99Wh|
//  |9.99V 99^C 59:59|
//  +----------------+

void updateDisplay() {
  char row[32];
  memset(row, 0, 17);

  float wattHours = joules / 3600;
  if (joules < 9999)
    fmtDouble(joules, 2, &row[0], 8);
  else
    fmtDouble(joules, 1, &row[0], 8);
  strcat(row, "J");
  char whStr[9];  
  fmtDouble(wattHours, 2, whStr, sizeof(whStr));
  sprintf(&row[9], "%5sWh", whStr);
  for (int i=0;i<16;i++)
    if (row[i] == 0)
      row[i] = ' ';
  lcd.setCursor(0, 0);
  lcd.print(row);
  
  if (testComplete) {
    lcd.setCursor(0, 1);
    lcd.print(" Test Complete! ");    
  } else {
    memset(row, 0, 17);
    fmtDouble(voltage, 2, &row[0], 8);
    strcat(row, "V");
    fmtDouble(temp, 0, &row[6], 8);
    strcat(&row[6], "\xDF");
    strcat(&row[6], "C");
    uint8_t degree = 0xEF;
    sprintf(&row[11], "%02d:%02d", hours, mins);
    for (int i=0;i<16;i++)
      if (row[i] == 0)
        row[i] = ' ';
    lcd.setCursor(0, 1);
    lcd.print(row);
  }
}

void fmtDouble(double val, byte precision, char *buf, unsigned bufLen = 0xffff);
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen = 0xffff, byte width = 0);

//
// Produce a formatted string in a buffer corresponding to the value provided.
// If the 'width' parameter is non-zero, the value will be padded with leading
// zeroes to achieve the specified width.  The number of characters added to
// the buffer (not including the null termination) is returned.
//
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen, byte width) {
  if (!buf || !bufLen)
    return(0);

  // produce the digit string (backwards in the digit buffer)
  char dbuf[10];
  unsigned idx = 0;
  while (idx < sizeof(dbuf)) {
    dbuf[idx++] = (val % 10) + '0';
    if ((val /= 10) == 0)
      break;
  }

  // copy the optional leading zeroes and digits to the target buffer
  unsigned len = 0;
  byte padding = (width > idx) ? width - idx : 0;
  char c = '0';
  while ((--bufLen > 0) && (idx || padding)) {
    if (padding)
      padding--;
    else
      c = dbuf[--idx];
    *buf++ = c;
    len++;
  }

  // add the null termination
  *buf = '\0';
  return(len);
}

//
// Format a floating point value with number of decimal places.
// The 'precision' parameter is a number from 0 to 6 indicating the desired decimal places.
// The 'buf' parameter points to a buffer to receive the formatted string.  This must be
// sufficiently large to contain the resulting string.  The buffer's length may be
// optionally specified.  If it is given, the maximum length of the generated string
// will be one less than the specified value.
//
// example: fmtDouble(3.1415, 2, buf); // produces 3.14 (two decimal places)
//
void fmtDouble(double val, byte precision, char *buf, unsigned bufLen) {
  if (!buf || !bufLen)
    return;

  // limit the precision to the maximum allowed value
  const byte maxPrecision = 6;
  if (precision > maxPrecision)
    precision = maxPrecision;

  if (--bufLen > 0) {
    // check for a negative value
    if (val < 0.0) {
      val = -val;
      *buf = '-';
      bufLen--;
    }

    // compute the rounding factor and fractional multiplier
    double roundingFactor = 0.5;
    unsigned long mult = 1;
    for (byte i = 0; i < precision; i++) {
      roundingFactor /= 10.0;
      mult *= 10;
    }

    if (bufLen > 0) {
      // apply the rounding factor
      val += roundingFactor;

      // add the integral portion to the buffer
      unsigned len = fmtUnsigned((unsigned long)val, buf, bufLen);
      buf += len;
      bufLen -= len;
    }

    // handle the fractional portion
    if ((precision > 0) && (bufLen > 0)) {
      *buf++ = '.';
      if (--bufLen > 0)
        buf += fmtUnsigned((unsigned long)((val - (unsigned long)val) * mult), buf, bufLen, precision);
    }
  }

  // null-terminate the string
  *buf = '\0';
} 

