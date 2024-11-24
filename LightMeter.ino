#include <Wire.h>
#include <BH1750.h>

float N[] = {45, 32, 22, 16, 11, 8, 5.6, 4, 2.8, 2, 1.4, 1};  // blender (f-stop)
float t[] = {1/2000.0, 1/1000.0, 1/500.0, 1/250.0, 1/125.0, 1/60.0, 1/30.0, 1/15.0, 1/8.0, 1/4.0, 1/2.0, 1.0};  // lukkertid
int S[] = {25, 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200};  // ISO-verdier

BH1750 lightMeter;

float tvalgt = 1/100.0;  // Default lukkertid (1/100)
int Svalgt = 200;  // Default ISO (200)
float E = 0;  // Lux fra sensor
float EVlux = 0;  // EV regnet ut fra lux
float tolerance = 0.5;  // Grense for hva som teller som riktig eksponering

float forrigeLux = 0;  // Forrige lux verdi
float blitsGrense = 50.0;  // Grense for å oppdage blits (forandring i lux)
unsigned long blitsTid = 0;  // tid for blits

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Start lyssensor
  if (lightMeter.begin()) {
    Serial.println("BH1750 sensor startet.");
  } else {
    Serial.println("Kunne ikke starte BH1750 sensor.");
    while (1);  // Stopp hvis sensor ikke blir funnet
  }
  
  // Ber om verdier for innstillinger
  Serial.println("Skriv inn lukkertid (t) og ISO (S) i følgende format:");
  Serial.println("'t=1/250 S=100'");
}

void loop() {
  if (Serial.available() > 0) {
    // Leser input
    String input = Serial.readStringUntil('\n');  
    input.trim();

    // Deler opp input
    int tPos = input.indexOf("t=");
    int SPos = input.indexOf("S=");

    if (tPos != -1) {
      // finn og float verdi for t
      String tInput = input.substring(tPos + 2);
      tvalgt = parseFraction(tInput);
      Serial.print("Valgt lukkertid: 1/");
      Serial.println(1/tvalgt);
    }

    if (SPos != -1) {
      // finn og int verdi for S
      String SInput = input.substring(SPos + 2);
      Svalgt = SInput.toInt();
      Serial.print("Valgt ISO: ");
      Serial.println(Svalgt);
    }

    Serial.println("Venter på blits...");

    while (true) {
      E = lightMeter.readLightLevel();  // Leser luxverdi

      // Se etter blits (økt lux)
      if (E > forrigeLux + blitsGrense) {
        blitsTid = millis();
        Serial.print("Blits gir luxverdi: ");
        Serial.print(E);
        Serial.print(" på motivet");
        break;  // Gå ut av loop når blits har blitt oppdaget
      }

      forrigeLux = E;
      delay(100);
    }

    // Finn EV utifra lux-målingen:
    EVlux = calculateEVlux(E);
    Serial.print(" - EV fra lux: ");
    Serial.println(EVlux);

    // Ser etter blender som gir lik EV
    float closestEV = -1;
    float closestN = -1;
    
    for (int i = 0; i < 12; i++) {
      float EVcalc = calculateEVs(N[i], tvalgt, Svalgt);
      if (abs(EVlux - EVcalc) < tolerance) {
        closestEV = EVcalc;
        closestN = N[i];
      }
    }

    if (closestN != -1) {
      Serial.print("Blenderverdien for å oppnå ønsket EV: f/");
      Serial.println(closestN);
    } else {
      Serial.println("Ingen passende blenderverdi funnet.");
    }
    
    // Spør på nytt etter t og S for å gi ny utregning
    Serial.println("Skriv inn t og S igjen i samme format (f.eks., t=1/250 S=100):");
  }

  delay(500);
}

// gjøre input-verdier brukbare
float parseFraction(String frac) {
  int slashIndex = frac.indexOf('/');
  if (slashIndex != -1) {
    String numeratorStr = frac.substring(0, slashIndex);
    String denominatorStr = frac.substring(slashIndex + 1);

    float numerator = numeratorStr.toFloat();
    float denominator = denominatorStr.toFloat();
    
    if (denominator != 0) {
      return numerator / denominator;
    }
  }
  
  return frac.toFloat();
}

// regne ut EV fra lux
float calculateEVlux(float E) {
  float EV_lux = 4.521185 * log(E) + -14.490967;
  return EV_lux;
}

// regne ut EV fra kamerainnstillinger 
float calculateEVs(float N, float t, int S) {
  return log2((N * N) / (t * (S / 100.0)));
}
