#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (lightMeter.begin()) {
    Serial.println("BH1750 Initialized!");
  } else {
    Serial.println("Failed to initialize BH1750");
  }
}

void loop() {
  float lux = lightMeter.readLightLevel();
  // sjekker om lysintensiteten er over 100 lux for å bare få output når blitsen går av
  if (lux > 100) {
    Serial.print("lux:");
    Serial.print(lux);
  }
  // setter en delay som gir noen få outputs per flash, sånn at jeg er sikker på å alltid få målt verdien men uten å måtte scrolle gjennom altfor mye outputs
  delay(40);
}
