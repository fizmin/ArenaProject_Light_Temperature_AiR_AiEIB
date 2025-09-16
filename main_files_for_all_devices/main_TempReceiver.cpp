#include <Wire.h>
#include <U8g2lib.h>
#include <ModbusRTU.h>
#include "loraRadio.h"
#include "loraEncryption.h"
#include "loraTransport.h"

#define SLAVE_ID 1
#define PIN_BUTTON 4

HardwareSerial ModbusSerial(1);
ModbusRTU mb;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

float lastTemp = 0.0;
float lastHum = 0.0;
float lastVolt = 0.0;
int batteryPercent = 0;
bool displayActive = false;
unsigned long displayStartTime = 0;

uint16_t data[3] = {0, 0, 0};

void setup() {
  Serial.begin(115200);
  ModbusSerial.begin(19200, SERIAL_8N1, 13, 14);
  mb.begin(&ModbusSerial);
  mb.slave(SLAVE_ID);
  mb.addHreg(100);
  mb.addHreg(101);
  mb.addHreg(102);

  if (!Lora::Radio::init()) {
    Serial.println("Błąd inicjalizacji radia");
    while (1);
  }
  Lora::Encryption::init();
  Lora::start_radio();

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  u8g2.begin();
}

void loop() {
  mb.task();

  uint8_t buf[64];
  uint8_t size = 0;
  uint8_t sender = 3;

  if (Lora::try_receive(&sender, buf, &size) == Lora::RECEIVED) {
    Serial.print("DEST_ADDR: ");
    Serial.println(buf[FRAME_POS_DEST_ADDR]);
    Serial.print("NEXT_ADDR: ");
    Serial.println(buf[FRAME_POS_NEXT_ADDR]);
    Serial.print("SEND_ADDR: ");
    Serial.println(buf[FRAME_POS_SEND_ADDR]);
    Serial.print("Odebrano pakiet od ");
    Serial.println(sender);

    char decrypted[17];
    memcpy(decrypted, buf, min(size, (uint8_t)16));
    decrypted[16] = '\0';

    sscanf(decrypted, "%5f%5f%5f", &lastTemp, &lastHum, &lastVolt);
    batteryPercent = (int)(((lastVolt - 2.82) / (3.93 - 2.82)) * 100.0);

    Serial.print("Dane: ");
    Serial.println(decrypted);
  }

  if (digitalRead(PIN_BUTTON) == LOW) {
    displayActive = true;
    displayStartTime = millis();
  }

  if (displayActive) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    u8g2.setCursor(0, 16);
    if (batteryPercent > 20)
      u8g2.print("Ostatni odczyt:");
    else
      u8g2.print("NISKI POZ. BATERII");

    u8g2.setCursor(0, 32);
    u8g2.print("Temp: "); u8g2.print(lastTemp); u8g2.print(" C");

    u8g2.setCursor(0, 48);
    u8g2.print("Wilg: "); u8g2.print(lastHum); u8g2.print(" %");

    u8g2.setCursor(0, 64);
    u8g2.print("Bateria: "); u8g2.print(lastVolt, 2);
    u8g2.print("V ("); u8g2.print(batteryPercent); u8g2.print("%)");

    u8g2.sendBuffer();
  }

  if (displayActive && millis() - displayStartTime > 5000) {
    displayActive = false;
    u8g2.clearDisplay();
  }

  data[0] = (uint16_t)(lastTemp * 100);
  data[1] = (uint16_t)(lastHum * 100);
  data[2] = batteryPercent;

  mb.Hreg(100, data[0]);
  mb.Hreg(101, data[1]);
  mb.Hreg(102, data[2]);

  delay(500);
}
