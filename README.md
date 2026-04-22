# Interaktywny sterownik mikrokontrolerowy z LCD, joystickiem i Bluetooth

Projekt z przedmiotu Interfejsy cyfrowe realizowany na mikrokontrolerze ATmega32A. Uklad laczy lokalna obsluge przez joystick, wyswietlacz LCD 2x16, komunikacje Bluetooth z telefonem oraz sterowanie serwomechanizmem.

## Repozytorium

Nazwa repozytorium: `atmega32a-bt-joystick-lcd`

Opis repozytorium: Projekt z przedmiotu Interfejsy cyfrowe realizowany na mikrokontrolerze ATmega32A, obejmujacy obsluge wyswietlacza LCD 2x16, joysticka analogowego, komunikacji Bluetooth oraz sterowania serwomechanizmem.

## Najwazniejsze funkcje

- menu sterowane joystickiem,
- ekran autora i informacji o projekcie,
- odbior tekstu z telefonu i wyswietlanie go na LCD,
- osobny tryb komend Bluetooth,
- podglad wartosci joysticka X/Y,
- sterowanie serwem joystickiem,
- zdalne ustawianie pozycji serwa przez komendy Bluetooth,
- raportowanie stanu joysticka przez UART/Bluetooth.

## Tryby pracy

1. O mnie
2. Aplikacja
3. BT TEXT
4. BT CTRL
5. JOY MONITOR
6. SERVO TEST

## Komendy Bluetooth

- `1` - wyslij pomoc
- `2tekst` - pokaz tekst na LCD
- `3` - wyslij jednorazowy raport joysticka
- `4` - wlacz stream raportow joysticka
- `5` - wylacz stream raportow joysticka
- `6` - pokaz dane autora
- `7` - wyczysc LCD
- `81` .. `89` - ustaw poziom serwa 1..9
- `90` - ustaw serwo w pozycji centralnej

## Struktura projektu

- `main.c` - logika aplikacji, menu, UART, ADC, Bluetooth i serwo
- `lcd.c` - sterownik LCD 2x16 w trybie 4-bitowym
- `lcd.h` - interfejs modulu LCD
- `CONNECTIONS.md` - lista polaczen sprzetowych
- `PROJECT_TRANSLATION.md` - krotkie tlumaczenie opisu projektu na angielski
- `report.tex` - robocza wersja sprawozdania LaTeX

## Uzyty sprzet

- ATmega32A
- zestaw uruchomieniowy ATB 1.05
- LCD 2x16 zgodny z HD44780
- joystick analogowy
- modul Bluetooth BTM222B
- serwomechanizm sterowany z Timer1



