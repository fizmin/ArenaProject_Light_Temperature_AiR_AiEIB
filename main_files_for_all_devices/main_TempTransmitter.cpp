#include <Wire.h>               // Obsługa komunikacji I2C (dla czujnika i wyświetlacza)
#include <Adafruit_BME280.h>    // Biblioteka dla czujnika środowiskowego BME280
#include <U8g2lib.h>            // Biblioteka graficzna dla wyświetlacza OLED
#include <esp_sleep.h>          // Obsługa trybów oszczędzania energii w ESP32

// Biblioteki LoRa – obsługa transmisji radiowej i szyfrowania
#include "loraRadio.h"
#include "loraEncryption.h"
#include "loraTransport.h"

// ====================== Definicje pinów i adresów ======================
#define BATTERY_PIN 35           // Wejście analogowe do pomiaru napięcia baterii
#define BUTTON_PIN 4             // Pin przycisku do wybudzania urządzenia
#define BASE_STATION_ADDRESS 4   // Adres odbiornika (stacja bazowa)

// ====================== Obiekty globalne ======================
Adafruit_BME280 bme;     // Obiekt czujnika temperatury, wilgotności i ciśnienia
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE); // Obiekt wyświetlacza OLED
bool displayActive = false; // Flaga informująca czy ekran OLED ma być aktywny

// ====================== Funkcja setup ======================
void setup() {
  Serial.begin(115200); // Uruchomienie portu szeregowego do debugowania
  delay(100);           // Krótka przerwa na stabilizację

  pinMode(BUTTON_PIN, INPUT); // Ustawienie przycisku jako wejście

  // Sprawdzenie powodu wybudzenia mikrokontrolera (przycisk / timer)
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
  bool wokenByButton = (wakeupReason == ESP_SLEEP_WAKEUP_EXT0); // true, jeśli obudził przycisk

  // Inicjalizacja czujnika BME280 (adres 0x76 na magistrali I2C)
  if (!bme.begin(0x76)) {  
    Serial.println("Nie znaleziono czujnika BME280!");
    while (1); // Zatrzymaj program, jeśli czujnik nie działa
  }
  Serial.println("Czujnik BME280 OK");

  // Jeśli urządzenie zostało wybudzone przyciskiem – aktywuj ekran OLED
  if (wokenByButton) {
    u8g2.begin();            // Start obsługi wyświetlacza
    displayActive = true;    // Włącz flagę wyświetlacza
    Serial.println("OLED aktywowany (wybudzenie przez przycisk)");
  }

// ====================== Inicjalizacja radia LoRa ======================
  if (!Lora::Radio::init()) {   // Inicjalizacja modułu radiowego
    Serial.println("Błąd inicjalizacji radia LoRa");
    while (1); // Zatrzymaj program, jeśli nie działa radio
  }
  Lora::Encryption::init();   // Uruchomienie szyfrowania AES
  Lora::start_radio();        // Start odbiornika/wysyłki pakietów

  // ====================== Pobranie pomiarów ======================
  float temperature = bme.readTemperature() - 3; // Pomiar temperatury (z offsetem -3°C)
  float humidity = bme.readHumidity();           // Pomiar wilgotności (%)
  
  // Odczyt napięcia baterii (ADC -> napięcie w Voltach)
  uint32_t analogValue = analogRead(BATTERY_PIN);
  float voltage = analogValue * (3.3 / 4095.0) * 2; // Przeliczenie na napięcie [V]

  // Obliczenie procentowego stanu baterii (wzór kalibracyjny)
  int batteryPercentage = (int)((voltage - 2.82) / (3.93 - 2.82) * 100.0);

  // ====================== Przygotowanie danych ======================
  char payload[16]; // Bufor na wiadomość tekstową (16 bajtów = 15 znaków + \0)
  snprintf(payload, sizeof(payload), "%05.1f%05.1f%05.2f", 
           temperature, humidity, voltage); // Zapis wartości jako ciąg znaków

  // ====================== Wysyłanie danych ======================
  bool result = Lora::send_with_ack(BASE_STATION_ADDRESS, (uint8_t *)payload, 15);
  Serial.println(result ? "Wysłano z powodzeniem" : "Błąd wysyłania");

  // ====================== Obsługa wyświetlacza ======================
  if (displayActive) {
    u8g2.clearBuffer();                 // Wyczyść bufor ekranu
    u8g2.setFont(u8g2_font_ncenB08_tr); // Ustaw czcionkę

    // Wyświetlenie temperatury
    u8g2.setCursor(0, 16);  u8g2.print("Temp: "); u8g2.print(temperature); u8g2.print(" C");

    // Wyświetlenie wilgotności
    u8g2.setCursor(0, 32);  u8g2.print("Wilg: "); u8g2.print(humidity); u8g2.print(" %");

    // Wyświetlenie napięcia baterii i procentu
    u8g2.setCursor(0, 48);  u8g2.print("Bateria: "); 
    u8g2.print(voltage, 2); u8g2.print("V (" ); 
    u8g2.print(batteryPercentage); u8g2.print("%)");

    u8g2.sendBuffer();  // Wyślij dane do wyświetlacza
    delay(5000);        // Zatrzymaj ekran na 5 sekund
  }

  // ====================== Przejście w tryb oszczędzania energii ======================
  enterDeepSleep();
}

// ====================== Funkcja główna loop ======================
// Pusta – wszystkie operacje wykonywane są w setup(), 
// a po nich mikrokontroler usypia.
void loop() {}

// ====================== Funkcja przejścia w deep sleep ======================
void enterDeepSleep() {
  Serial.println("Przechodzenie w tryb deep sleep...");

  // Jeśli ekran był włączony – wyłącz go
  if (displayActive) {
    u8g2.clear();
    u8g2.sendBuffer();
    u8g2.setPowerSave(1); // Tryb oszczędzania energii dla OLED
  }

  // Ustaw budzenie po 59 sekundach (timer)
  esp_sleep_enable_timer_wakeup(59 * 1000000);

  // Ustaw budzenie także przez przycisk (przerwanie na GPIO4)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 1);

  // Uruchom tryb głębokiego snu – mikrokontroler zatrzyma pracę
  esp_deep_sleep_start();
}
