#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT Sensor
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensor Pins
#define MQ_PIN A0
#define FLOOD_PIN A1
#define BUZZER_PIN 4
#define LED_PIN 5

// MQ2 Constants
#define GAS_LPG     0
#define GAS_CO      1
#define GAS_SMOKE   2

float Ro = 10.0;
float LPGCurve[3]   = {2.3, 0.21, -0.47};
float COCurve[3]    = {2.3, 0.72, -0.34};
float SmokeCurve[3] = {2.3, 0.53, -0.44};

float MQRead(int pin) {
  float rs = 0;
  for (int i = 0; i < 5; i++) {
    rs += analogRead(pin);
    delay(50);
  }
  return rs / 5;
}

float MQGetGasPercentage(float rs_ro_ratio, int gas_id) {
  float x = 0;
  if (gas_id == GAS_LPG)
    x = ((log10(rs_ro_ratio) - LPGCurve[1]) / LPGCurve[2]);
  else if (gas_id == GAS_CO)
    x = ((log10(rs_ro_ratio) - COCurve[1]) / COCurve[2]);
  else if (gas_id == GAS_SMOKE)
    x = ((log10(rs_ro_ratio) - SmokeCurve[1]) / SmokeCurve[2]);
  return pow(10, x);
}

void displayOLED(float temp, float hum, float lpg, float co, float smoke, bool flood, String airStatus) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("T: "); display.print(temp); display.print("C ");
  display.print("H: "); display.print(hum); display.println("%");

  display.setCursor(0, 16);
  display.print("LPG: "); display.print(lpg); display.print(" CO: "); display.print(co);

  display.setCursor(0, 32);
  display.print("Smoke: "); display.print(smoke);

  display.setCursor(0, 48);
  display.print("Flood: "); display.print(flood ? "YES" : "NO");
  display.print(" AQ: "); display.println(airStatus);

  display.display();
}

void sendToSerial(float temp, float hum, float lpg, float co, float smoke, bool flood, String airStatus) {
  Serial.print("Temp: "); Serial.print(temp); Serial.print("C | ");
  Serial.print("Humidity: "); Serial.print(hum); Serial.print("% | ");
  Serial.print("LPG: "); Serial.print(lpg); Serial.print(" ppm | ");
  Serial.print("CO: "); Serial.print(co); Serial.print(" ppm | ");
  Serial.print("Smoke: "); Serial.print(smoke); Serial.print(" ppm | ");
  Serial.print("Flood: "); Serial.print(flood ? "YES" : "NO"); Serial.print(" | ");
  Serial.print("Air: "); Serial.println(airStatus);
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true); // hang
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Initializing...");
  display.display();
  delay(2000);
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  int floodValue = analogRead(FLOOD_PIN); // 0-1023
  bool floodDetected = floodValue < 300;

  float rs = MQRead(MQ_PIN);
  float rs_ro = rs / Ro;

  float lpg = MQGetGasPercentage(rs_ro, GAS_LPG);
  float co = MQGetGasPercentage(rs_ro, GAS_CO);
  float smoke = MQGetGasPercentage(rs_ro, GAS_SMOKE);

  String airStatus = "Good";
  bool gasDanger = false;

  if (lpg > 1000 || co > 30 || smoke > 300) {
    airStatus = "DANGER";
    gasDanger = true;
  } else if (lpg > 400 || co > 10 || smoke > 150) {
    airStatus = "Moderate";
  }

  // Alert logic
  if (floodDetected || gasDanger) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }

  displayOLED(temperature, humidity, lpg, co, smoke, floodDetected, airStatus);
  sendToSerial(temperature, humidity, lpg, co, smoke, floodDetected, airStatus);

  delay(2000);
}
