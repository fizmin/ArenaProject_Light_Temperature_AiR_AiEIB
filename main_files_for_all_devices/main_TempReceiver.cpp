#include <Wire.h>       //Bibliotekta obsługująca komunikację I2C
#include <U8g2lib.h>    // Biblioteka do obsługi wyświetlacza OLED
#include <ModbusRTU.h>   // Biblioteka implementująca protokół Modbus RTU
#include "loraRadio.h"   // Obsługa radia LoRa (fizyczna warstwa komunikacji)
#include "loraEncryption.h"   // Biblioteka szyfrowania wiadomości
#include "loraTransport.h"    // Obsługa protokołu transportowego (ramki, ACK)

#define SLAVE_ID 1    // Identyfikator odbiornika w sieci Modbus (adres slave)
#define PIN_BUTTON 4  // Pin przycisku wybudzającego wyświetlacz 

// Utworzenie obiektu portu szeregowego dla Modbus (używany UART1 ESP32)
HardwareSerial ModbusSerial(1);
ModbusRTU mb; // Obiekt protokołu Modbus

// Obiekt wyświetlacza OLED 128x64 z kontrolerem SSD1306
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Zmienne globalne
float lastTemp = 0.0;   // Ostatnia odebrana temperatura (°C)
float lastHum = 0.0;    // Ostatnia odebrana wilgotność (%)
float lastVolt = 0.0;   // Ostatnie odebrane napięcie baterii (V)
int batteryPercent = 0;   // Procent baterii nadajnika w %
bool displayActive = false;  // Flaga, czy ekran jest aktywny
unsigned long displayStartTime = 0;  // Czas aktywacji ekranu
uint16_t data[3] = {0, 0, 0};   // Tablica danych do wysyłki przez Modbus (temperatura, wilgotność, bateria)


void setup() {      // Funkcja setup
  Serial.begin(115200);    // Uruchomienie monitora szeregowego (debug)
  ModbusSerial.begin(19200, SERIAL_8N1, 13, 14);  // Inicjalizacja komunikacji Modbus RTU (prędkość 19200, format 8N1, RX=13, TX=14)
  mb.begin(&ModbusSerial);    // Start stosu Modbus
  mb.slave(SLAVE_ID);   // Ustawienie adresu slave = 1
  mb.addHreg(100);      // Rejestr Holding Register dla temperatury
  mb.addHreg(101);      // Rejestr dla wilgotności
  mb.addHreg(102);      // Rejestr dla stanu baterii

 // Inicjalizacja komunikacji radiowej LoRa
  if (!Lora::Radio::init()) {
    Serial.println("Błąd inicjalizacji radia");
    while (1);    // Zatrzymaj pracę, jeśli radio się nie uruchomiło
  }
  Lora::Encryption::init(); // Uruchomienie modułu szyfrowania AES
  Lora::start_radio();      // Start procesu nasłuchiwania pakietów

  pinMode(PIN_BUTTON, INPUT_PULLUP); // Konfiguracja pinu przycisku (aktywny po zwarciu do GND)
  u8g2.begin(); // Inicjalizacja wyświetlacza OLED
}

void loop() {   //Funkcja loop
  mb.task();    // Obsługa żądań Modbus (konieczne do działania protokołu)

  uint8_t buf[64];   // Bufor na odebrane dane
  uint8_t size = 0;  // Rozmiar odebranej wiadomości
  uint8_t sender = 3; // Adres nadawcy (czujnik temperatury)

  // Próba odbioru pakietu LoRa
  if (Lora::try_receive(&sender, buf, &size) == Lora::RECEIVED) {
    // Debug: wypisanie adresów ramki
    Serial.print("DEST_ADDR: "); Serial.println(buf[FRAME_POS_DEST_ADDR]);
    Serial.print("NEXT_ADDR: "); Serial.println(buf[FRAME_POS_NEXT_ADDR]);
    Serial.print("SEND_ADDR: "); Serial.println(buf[FRAME_POS_SEND_ADDR]);
    Serial.print("Odebrano pakiet od "); Serial.println(sender);

    // Deszyfrowanie danych (ciąg znaków ASCII do 16 bajtów)
    char decrypted[17];
    memcpy(decrypted, buf, min(size, (uint8_t)16));
    decrypted[16] = '\0'; // Dodanie znaku końca stringa

    // Rozpakowanie danych (format: temperatura, wilgotność, napięcie)
    sscanf(decrypted, "%5f%5f%5f", &lastTemp, &lastHum, &lastVolt);
    batteryPercent = (int)(((lastVolt - 2.82) / (3.93 - 2.82)) * 100.0);  // Obliczenie poziomu baterii w procentach

    Serial.print("Dane: "); Serial.println(decrypted); // Debug: wyświetlenie odebranych danych
  }
  else if (Lora::try_receive(&sender, buf, &size) == Lora::GOT_NOTHING) {
    // tu nic nie rób — nie wypisuj "DEST_ADDR", żeby nie drukować śmieci
  }

  // Obsługa przycisku – włączenie wyświetlacza po naciśnięciu
  if (digitalRead(PIN_BUTTON) == LOW) {
    displayActive = true;
    displayStartTime = millis();  // Zapisz czas włączenia
  }

  // Jeśli wyświetlacz aktywny – pokaż dane
  if (displayActive) {
    u8g2.clearBuffer();                 // Wyczyść bufor ekranu
    u8g2.setFont(u8g2_font_ncenB08_tr); // Ustaw czcionkę

    // Nagłówek w zależności od stanu baterii
    u8g2.setCursor(0, 16);
    if (batteryPercent > 20)
      u8g2.print("Ostatni odczyt:");
    else
      u8g2.print("NISKI POZ. BATERII");
    
    // Wyświetlanie temperatury
    u8g2.setCursor(0, 32);  u8g2.print("Temp: "); u8g2.print(lastTemp); u8g2.print(" C");

    // Wyświetlanie wilgotności
    u8g2.setCursor(0, 48);u8g2.print("Wilg: "); u8g2.print(lastHum); u8g2.print(" %");

    // Wyświetlanie napięcia i procentu baterii
    u8g2.setCursor(0, 64);  u8g2.print("Bateria: "); u8g2.print(lastVolt, 2);
    u8g2.print("V ("); u8g2.print(batteryPercent); u8g2.print("%)");

    u8g2.sendBuffer();  // Prześlij zawartość bufora do wyświetlacza
  }

  // Wyłączenie wyświetlacza po 5 sekundach
  if (displayActive && millis() - displayStartTime > 5000) {
    displayActive = false;
    u8g2.clearDisplay(); // Wyczyść ekran
  }
  // Przygotowanie danych do Modbus (int16 -> *100 dla zachowania części dziesiętnej)
  data[0] = (uint16_t)(lastTemp * 100);  // np. 23.45°C zapisane jako 2345
  data[1] = (uint16_t)(lastHum * 100);  // np. 56.78% zapisane jako 5678
  data[2] = batteryPercent;            // Poziom baterii w %

  // Wysłanie danych do rejestrów Modbus
  mb.Hreg(100, data[0]); // temperatura
  mb.Hreg(101, data[1]); // wilgotność
  mb.Hreg(102, data[2]); // bateria

  delay(500);   // Krótka przerwa (zmniejsza obciążenie procesora)
}
