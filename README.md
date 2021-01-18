# StreamFiler

## Feladatkiírás

### NAME
* streamfiler - TCP/IP stream fogadó és lemezre író alkalmazás

### SYNOPSIS
* streamfiler [-options] portnumber

### DESCRIPTION
* A streamfiler egy porton figyelő alkalmazás, ami a csatlakozott connection
  tartalmát lemezre írja. A kiírt fájl neve megegyezik az érkezési
  időponttal (milisecundum pontossággal). Egyszerre több connection-t tud
  fogadni paraméterezéstől függően.

### OPTIONS
* -c connections
    * A maximálisan fogadható párhuzamos connection-ök száma
* -f folder
    * A mappa, ahova az érkezetett állományokat letárolja a program. Ha nem
      létezik induláskor, akkor a program létrehozza azt.
* -h
    * Ez a help
* -l limit
    * Ekkora limit adatmennyiséget enged szálanként és másodpercenként
      kilobyte-ban a portjára folyatni a program
* -t timeout
    * Azt az időt határozza meg másodpercben, ami után a program bontja az
      idle connection-öket. Timeout -1 esetén nem bontja.
      
## Fordítás és futtatás

### Tudnivalók
* A programok 64 bites Windows 10 Enterprise operációs rendszeren lettek megírva és futtatva.
* A repository tartalmaz egy JAVA klienst, ami képes egy mappában lévő összes fájl elküldésére.
* Az ```src``` mappában található két ```.bat``` kiterjesztésű fájl (```runServer.bat``` és ```runClient.bat```), melyek a szerver és a kliens gyorsabb fordítását és futását   teszik lehetővé. Ebben az esetben, ezek szerkeztésével változtathatóak meg a szerver és a kliens parancssori paramétereik.

### C++ szerver fordítása és futtatása
* Fordítás MinGW-W64-el történt.
* Fordítás: g++ StreamFilerServer.cpp -std=c++17 -lwsock32 -o StreamFilerServer 
* Futtatás: StreamFilerServer -c 3 -f "..\io\output" -l 8192 -t 0 8080

### JAVA kliens fordítása és futtatása
* Fordítás JDK 13.0.2-vel történt.
* Fordítás: javac StreamFilerClient.java
* Futtatás: java StreamFilerClient localhost 8080 ..\io\input\
