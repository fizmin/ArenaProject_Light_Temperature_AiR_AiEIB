#ifndef LORA_ROUTING_H
#define LORA_ROUTING_H

#include <Arduino.h>
#include "loraRadio.h"
#include "params/user_params.h"
#include "params/frame_spec.h"

namespace Lora{
    namespace Routing{

        enum Receive_result{
            ROUTING_GOT_NOTHING =       0x00,
            ROUTING_RECEIVED =          0x01,
            ROUTING_FORWARDED =         0x03,
            ROUTING_GOT_BAD_ADDRESS =   0x04,
            ROUTING_TTL_EXPIRED =       0x05,
        };


        //wysyła ramkę do zadanego adresu. nadaje ramce cały segment adresu
        void send(uint8_t destination, uint8_t *frame, uint8_t *frame_size);

        //zczytuje radio, zależnie od adresów przesyła dalej lub odbiera
        //Zwraca: informację co udało się zrobić. Ewentualne dane lądują w 'buf' (gdy ROUTING_RECEIVED)
        Receive_result try_receive(uint8_t *buf, uint8_t *frame_size);

        //pisze na konsoli informacje o adresie i routing table
        void print_info();


    //implementacja:

        void send(uint8_t destination, uint8_t *frame, uint8_t *frame_size)
        {
            frame[FRAME_POS_SEND_ADDR] = ADDRESS;
            frame[FRAME_POS_DEST_ADDR] = destination;
            frame[FRAME_POS_NEXT_ADDR] = routing_table[destination];

            Radio::send_bytes(frame, *frame_size);
        }

        //podaje dalej ramkę, zależnie od jej segmentu adresu
        Receive_result _forward(uint8_t *frame, uint8_t *frame_size)
        {
            if(frame[FRAME_POS_TTL] <= 0)
                return ROUTING_TTL_EXPIRED;
            
            frame[FRAME_POS_TTL] -= 1;
            frame[FRAME_POS_NEXT_ADDR] = routing_table[frame[FRAME_POS_DEST_ADDR]]; //ustawianie następnego node'a, zależnie od adresata

            vTaskDelay(10); //raczej niepotrzebne

            Radio::send_bytes(frame, *frame_size);

            if(DEBUG)
            {
                Serial.print("[routing] Forwarded to: ");
                Serial.print(frame[FRAME_POS_NEXT_ADDR]);
                Serial.print("; Originaly from: ");
                Serial.println(frame[FRAME_POS_SEND_ADDR]);
            }
                
            return ROUTING_FORWARDED;
        }

        Receive_result try_receive(uint8_t *buf, uint8_t *frame_size)
        {
            int32_t rssi; //nieużywana zmienna
            *frame_size = Radio::try_receive_bytes(buf, &rssi);

            if(*frame_size > 0 && buf[FRAME_POS_NEXT_ADDR] == ADDRESS) //sprawdza czy powinien cokolwiek robić
            {
                if(buf[FRAME_POS_DEST_ADDR] == ADDRESS) //jest docelowym adresatem
                {
                    return ROUTING_RECEIVED;
                }
                else if(buf[FRAME_POS_DEST_ADDR] < NET_SIZE) //powinien podać dalej
                {
                    return _forward(buf, frame_size);
                }
                else //ten błąd nie powinien występować, ale nigdy nie wiadomo...
                {
                    return ROUTING_GOT_BAD_ADDRESS;
                }
            }

            return ROUTING_GOT_NOTHING;
        }

        void print_info()
        {
            Serial.print("Device adress: ");
            Serial.println(ADDRESS);
            Serial.println("Routing table:");
            for (int i = 0; i < NET_SIZE; i++)
            {
                Serial.print(i);
                Serial.print(" Sending to: ");
                Serial.println(routing_table[i]);
            }
        }

    }
}



#endif