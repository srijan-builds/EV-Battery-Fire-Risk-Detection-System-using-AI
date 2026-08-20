#include <SPI.h>
#include <LoRa.h>

#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2

void setup() {
  Serial.begin(9600);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed");
    while (1);
  }
  Serial.println("LoRa receiver ready");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) return;

  String received = "";
  while (LoRa.available()) {
    received += (char)LoRa.read();
  }

  // Parse CSV: temp,current,voltage,status
  int first = received.indexOf(',');
  int second = received.indexOf(',', first + 1);
  int third = received.indexOf(',', second + 1);

  if (first > 0 && second > first && third > second) {
    float temp = received.substring(0, first).toFloat();
    float current = received.substring(first + 1, second).toFloat();
    float voltage = received.substring(second + 1, third).toFloat();
    String status = received.substring(third + 1);

    // Output as single CSV line for backend
    Serial.print(temp, 2);
    Serial.print(",");
    Serial.print(current, 2);
    Serial.print(",");
    Serial.print(voltage, 2);
    Serial.print(",");
    Serial.println(status);
  }
}