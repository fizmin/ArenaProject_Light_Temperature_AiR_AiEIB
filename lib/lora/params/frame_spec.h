#ifndef FRAME_SPEC_H
#define FRAME_SPEC_H

#include <Arduino.h>

#define MAX_FRAME_SIZE 100

#define FRAME_VERSION         0
#define VERSION_SIZE_BITS     3     //lepiej nie ruszać

//Adresy początków segmentów ramki (numer bajtu)

//segment adresu
#define FRAME_POS_NEXT_ADDR   00
#define FRAME_POS_DEST_ADDR   01
#define FRAME_POS_SEND_ADDR   02

//segment metadanych
#define FRAME_POS_TYPE        03
#define FRAME_POS_ID          04
#define FRAME_POS_TTL         05
#define FRAME_POS_CRC         06    //crc nie jest na końcu, żeby wszystkie adresy były stałe. jedyne co się rusza to wielkość wiadomości

//segment danych
#define FRAME_POS_MSG         07    //encrypted; zawiera w sobie token




//Adres wewnątrz bajtu typu (maski bitowe)

#define TYPE_MASK_VER       B11100000
#define TYPE_MASK_ACK       B00010000
#define TYPE_MASK_SYNC      B00001000
#define TYPE_MASK_NEED_ACK  B00000100
#define TYPE_MASK_RESEND    B00000010
#define TYPE_MASK_ERROR     B00000001


#define TOKEN_SIZE            2     //w bajtach; min 1, max 4
#define TOKEN_TRESHOLD        20
#define TOKEN_MAX            ((uint64_t) pow(2, 8 * TOKEN_SIZE) - 1)


#define INIT_TTL            4   //TTL (Time To Leave) nowej wiadomości - max ilość przekierowań podczas routowania


struct LoraFrame {
    uint8_t buf[MAX_FRAME_SIZE] = {0};
    uint8_t size = 0;
};

#endif