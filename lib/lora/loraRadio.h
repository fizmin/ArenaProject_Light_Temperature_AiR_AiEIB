#ifndef LORA_RADIO_h
#define LORA_RADIO_h

#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h>
#include "params/hardware.h"
#include "params/user_params.h"

namespace Lora{
    namespace Radio{
        
        volatile bool is_sending = false; //jak radio wysyła, nie może jednocześnie słuchać

        uint64_t send_time;

        //Zwraca: true gdzy inicjalizacja się powiodła
        bool init()
        {
            SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
            LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
            if (!LoRa.begin(LoRa_frequency)) 
                return false;

            return true;
        }

        //Sprawdza czy LORA ma pakiety do odebrania
        //Zwraca: ilość odebranych bitów
        //        odebrane dane zapisywane są w parametrze buf
        uint16_t try_receive_bytes(uint8_t *buf, int32_t *RSSI)
        {
            if(is_sending)
                return 0;

            uint16_t packetSize = LoRa.parsePacket();
            if (packetSize) {
                uint16_t pos=0;
                
                while (LoRa.available()) {
                    buf[pos++] = LoRa.read();
                }

                *RSSI = LoRa.packetRssi();

                if(DEBUG)
                {
                    Serial.print("[radio] Got: ");
                    Serial.print(packetSize);
                    Serial.println(" bytes");
                }

                return pos;
            }

            return 0;
        }

        //Wysyła pakiet, nie sprawdza czy doszedł
        void send_bytes(uint8_t *buf, int32_t buf_size)
        {
            is_sending = true;

            if(DEBUG)
                send_time = millis();
            
            LoRa.beginPacket();
            LoRa.write(buf,buf_size);
            LoRa.endPacket();

            if(DEBUG)
            {
                send_time = millis()-send_time;
                Serial.print("[radio] Sent: ");
                Serial.print(buf_size);
                Serial.print(" bytes in ");
                Serial.print(send_time);
                Serial.println(" ms.");
            }

            is_sending = false;
        }
    }
}




#endif