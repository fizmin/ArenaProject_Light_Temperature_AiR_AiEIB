#include <Arduino.h>
#include "../lib/oled.h"
#include "../lib/lora/loraTransport.h"

#define CH0_ON_PIN    12
#define CH0_OFF_PIN   13
#define CH1_ON_PIN    34
#define CH1_OFF_PIN   15
#define TEST_PIN      25
#define BATTERY_ADC_PIN     35     // Pin do pomiaru napięcia
#define BATTERY_EN_PIN      14     // Pin do włączania dzielnika napięcia 
#define BATTERY_RAW_MIN     1816   // ~3,25V na baterii
#define BATTERY_RAW_MAX     2416   // ~4,25V na baterii

//maska jako parametr definiujący, co jest w stanie wybudzić naszego esp32 w funkcji  esp_sleep_enable_ext1_wakeup
uint64_t maska = (1ULL << CH0_ON_PIN) | (1ULL << CH0_OFF_PIN) | (1ULL << CH1_ON_PIN) | (1ULL << CH1_OFF_PIN) | (1ULL << TEST_PIN);

//adres radiowy stacji do której ma być wysyłane
#define RELAY_STATION_ADRESS 0


//program wykorzystuje deep sleep do oszczędzania energii. Zasypia zawsze na koniec pętli setup,
//a po obudzeniu przez przycisk zaczyna cały program od początku (od setup znowu).
//dlatego nie ma pętli loop.

void showBatteryLevel();

void setup() {

  //inicjalizacja obiektów

  Serial.begin(9600);

  Oled::init();

  showBatteryLevel(); //  Pokaż poziom baterii na starcie

  if(Lora::Radio::init())
        Serial.println("LoRa Started");
    else
        Serial.println("LoRa ERROR");

  Lora::Encryption::init();

  Lora::start_radio();

  //esp_sleep_enable_ext1_wakeup(0x40000F000,ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_sleep_enable_ext1_wakeup(maska,ESP_EXT1_WAKEUP_ANY_HIGH);

  //obsługa przycisków po obudzeniu

  delay(100);

  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  uint16_t pressed_pin = (log(GPIO_reason))/log(2);
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println(pressed_pin, 0);

  // [0,0], [0,1] - on, off przekaźnika 0
  // [1,0], [1,1] - on, off przekaźnika 1
  // [0] - test (chodzi o to żeby wiadomość miała 1 znak)

  switch(pressed_pin)
  {
    //kanał 0
    case CH0_ON_PIN:
      {byte msg[2] = {0,1};
      //Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));
        if(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2) > 0)
      {
        Oled::drawBigText(4,15,F("Relay 0"));
        Oled::drawBigText(4,40,F("ON"));
      }else
      {
        Oled::drawBigText(4,15,F("Relay 0"));
        Oled::drawBigText(4,40,F("Failed"));
      }
      }
    break;

    case CH0_OFF_PIN:
      {byte msg[2] = {0,0};
     //Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));
        if(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2) > 0)
      {
        Oled::drawBigText(4,15,F("Relay 0"));
        Oled::drawBigText(4,40,F("OFF"));
      }else
      {
        Oled::drawBigText(4,15,F("Relay 0"));
        Oled::drawBigText(4,40,F("Failed"));
      }
      }
    break;

    //kanał 1
    case CH1_ON_PIN:
      {byte msg[2] = {1,1};
      //Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));
        if(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2) > 0)
      {
        Oled::drawBigText(4,15,F("Relay 1"));
        Oled::drawBigText(4,40,F("ON"));
      }else
      {
        Oled::drawBigText(4,15,F("Relay 1"));
        Oled::drawBigText(4,40,F("Failed"));
      }
      }
    break;
  
    case CH1_OFF_PIN:
      {byte msg[2] = {1,0};
      //Serial.println(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2));
        if(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,2) > 0)
      {
        Oled::drawBigText(4,15,F("Relay 1"));
        Oled::drawBigText(4,40,F("OFF"));
      }else
      {
        Oled::drawBigText(4,15,F("Relay 1"));
        Oled::drawBigText(4,40,F("Failed"));
      }
      }
    break;

    //test
    case TEST_PIN:
      {byte msg[1] = {0};
      if(Lora::send_with_ack(RELAY_STATION_ADRESS,msg,1) > 0)
      {
        Oled::drawBigText(4,15,F("TEST OK"));
      }else
      {
        Oled::drawBigText(4,15,F("TEST"));
        Oled::drawBigText(4,40,F("Failed"));
      }
      
      
      }
    break;
  }

  delay(1000);

  Oled::clear();
  Oled::sleepDisplay(&Oled::display);
  

  esp_deep_sleep_start();
}

void loop() {
  //nic się nie dzieje
}

void showBatteryLevel() {
  pinMode(BATTERY_EN_PIN, OUTPUT);
  digitalWrite(BATTERY_EN_PIN, HIGH); // Włącz dzielnik napięcia

  delay(100); // Poczekaj na stabilizację napięcia

  int raw = analogRead(BATTERY_ADC_PIN);
  float percent = ((float)(raw - BATTERY_RAW_MIN) / (BATTERY_RAW_MAX - BATTERY_RAW_MIN)) * 100.0;

  if (percent > 100.0) percent = 100.0;
  if (percent < 0.0) percent = 0.0;

  Oled::clear();

  if (percent > 0.0)
    Oled::drawText(4, 0, "Bat: " + String((int)percent) + "%");
  else
    Oled::drawText(4, 0, "LOW BATTERY");

 // delay(3000); // Pokazuj przez 3 sekundy

  digitalWrite(BATTERY_EN_PIN, LOW); // Wyłącz dzielnik napięcia

}