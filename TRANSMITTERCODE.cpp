#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "DHT.h"

// -------- DHT11 --------
#define DHTPIN 4          // D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -------- INA219 --------
Adafruit_INA219 ina219;

// -------- LoRa Pins --------
#define LORA_SS 5         // D5
#define LORA_RST 14       // D14
#define LORA_DIO0 2       // D2

// -------- Relay (Optional for testing) --------
#define RELAY_PIN 25      // D25

// -------- Safety Thresholds --------
float TEMP_LIMIT = 45.0;
float CURRENT_LIMIT = 1.2;
float MIN_VOLTAGE = 3.5;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("EV Battery Monitoring System Starting...");

  // -------- DHT --------
  dht.begin();

  // -------- INA219 --------
  Wire.begin(21, 22);   // SDA = D21, SCL = D22
  if (!ina219.begin()) {
    Serial.println("INA219 Failed!");
    while (1);
  }

  // -------- Relay --------
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Charging ON initially

  // -------- LoRa --------
  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa Failed!");
    while (1);
  }

  Serial.println("System Ready");
}

void loop() {

  // -------- Read DHT --------
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    temperature = 0;
  }

  // -------- Read INA219 --------
  float voltage = ina219.getBusVoltage_V();
  float current = ina219.getCurrent_mA() / 1000.0;

  // -------- Safety Logic --------
  bool unsafe = false;

  if (temperature > TEMP_LIMIT ||
      current > CURRENT_LIMIT ||
      voltage < MIN_VOLTAGE) {
    unsafe = true;
  }

  String status;

  if (unsafe) {
    digitalWrite(RELAY_PIN, LOW);   // Disconnect charging
    status = "DISCONNECTED";
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // Charging allowed
    status = "NORMAL";
  }

  // -------- Serial Monitor --------
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C | Voltage: ");
  Serial.print(voltage);
  Serial.print(" V | Current: ");
  Serial.print(current);
  Serial.print(" A | Status: ");
  Serial.println(status);

  // -------- Send via LoRa --------
  String data = String(temperature) + "," +
                String(current) + "," +
                String(voltage) + "," +
                status;

  LoRa.beginPacket();
  LoRa.print(data);
  LoRa.endPacket();

  delay(1000);
}