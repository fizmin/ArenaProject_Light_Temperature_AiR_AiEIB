#ifndef ENC_H
#define ENC_H

#include <Arduino.h>
#include "params/user_params.h"
#include "params/frame_spec.h"
#include "params/hardware.h"
#include "../aes/Cipher.h"
#include "../../src/secrets.h"         //jeżeli nie ma pliku secrets.h, postępuj zgodnie z instrukcjami w secrets_TEMPLATE.h

namespace Lora{
    namespace Encryption{

        int32_t send_tokens[NET_SIZE];
        int32_t recv_tokens[NET_SIZE];

        byte enc_buf[MAX_FRAME_SIZE];

        Cipher * cipher = new Cipher();

        void init()
        {
            cipher->setKey(AES_KEY);

            randomSeed(analogRead(RANDOM_SRC_PIN));

            if(DEBUG)
                Serial.print("[encryption] Set recv tokens: ");
            for (size_t i = 0; i < NET_SIZE; i++)
            {
                send_tokens[i] = 0;
                //recv_tokens[i] = 2;
                recv_tokens[i] = random(0,TOKEN_MAX);
                
                if(DEBUG)
                {
                    Serial.print(recv_tokens[i]);
                    Serial.print(" ");
                }
            }
            
            if(DEBUG)
                Serial.println("");
            
        }

        void _split_to_bytes(uint8_t *array, uint32_t number, uint8_t num_bytes) {
            for (int i = num_bytes - 1; i >= 0; i--) {
                array[i] = number & 0xFF; // get the least significant byte of number
                number >>= 8; // shift number to the right by one byte
            }
        }

        uint32_t _merge_from_bytes(uint8_t *array, uint8_t num_bytes) {
            uint32_t number = 0;
            for (int i = 0; i < num_bytes; i++) {
                number <<= 8; // shift number to the left by one byte
                number |= array[i]; // add the current byte to the number
            }
            return number;
        }

        void encrypt(uint8_t *input, uint8_t in_size, uint8_t*output, uint8_t *out_size, uint8_t dest_address)
        {
            send_tokens[dest_address] = (send_tokens[dest_address]+1) % TOKEN_MAX;
            uint32_t token = send_tokens[dest_address];

            //składanie wiadomości
            _split_to_bytes(enc_buf,token, TOKEN_SIZE);
            memcpy(enc_buf+TOKEN_SIZE, input, in_size);

            cipher->encryptBytes(enc_buf, in_size+TOKEN_SIZE, output, (int8_t*) out_size);
        }

        // uint8_t _dist_a_to_b(uint32_t a, uint32_t b, uint32_t overflow) {
        //     uint64_t big_b = b;
        //     if(b < a)
        //         big_b += overflow;
        //     a = big_b - a;
        //     return a;
        // }

        bool _validate_token(uint32_t token, byte address)
        {

            //uint32_t diff = _dist_a_to_b(recv_tokens[address], token, TOKEN_MAX);
            int64_t diff = token - recv_tokens[address];    //wywali przy przepełnieniu (TOKEN_MAX), ale nie szkodzi. jest przynajmniej czytelnie.

            if(DEBUG)
            {
                Serial.print("[encryption] Validating: ");
                Serial.print(recv_tokens[address]);
                Serial.print(" - ");
                Serial.print(token);
                Serial.print(" = ");
                Serial.print(diff);
            }

            if(diff < TOKEN_TRESHOLD && diff > 0)
            {
                if(DEBUG)
                    Serial.println(" Success.");
                recv_tokens[address] = token;
                return true;
            }

            if(DEBUG)
                Serial.println(" Failed.");

            return false;
        }

        //zwraca false, gdy token jest zły
        bool decrypt(byte *input, uint8_t in_size, byte*output, uint8_t *out_size, byte sender_address)
        {
            uint8_t eb_size = 0;
            cipher->decryptBytes(input, in_size, enc_buf, (int8_t*) &eb_size);

            if(eb_size == 0)
                return true;    //wiadomość jest pusta, więc nawet nie trzeba sprawdzać tokena, ale nie ma błędu

            uint32_t token = _merge_from_bytes(enc_buf,TOKEN_SIZE);

            if(!_validate_token(token, sender_address))
                return false;

            *out_size = eb_size - TOKEN_SIZE;

            memcpy(output, enc_buf+TOKEN_SIZE, *out_size);

            return true;
        }

        void get_next_recv_token(uint8_t *token_as_arr, uint8_t address)
        {
            _split_to_bytes(token_as_arr, ((recv_tokens[address])+1) % TOKEN_MAX, TOKEN_SIZE);
        }

        void set_send_token(uint8_t *token_as_arr, uint8_t address)
        {
            send_tokens[address] = _merge_from_bytes(token_as_arr, TOKEN_SIZE);
        }

        void swap_token(uint8_t *msg, uint8_t msg_size, uint8_t address, uint8_t *new_token_as_arr)
        {
            uint8_t eb_size = 0;
            cipher->decryptBytes(msg, msg_size, enc_buf, (int8_t*) &eb_size);
            
            memcpy(enc_buf, new_token_as_arr, TOKEN_SIZE);
            send_tokens[address] = _merge_from_bytes(new_token_as_arr, TOKEN_SIZE);

            int8_t cipher_size = 0;
            cipher->encryptBytes(enc_buf,eb_size,msg, &cipher_size);
            if(DEBUG)
                Serial.print("[encryption] Swapped token to: ");
                Serial.println(send_tokens[address]);
        }
    }
}




#endif