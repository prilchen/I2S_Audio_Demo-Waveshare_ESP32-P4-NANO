# I2S Audio Demo für Waveshare ESP32-P4-NANO

Dieses Projekt ist ein Beispielcode, um die Audio-Funktionen des **Waveshare ESP32-P4-NANO** Boards zu testen. Es demonstriert die Ausgabe von Musik über die I2S-Schnittstelle (Inter-IC Sound) an einen angeschlossenen Lautsprecher.

## 📝 Beschreibung

Der Code initialisiert den I2S-Bus des ESP32-P4 Chips und gibt Audiodaten (z. B. ein Musikstück, das als Byte-Array im Code hinterlegt ist) über den Onboard-Verstärker aus. Es dient als "Hello World" für Audio-Anwendungen.

## 🛠 Hardware Voraussetzungen

* **Entwicklungsboard:** Waveshare ESP32-P4-NANO
* **Lautsprecher:** 4 Ohm oder 8 Ohm Lautsprecher (angeschlossen an die weiße JST-Buchse auf dem Board)
* **Verbindung:** USB-C Kabel (angeschlossen am Port **UART**)

## 💻 Software Umgebung

Das Projekt wurde mit folgenden Werkzeugen erstellt und getestet:

* **Entwicklungsumgebung:** Visual Studio Code (VS Code)
* **Erweiterung:** Espressif IDF Extension für VS Code
* **Framework Version:** ESP-IDF v5.3 (oder neuer)
* **Ziel-Chip (Target):** esp32p4

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
