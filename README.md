## Per leggere da terminale:
```
./receiver /dev/ttyUSB0
```
Poi digitando:
- _o_ (online mode), si può vedere in live il feed del sensore ogni x secondi a scelta dell'utente
- _q_ (query mode), vengono mostrati i valori richiesti dalla traccia
- _c_ (clearing mode), sono puliti gli array dove vengono salvate le misurazioni

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

(1 giro sono circa 2 ohm)\
Dopo il ricevimento la resistenza del potenziometro era 70kOhm.

## Sensibilità Reale
Sembra che per carichi che consumano meno di 0.1 A il sensore non sia molto preciso.
Usando carichi resistivi (ventilatori e lampade) e una pinza amperometrica, ho preso alcune misure (taratura_sensore.csv).
Fornendo poi queste misure a ml.py, ho trovato le costanti di calibrazione:
0.5585782067493418 -0.023709353719794463 Ossia: 0.586 -0.0237
Alla fine dei conti, la sensibilità più o meno è di 60mA.