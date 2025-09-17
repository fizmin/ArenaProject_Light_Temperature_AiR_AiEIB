#ifndef USER_P_H
#define USER_P_H

//ustawienia konkretnego urządzenia. ustawić według życzenia przed wgraniem kodu na esp32

#define DEBUG true

#define ADDRESS 4
#define NET_SIZE 5

// 0 - router
// 1 - pilot
// 2 - kontroler zapasowy
// 3 - czujnik temperatury
// 4 - odbiornik pomiarów temperatury

int routing_table[NET_SIZE] = {
    0, // do rutera: bezpośrednio
    1, // do pilota: bezpośrednio
    2, // do zapasowego: bezpośrednio
    3, // do czujnika: bezpośrednio
    4  // do siebie: nie wysyłaj (albo zignoruj)
};

#endif
