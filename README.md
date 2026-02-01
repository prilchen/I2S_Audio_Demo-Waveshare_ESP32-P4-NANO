# I2S Audio Demo – Waveshare ESP32‑P4‑NANO  
**ES8311 Codec · I2S Wiedergabe & Echo · ESP‑IDF Beispielprojekt**

Dieses Projekt demonstriert die Audio‑Funktionen des **Waveshare ESP32‑P4‑NANO** in Kombination mit dem **ES8311‑Audio‑Codec**.  
Es zeigt, wie man Audio über I2S ausgibt, ein Mikrofon einliest und PCM‑Dateien abspielt.

Das Beispiel unterstützt zwei Betriebsmodi:

- 🎵 **Musikmodus** – spielt eine eingebettete PCM‑Datei ab  
- 🔁 **Echomodus** – gibt das Mikrofonsignal in Echtzeit wieder

---

## 📦 Features

- Initialisierung des ES8311‑Codecs über I2C  
- I2S‑Konfiguration für 16‑Bit Stereo  
- Wiedergabe einer eingebetteten PCM‑Datei  
- Mikrofon‑Echo in Echtzeit  
- Aktivierung eines externen Audio‑Verstärkers (PA‑Pin)  
- Kompatibel mit ESP‑IDF

---

## 🧩 Hardware

- **Waveshare ESP32‑P4‑NANO**
- **ES8311 Audio‑Codec**
- Kopfhörer 4 Ohm oder 8 Ohm Lautsprecher (angeschlossen an die weiße JST-Buchse auf dem Board) oder Lautsprecher  
- Optional: Mikrofon (für Echomodus)
- Verbindung: USB-C Kabel (angeschlossen am Port **UART**)

---

## ⚙️ Software‑Voraussetzungen

- Entwicklungsumgebung:** Visual Studio Code (VS Code)
- Erweiterung:** Espressif IDF Extension für VS Code
- ESP‑IDF (empfohlen: v5.x oder neuer)
- CMake / Ninja (Standard bei ESP‑IDF)
- Toolchain für ESP32‑P4

---

## 🚀 Projektstruktur

```
main/
 ├── i2s_es8311_example.c   # Hauptbeispiel
 ├── example_config.h       # Pin‑ und Parameterkonfiguration
 └── CMakeLists.txt
```

---

## 🔌 GPIO‑Initialisierung

Das Projekt aktiviert einen definierten GPIO‑Pin (z. B. GPIO48), um einen  
**externen Audio‑Verstärker (PA)** einzuschalten:

```c
gpio_set_level(GPIO_OUTPUT_PA, 1);
```

---

## 🎚️ ES8311‑Codec‑Initialisierung

Der Codec wird über I2C konfiguriert:

- Sample‑Rate  
- Master‑Clock (MCLK)  
- Auflösung: 16‑Bit  
- Lautstärke  
- Mikrofon‑Konfiguration  
- Optional: Mikrofonverstärkung

---

## 🎧 I2S‑Initialisierung

Das Projekt richtet I2S wie folgt ein:

- **Master‑Modus**
- **Philips‑Standard**
- **16‑Bit Stereo**
- Zuweisung der Pins:
  - MCLK  
  - BCLK  
  - WS  
  - DOUT  
  - DIN  

Sowohl TX (Ausgabe) als auch RX (Eingabe) werden aktiviert.

---

## 🎵 Musikmodus

Wenn `CONFIG_EXAMPLE_MODE_MUSIC` aktiviert ist:

- Eine PCM‑Datei wird als Binärressource eingebettet  
- Daten werden vorab in den TX‑Kanal geladen  
- Die Musik wird in einer Endlosschleife abgespielt

---

## 🔁 Echomodus

Wenn der Musikmodus deaktiviert ist:

- Mikrofon‑Daten werden über I2S eingelesen  
- Direkt wieder über I2S ausgegeben  
- Dadurch entsteht ein **Live‑Echo**

---

## ▶️ Anwendung starten

Projekt kompilieren:

```bash
idf.py build
```

Flashen:

```bash
idf.py flash
```

Serielle Ausgabe:

```bash
idf.py monitor
```

---

## ⚙️ Konfiguration

Die Einstellungen findest du in:

```
example_config.h
```

Dort kannst du u. a. anpassen:

- I2S‑Pins  
- I2C‑Pins  
- Sample‑Rate  
- MCLK‑Multiplikator  
- Lautstärke  
- Betriebsmodus (Musik / Echo)

---

## 🚀 Installation & Nutzung

1.  **Projekt öffnen:**
    Lade dieses Repository herunter und öffne den Ordner in Visual Studio Code.

2.  **Ziel wählen:**
    Setze das Target auf den P4-Chip:
    * Befehl: `ESP-IDF: Set Espressif Device Target` -> `esp32p4`

3.  **Kompilieren (Build):**
    Erstelle das Programm.
    * Klicke auf das **Zylinder-Symbol** (Build) in der unteren Leiste.

4.  **Flashen:**
    Übertrage das Programm auf das Board.
    * Verbinde das Board über den **UART**-Port.
    * Klicke auf das **Blitz-Symbol** (Flash).

5.  **Testen:**
    Nach dem Neustart sollte leise Musik aus dem angeschlossenen Lautsprecher zu hören sein.

## ⚖️ Lizenz

Dieses Projekt basiert auf Beispielcode von Espressif Systems. (https://www.waveshare.com/wiki/ESP32-P4-Nano-StartPage)

* **Lizenz:** CC0-1.0 (Creative Commons Zero v1.0 Universal)
* **Status:** Public Domain (Gemeinfrei)

Der Code kann frei verwendet, verändert und für eigene Projekte genutzt werden.

## 👤 Autor & Kontakt

**Prilchen LABS**
* Webseite: [prilchen.de](https://prilchen.de)
* YouTube: [Prilchen auf YouTube](https://www.youtube.com/@prilchen)
* GitHub: [prilchen](https://github.com/prilchen)

---
*Hinweis: Dies ist ein Lernprojekt für Ausbildungszwecke (FiSi / ITSE) und Bastler.*
