#include <RCSwitch.h>

#define RX_PIN 0

RCSwitch receiver = RCSwitch();
unsigned long temperature_value;
float minus_compensation = 20.0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up...");
  receiver.enableReceive(RX_PIN);  // Receiver on interrupt 0 => that is pin #2
  Serial.println("running");
}

void loop() {
  if (receiver.available()) {
    int bitlength = receiver.getReceivedBitlength();
    unsigned long temperature_value = receiver.getReceivedValue();

    Serial.print("Received Value is:");
    Serial.println(temperature_value);
    
    float inside = (temperature_value % 100000000 / 1000000.0) - minus_compensation;
    float outside = (temperature_value % 10000 / 100.0) - minus_compensation;
    Serial.print("Inside: ");
    Serial.println(inside);
    Serial.print("Outside: ");
    Serial.println(outside);
    receiver.resetAvailable();
  }
}
