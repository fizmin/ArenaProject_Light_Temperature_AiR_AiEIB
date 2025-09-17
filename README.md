# Opis projektu  

Repozytorium zawiera projekt studencki realizowany na Politechnice Śląskiej w roku 2025. Jego celem jest zaprojektowanie i implementacja systemu oświetlenia estradowego oraz modułu pomiaru temperatury. Rozwiązanie opiera się na mikrokontrolerach **ESP32** oraz komunikacji bezprzewodowej w standardzie **LoRa**.  

## Struktura repozytorium  

W repozytorium znajdują się:  
- kod źródłowy projektu,  
- folder `main_files_for_all_devices` zawierający pliki `main` i pliki `user_params` dla pięciu urządzeń wchodzących w skład systemu.  

Aby wgrać kod na wybrane urządzenie, należy:  
1. Pobrać repozytorium,  
2. Podmienić plik `main` i plik `user_params` na wersję odpowiadającą konkretnemu urządzeniu,  
3. Usunąć folder `main_files_for_all_devices`,  
4. Wgrać przygotowany projekt na mikrokontroler ESP32.  

## Urządzenia w projekcie  

System składa się z pięciu urządzeń, z których każde posiada przypisany identyfikator w sieci:  

- **0** – router [Router],  
- **1** – pilot [RemoteControler],  
- **2** – kontroler zapasowy [BackupController],  
- **3** – czujnik temperatury [TempTransmitter],  
- **4** – odbiornik pomiarów temperatury [TempReciever].  

Każde urządzenie zawiera własną tablicę `routing_table`, określającą docelowe węzły komunikacji w sieci LoRa.  

## Bezpieczeństwo komunikacji  

W projekcie zastosowano szyfrowanie komunikacji w oparciu o algorytm **AES**. Klucz szyfrujący zapisany jest w pliku `secrets.h`.  
⚠️ Należy pamiętać o jego zmianie przed wdrożeniem systemu – domyślny klucz umieszczony w repozytorium nie powinien być wykorzystywany w projekcie  
