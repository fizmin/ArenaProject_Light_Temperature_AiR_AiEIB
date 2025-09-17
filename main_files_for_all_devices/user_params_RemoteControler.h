#ifndef USER_P_H
#define USER_P_H

//ustawienia konkretnego urządzenia. ustawić według życzenia przed wgraniem kodu na esp32

#define DEBUG true

#define ADDRESS 1
#define NET_SIZE 5

// 0 - ruter
// 1 - pilot 
// 2 - kontroler zapasowy
// 3 - czujnik temperatury
// 4 - odbiornik pomiarów temperatury

int routing_table[NET_SIZE] = {
    0, // do routera: bezpośrednio
    1, // do siebie: nie wysyłaj (albo zignoruj)
    0, // do zapasowego: przez ruter
    0, // do czujnika: przez router
    0  // do odbiornika: przez router
};



#endif
