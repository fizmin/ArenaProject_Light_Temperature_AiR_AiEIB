# Opis projektu  

Repozytorium zawiera projekt studencki realizowany na Politechnice Śląskiej w roku 2025. Jego celem jest zaprojektowanie i implementacja systemu oświetlenia estradowego oraz modułu pomiaru temperatury. Rozwiązanie opiera się na mikrokontrolerach **ESP32** oraz komunikacji bezprzewodowej w standardzie **LoRa**.  

## Struktura repozytorium  

W repozytorium znajdują się:  
- kod źródłowy projektu,  
- folder `main_files_for_all_devices` zawierający pliki `main` dla pięciu urządzeń wchodzących w skład systemu.  

Aby wgrać kod na wybrane urządzenie, należy:  
1. Pobrać repozytorium,  
2. Podmienić plik `main` na wersję odpowiadającą konkretnemu urządzeniu,  
3. Usunąć folder `main_files_for_all_devices`,  
4. Wgrać przygotowany plik `main` na mikrokontroler ESP32.  

## Urządzenia w projekcie  

System składa się z pięciu urządzeń, z których każde posiada przypisany identyfikator w sieci:  

- **0** – router,  
- **1** – pilot,  
- **2** – kontroler zapasowy,  
- **3** – czujnik temperatury,  
- **4** – odbiornik pomiarów temperatury.  

Każde urządzenie zawiera własną tablicę `routing_table`, określającą docelowe węzły komunikacji w sieci LoRa.  

## Bezpieczeństwo komunikacji  

W projekcie zastosowano szyfrowanie komunikacji w oparciu o algorytm **AES**. Klucz szyfrujący zapisany jest w pliku `secrets.h`.  
⚠️ Należy pamiętać o jego zmianie przed wdrożeniem systemu – domyślny klucz umieszczony w repozytorium nie powinien być wykorzystywany w środowisku produkcyjnym.  
