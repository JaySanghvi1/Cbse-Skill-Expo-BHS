#include <SoftwareSerial.h>

#define trigPin 2
#define echoPin 3
#define mq2Pin A0
#define ledPin 6
#define buzzerPin 7

SoftwareSerial bluetooth(4, 5); // RX, TX

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(mq2Pin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  bluetooth.begin(9600);

  Serial.println("Weather Station Started");
  bluetooth.println("मौसम केंद्र प्रारंभ हुआ | Weather Station Started");
}

void loop() {
  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  float distance = duration * 0.034 / 2;

  // Read gas
  int mq2Value = analogRead(mq2Pin);
  float gasLevel = map(mq2Value, 0, 1023, 0, 100);

  float temperature = 28.5; // Dummy values
  float humidity = 62.0;

  String status = "";
  String statusHindi = "";

  // Default: safe
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // VALID DISTANCE
  if (distance > 0 && distance < 400) {
    if (distance > 10) {
      status = "✅ Safe";
      statusHindi = "सुरक्षित";
    } else if (distance > 6) {
      status = "⚠️ Yellow Alert (Evacuate)";
      statusHindi = "⚠️ पीला अलर्ट (स्थान खाली करें)";
    } else {
      status = "🔴 Severe Flood Alert!";
      statusHindi = "🔴 गंभीर बाढ़!";
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, HIGH); // Only here buzzer ON
    }
  } else {
    status = "📡 No reading or out of range";
    statusHindi = "📡 रीडिंग नहीं मिली";
  }

  // Print to serial
  Serial.println("------ WEATHER STATION ------");
  Serial.print("Flood Distance: "); Serial.print(distance); Serial.println(" cm");
  Serial.print("Flood Status: "); Serial.println(status);
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Gas Level (MQ-2): "); Serial.print(gasLevel); Serial.println(" %");
  Serial.println();

  // Send to Bluetooth
  bluetooth.println("---- मौसम केंद्र डेटा ----");
  bluetooth.print("बाढ़ दूरी: "); bluetooth.print(distance); bluetooth.println(" सेमी");
  bluetooth.print("स्थिति: "); bluetooth.println(statusHindi);
  bluetooth.print("तापमान: "); bluetooth.print(temperature); bluetooth.println(" °से");
  bluetooth.print("नमी: "); bluetooth.print(humidity); bluetooth.println(" %");
  bluetooth.print("गैस स्तर: "); bluetooth.print(gasLevel); bluetooth.println(" %");
  bluetooth.println();

  delay(2000);
}
