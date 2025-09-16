#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

// Piny przekaźników
#define RELAY_0 12
#define RELAY_1 14

// Adresy węzłów
#define REMOTE_ADRESS 1
#define BACKUP_CONTROLLER_ADRESS 2
#define TEMPERATUR_SENSOR_ADRESS 3
#define TEMP_DATA_RX_ADRESS 4

void setup() {
  Serial.begin(9600);
  Oled::init();

  pinMode(RELAY_0, OUTPUT);
  pinMode(RELAY_1, OUTPUT);

  if (Lora::Radio::init())
    Serial.println("LoRa Started");
  else
    Serial.println("LoRa ERROR");

  Lora::Encryption::init();
  Lora::start_radio();
}

void loop() {
  uint8_t msg[MAX_FRAME_SIZE];
  uint8_t msg_size;
  uint8_t addr;

  int received = Lora::try_receive(&addr, msg, &msg_size);

  if (received == Lora::RECEIVED) {
    Serial.print("Received packet from ");
    Serial.print(addr);
    Serial.print(": ");
    for (int i = 0; i < msg_size; i++) {
      Serial.print(msg[i]);
      Serial.print(" ");
    }
    Serial.println();

    if (addr == REMOTE_ADRESS) {
      // Obsługa przekaźników
      if (msg_size > 1) {
        if (msg[0] == 0) {
          if (msg[1] == 0) {
            digitalWrite(RELAY_0, LOW);
            Oled::drawBigText(0,20,F("RELAY_0 LOW"));
          } else {
            digitalWrite(RELAY_0, HIGH);
            Oled::drawBigText(0,20,F("RELAY_0 HIGH"));
          }

        } else {
          if (msg[1] == 0) {
            digitalWrite(RELAY_1, LOW);
            Oled::drawBigText(0,20,F("RELAY_1 LOW"));
          } else {
            digitalWrite(RELAY_1, HIGH);
            Oled::drawBigText(0,20,F("RELAY_1 HIGH"));
          }
        }
      }
    } 
    else if (addr == BACKUP_CONTROLLER_ADRESS ||
             addr == TEMPERATUR_SENSOR_ADRESS ||
             addr == TEMP_DATA_RX_ADRESS) 
    {
      // Forwardujemy pakiet dalej
      Lora::Routing::Receive_result forward_result = Lora::Routing::_forward(msg, &msg_size);
      Serial.print("Forward result: ");
      Serial.println(forward_result);
    } 
    else {
      // Obsługa błędnego adresu
      Serial.println("Received packet with unknown or invalid address.");
    }
  }

  delay(1000);

  Oled::clear();
}
