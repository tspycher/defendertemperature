#include <OneWire.h>
#include <DallasTemperature.h>
#include <RCSwitch.h>


#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>

#endif
// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TX_PIN 3

#define TEMPERATURE_INDEX_OUTSIDE 0
#define TEMPERATURE_INDEX_INSIDE 1

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
RCSwitch sender = RCSwitch();

U8G2_SSD1306_128X32_UNIVISION_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ A5, /* data=*/ A4);


float temperature_outside;
float temperature_inside;
unsigned long looper;

void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  sensors.begin();  // Start up the library
  Serial.begin(115200);
  sender.enableTransmit(TX_PIN);
  u8g2.begin();

  temperature_outside = 0.0;
  temperature_inside = 0.0;
  looper = 0;
}

void debug_temperature() {
  Serial.print("Temperature outside: ");
  Serial.print(temperature_outside);
  Serial.print(" C ");
  Serial.print("Temperature inside: ");
  Serial.print(temperature_inside);
  Serial.println(" C");
}

void blink(int delay_ms=1000) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(delay_ms/2);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(delay_ms/2);
}

void send_temperature(float inside, float outside, int identifier = 11, float minus_compensation = 20.0) {
    unsigned long transcoded_inside = (unsigned long)((inside + minus_compensation) * 100.0);
    unsigned long transcoded_outside = (unsigned long)((outside + minus_compensation) * 100.0);
    unsigned long rfpayload = ((unsigned long)identifier * 100000000) + transcoded_inside + (transcoded_outside * 10000);
    Serial.print("Sending Temperature over 433Mhz -> Inside: ");
    Serial.print(transcoded_inside);
    Serial.print(" Outside: ");
    Serial.print(transcoded_outside);
    Serial.print(" Minus Compensation: ");
    Serial.print(minus_compensation);
    Serial.print(" Final Payload: ");
    Serial.println(rfpayload);
    sender.send(rfpayload, 32);    
}
void loop(void)
{ 
  float new_temperature_outside;
  float new_temperature_inside;
  bool updated = false;
  
  sensors.requestTemperatures();
  
  new_temperature_outside = sensors.getTempCByIndex(TEMPERATURE_INDEX_OUTSIDE);
  new_temperature_inside = sensors.getTempCByIndex(TEMPERATURE_INDEX_INSIDE);
  
  if(new_temperature_outside <= 50.0 && new_temperature_outside >= -40.0) {
    if(new_temperature_outside != temperature_outside) {
      temperature_outside = new_temperature_outside;
      updated = true;
    }
  }
  
  if(new_temperature_inside <= 50.0 && new_temperature_inside >= -40.0) {
    if(new_temperature_inside != temperature_inside) {
      temperature_inside = new_temperature_inside;
      updated = true;
    }
  }

  String tmp_out = String(temperature_outside) + String("C");
  String tmp_in = String(temperature_inside) + String("C");
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(0,12, "Inside");
    u8g2.drawStr(0,32, "Outside");
    u8g2.drawStr(60,12, tmp_in.c_str());
    u8g2.drawStr(60,32, tmp_out.c_str());
  } while ( u8g2.nextPage() );

  if(updated) {
    Serial.println("Temperature Change");
    debug_temperature();
    send_temperature(temperature_inside, temperature_outside);
    blink(1000);
  } else {
    if(looper % 10 == 0) {
      debug_temperature();
      Serial.println("Periodic Temperature Broadcast");
      send_temperature(temperature_inside, temperature_outside);
      blink(1000);
    } else {
      delay(1000);
    }
  }
  ++looper;
}
