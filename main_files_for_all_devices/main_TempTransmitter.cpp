#include <Wire.h>
#include <Adafruit_BME280.h>
#include <U8g2lib.h>
#include <esp_sleep.h>

#include "loraRadio.h"
#include "loraEncryption.h"
#include "loraTransport.h"

#define BATTERY_PIN 35
#define BUTTON_PIN 4
#define BASE_STATION_ADDRESS 1  // adres odbiornika

Adafruit_BME280 bme;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
bool displayActive = false;

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(BUTTON_PIN, INPUT);

  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  bool wokenByButton = wakeupReason == ESP_SLEEP_WAKEUP_EXT0;

  if (!bme.begin(0x76)) {
    Serial.println("Nie znaleziono BME280.");
    while (1);
  }
  Serial.println("BME280 OK");

  if (wokenByButton) {
    u8g2.begin();
    displayActive = true;
    Serial.println("OLED aktywowany (wybudzenie przez przycisk)");
  }

  // Inicjalizacja radia i transportu
  if (!Lora::Radio::init()) {
    Serial.println("Błąd inicjalizacji radia");
    while (1);
  }
  Lora::Encryption::init();
  Lora::start_radio();

  // Pomiar danych
  float temperature = bme.readTemperature() - 6;
  float humidity = bme.readHumidity();
  uint32_t analogValue = analogRead(BATTERY_PIN);
  float voltage = analogValue * (3.3 / 4095.0) * 2;
  int batteryPercentage = (int)((voltage - 2.82) / (3.93 - 2.82) * 100.0);

  char payload[16];
  snprintf(payload, sizeof(payload), "%05.1f%05.1f%05.2f", temperature, humidity, voltage);  // 15 znaków

  bool result = Lora::send_with_ack(BASE_STATION_ADDRESS, (uint8_t *)payload, 15);
  Serial.println(result ? "📡 Wysłano z powodzeniem" : "Błąd wysyłania");

  if (displayActive) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, 16);
    u8g2.print("Temp: "); u8g2.print(temperature); u8g2.print(" C");
    u8g2.setCursor(0, 32);
    u8g2.print("Wilg: "); u8g2.print(humidity); u8g2.print(" %");
    u8g2.setCursor(0, 48);
    u8g2.print("Bateria: "); u8g2.print(voltage, 2); u8g2.print("V (" );
    u8g2.print(batteryPercentage); u8g2.print("%)");
    u8g2.sendBuffer();
    delay(5000);
  }

  enterDeepSleep();
}

void loop() {}

void enterDeepSleep() {
  Serial.println("Przechodzenie w deep sleep");

  if (displayActive) {
    u8g2.clear();
    u8g2.sendBuffer();
    u8g2.setPowerSave(1);
  }

  esp_sleep_enable_timer_wakeup(59 * 1000000);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 1);
  esp_deep_sleep_start();
}
