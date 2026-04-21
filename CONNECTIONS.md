# Lista polaczen sprzetowych

Poniższa lista odpowiada finalnej wersji kodu z tego repozytorium.

## LCD 2x16

- RS -> PB0
- E -> PB1
- D4 -> PB2
- D5 -> PB3
- D6 -> PB4
- D7 -> PB5
- RW -> GND
- VSS -> GND
- VDD -> +5V
- VO -> potencjometr kontrastu

## Joystick analogowy

- VRx -> PA2 / ADC2
- VRy -> PA3 / ADC3
- SW -> PD5
- VCC -> +5V
- GND -> GND

## Bluetooth BTM222B

- TX modulu -> PD0 / RXD
- RX modulu -> PD1 / TXD
- VCC -> +5V
- GND -> GND

## Serwomechanizm

- sygnal sterujacy -> PD4 / OC1B
- zasilanie -> zgodnie z wymaganiami serwa
- masa serwa -> wspolna masa z ukladem

## Uwagi praktyczne

- LCD w tej wersji jest podlaczony do portu B.
- Odczyt joysticka korzysta z wejsc ADC2 i ADC3.
- Przycisk joysticka jest obslugiwany jako wejscie z podciaganiem na PD5.
- Serwo korzysta z wyjscia PWM Timer1 na OC1B.
- Wszystkie moduly musza miec wspolna mase z mikrokontrolerem.