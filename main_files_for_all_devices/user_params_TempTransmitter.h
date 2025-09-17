#ifndef USER_P_H
#define USER_P_H

//ustawienia konkretnego urządzenia. ustawić według życzenia przed wgraniem kodu na esp32

#define DEBUG true

#define ADDRESS 3
#define NET_SIZE 5

// 0 - ruter
// 1 - pilot 
// 2 - kontroler zapasowy
// 3 - czujnik temperatury
// 4 - odbiornik pomiarów temperatury

//ODKOMENTOWAĆ POTRZEBNE

// Jeśli komunkacja nadajnik > przez router > odbiornik
int routing_table[NET_SIZE] = {
    0, // do rutera: pośrednio przez router
    0, // do pilota: pośrednio przez router
    0, // do zapasowego: pośrednio przez router
    3, // do siebie: nie wysyłaj (albo zignoruj)
    0  // do odbiornika: pośrednio przez router
};

/* 
// Jeśli komunkacja bezpośrednia nadajnik > odbiornik
int routing_table[NET_SIZE] = {
    0, // do rutera: pośrednio przez router
    0, // do pilota: pośrednio przez router
    0, // do zapasowego: pośrednio przez router
    3, // do siebie: nie wysyłaj (albo zignoruj)
    4  // do odbiornika: bezpośrednio
};
*/
#endif
