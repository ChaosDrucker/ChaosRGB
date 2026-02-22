# CHAOS RGB - Pinout & Verkabelung

## Hardware-Komponenten

| Nr. | Komponente | Beschreibung |
|-----|-----------|--------------|
| 1 | ESP32 DevKit V1 | Mikrocontroller (30-Pin Version) |
| 2 | WS2812B LED-Strip | 150 LEDs, adressierbarer RGB-Strip |
| 3 | WS2812B LED-Ring | 36 LEDs, adressierbarer RGB-Ring |
| 4 | 4x Arcade-Taster | Rot, Gruen, Blau, Gelb - mit eingebauter LED |
| 5 | Passiver Buzzer | Fuer Tonsignale und Spielsounds |
| 6 | SSD1306 OLED Display | 128x64 Pixel, I2C-Schnittstelle |
| 7 | 5V Netzteil | Min. 10A fuer LED-Strip + Ring |
| 8 | Kabel & Steckverbinder | Dupont-Kabel, Loetmaterial |

## Pinout-Tabelle

### LED-Strips

| GPIO | Richtung | Komponente | Hinweis |
|------|----------|------------|---------|
| 32 | OUTPUT (DATA) | LED-Strip (150 LEDs) | WS2812B, Hauptstrip fuer Spiele |
| 14 | OUTPUT (DATA) | LED-Ring (36 LEDs) | WS2812B, Ring fuer Spiele + Uhr |

### Taster (Buttons)

| GPIO | Richtung | Komponente | Hinweis |
|------|----------|------------|---------|
| 18 | INPUT_PULLUP | Taster Rot | Aktiv LOW (gegen GND schalten) |
| 17 | INPUT_PULLUP | Taster Gruen | Aktiv LOW (gegen GND schalten) |
| 25 | INPUT_PULLUP | Taster Blau | Aktiv LOW (gegen GND schalten) |
| 27 | INPUT_PULLUP | Taster Gelb | Aktiv LOW (gegen GND schalten) |

### Taster-LEDs

| GPIO | Richtung | Komponente | Hinweis |
|------|----------|------------|---------|
| 19 | OUTPUT | LED in Taster Rot | Beleuchtung im Taster |
| 16 | OUTPUT | LED in Taster Gruen | Beleuchtung im Taster |
| 33 | OUTPUT | LED in Taster Blau | Beleuchtung im Taster |
| 26 | OUTPUT | LED in Taster Gelb | Beleuchtung im Taster |

### Buzzer

| GPIO | Richtung | Komponente | Hinweis |
|------|----------|------------|---------|
| 23 | OUTPUT | Passiver Buzzer | Tonsignal ueber tone() |

### OLED Display (I2C)

| GPIO | Funktion | Komponente | Hinweis |
|------|----------|------------|---------|
| 21 | SDA | SSD1306 OLED | I2C Datenleitung |
| 22 | SCL | SSD1306 OLED | I2C Taktleitung |

I2C-Adresse: `0x3C`

## Verkabelungsschema

```
                         ESP32 DevKit V1
                        ┌───────────────┐
                        │               │
          Taster Gruen ─┤ GPIO 17    32 ├─ LED-Strip DATA (150 LEDs)
          Taster Rot   ─┤ GPIO 18    33 ├─ LED Taster Blau
          LED Taster Rot┤ GPIO 19    25 ├─ Taster Blau
          OLED SDA     ─┤ GPIO 21    26 ├─ LED Taster Gelb
          OLED SCL     ─┤ GPIO 22    27 ├─ Taster Gelb
          Buzzer       ─┤ GPIO 23    14 ├─ LED-Ring DATA (36 LEDs)
          LED Taster Gr.┤ GPIO 16       │
                        │           3V3 ├─ OLED VCC
                        │           GND ├─ Gemeinsame Masse
                        │           5V  ├─ (Nur fuer ESP32, NICHT fuer LEDs!)
                        └───────────────┘
```

## Verkabelungsdetails

### Taster (Buttons)
```
    ESP32 GPIO ──── Taster ──── GND
```
- Interne Pull-Up Widerstaende werden im Code aktiviert (`INPUT_PULLUP`)
- Kein externer Widerstand noetig
- Taster verbindet GPIO mit GND wenn gedrueckt (Aktiv LOW)

### Taster-LEDs
```
    ESP32 GPIO ──── [Vorwiderstand] ──── LED (+) ──── GND
```
- Vorwiderstand je nach LED-Typ (typisch 220-470 Ohm)
- Bei Arcade-Tastern mit eingebauter LED: Datenblatt beachten

### LED-Strip & Ring (WS2812B)
```
    5V Netzteil (+) ──── LED-Strip VCC (Rot)
    5V Netzteil (-) ──── LED-Strip GND (Weiss) ──── ESP32 GND
    ESP32 GPIO      ──── LED-Strip DATA (Gruen)
```

**WICHTIG - Stromversorgung:**
- LED-Strip (150 LEDs): Max. ~9A bei voller Helligkeit (weiss, 100%)
- LED-Ring (36 LEDs): Max. ~2.2A bei voller Helligkeit
- **Separate 5V Stromversorgung verwenden!** Nicht ueber den ESP32 versorgen!
- GND vom Netzteil mit ESP32 GND verbinden (gemeinsame Masse)
- Optional: 1000uF Kondensator am Netzteil-Eingang der LEDs
- Optional: 330-470 Ohm Widerstand in die DATA-Leitung

### OLED Display (I2C)
```
    ESP32 3.3V ──── OLED VCC
    ESP32 GND  ──── OLED GND
    ESP32 GPIO 21 ── OLED SDA
    ESP32 GPIO 22 ── OLED SCL
```
- Display laeuft mit 3.3V (NICHT 5V!)
- I2C-Adresse: 0x3C (Standard fuer SSD1306 128x64)

### Buzzer
```
    ESP32 GPIO 23 ──── Buzzer (+) ──── GND
```
- Passiver Buzzer (aktiver Buzzer funktioniert NICHT korrekt mit tone())

## Zusammenfassung GPIO-Belegung

```
GPIO 14 ── LED-Ring DATA          GPIO 25 ── Taster Blau
GPIO 16 ── LED Taster Gruen       GPIO 26 ── LED Taster Gelb
GPIO 17 ── Taster Gruen           GPIO 27 ── Taster Gelb
GPIO 18 ── Taster Rot             GPIO 32 ── LED-Strip DATA
GPIO 19 ── LED Taster Rot         GPIO 33 ── LED Taster Blau
GPIO 21 ── OLED SDA
GPIO 22 ── OLED SCL
GPIO 23 ── Buzzer
```

Insgesamt: **13 GPIOs** belegt
