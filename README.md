## Per leggere da terminale:
```
./receiver /dev/ttyUSB0
```

## Collegamento sensore-arduino:
1. GND->GND (marrone)
2. GND->GND (marrone)
3. OUT->A0 (bianco)
4. VCC->5V (rosso)

## Sensitività segnale sensore
La sensitività del segnale è 0.05. L'ho calcolato dal datasheet, infatti abbiamo:
- Input Current = 0-10A (50 ohm)
- Rated Output Current = 5mA at an input of 5A (a una corrente di input uguale a 1A corrisponde un output di 1mA)

Significa che usando la legge di Ohm:

``V=RI => 50 ohm * 1mA => 0.05V per A``

## Potenziometro
Con la vite in basso a destra, girando:
- verso destra aumenta la resistenza
- verso sinistra diminuisce
(1 giro sono circa 2 ohm)

## Breadboard
Collegare i due GND in una strip e collegare quella strip a GND dell'arduino.
