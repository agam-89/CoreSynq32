# CoreSynq32

> Real-time 3-channel drum sequencer on ESP32 — FreeRTOS task architecture, 22050Hz PCM sample playback via onboard DAC, and live OLED grid display.

---

## Demo



[![CoreSynq32 Demo](https://img.youtube.com/vi/T27PNiOOfRY/0.jpg)](https://www.youtube.com/watch?v=T27PNiOOfRY)



> Replace the link above with your actual demo video after uploading it to the repo.

---

## Overview

CoreSynq32 is an 8-step programmable drum sequencer built on the ESP32. It plays back real WAV drum samples (kick, snare, hi-hat) through the ESP32's onboard DAC into an LM386 amplifier and speaker. Patterns are programmed live via tactile buttons and visualized on an SSD1306 OLED display.

The project is built around a concurrent FreeRTOS task architecture — sequencer timing, audio playback, and UI all run as independent tasks communicating via shared state and critical sections, rather than a single blocking loop.

---

## Features

- 3-channel step sequencer — Kick, Snare, Hi-Hat
- 8-step grid with page toggle (Steps 1–4 / Steps 5–8)
- Real WAV sample playback at 22050Hz via ESP32 DAC
- Hardware timer ISR drives sample-accurate PCM output
- Live OLED display — scrolling playhead, active steps, BPM, track info
- BPM control via potentiometer (60–240 BPM)
- Play/Stop, Track Select, Page Toggle, and Clear controls

---

## Hardware

| Component | Details |
|-----------|---------|
| ESP32 DevKit V1 | Main microcontroller |
| SSD1306 OLED | 128×64, I2C |
| Tactile push buttons | ×4 step + ×4 control |
| Potentiometer | BPM control, GPIO 34 |
| LM386 amplifier | Audio output stage |
| Speaker | Salvaged desktop speaker |

Amplifier circuit schematic (LM386 + coupling caps) is included in `/schematics`.

---

## Pin Map

| GPIO | Function |
|------|----------|
| 13, 12, 14, 27, 26, 33, 32, 15 | Step buttons 1–8 |
| 4 | Track select |
| 5 | Page toggle |
| 6 | Play / Stop |
| 7 | Clear current track |
| 34 | BPM potentiometer |
| 21 | OLED SDA |
| 22 | OLED SCL |
| 25 | DAC audio output |

---

## Controls

| Button | Action |
|--------|--------|
| STEP 1–4 | Toggle steps on current page |
| Track Select | Cycle between KICK → SNARE → HAT |
| Page Toggle | Switch between steps 1–4 (PG:A) and 5–8 (PG:B) |
| Play/Stop | Start or pause sequencer |
| Clear | Erase all steps on current track |
| Potentiometer | Adjust BPM live (60–240) |

---

## Architecture

```
┌─────────────┐   shared state    ┌──────────────────┐
│  vUI Task   │ ────────────────▶ │  vSequencer Task │
│  (Core 0)   │                   │  (Core 1)        │
│             │                   │                  │
│ Button scan │                   │ Advances playhead│
│ BPM read    │                   │ Calls triggerSound│
│ OLED draw   │                   └────────┬─────────┘
└─────────────┘                            │
                                           ▼
                                 ┌──────────────────┐
                                 │  Timer ISR       │
                                 │  @ 22050 Hz      │
                                 │                  │
                                 │ Feeds DAC one    │
                                 │ sample at a time │
                                 │ Mixes 3 channels │
                                 └──────────────────┘
```

- `vSequencer` runs on Core 1 at highest priority — handles beat timing
- `vUI` runs on Core 0 — handles display refresh and button scanning
- Hardware timer ISR fires at 22050Hz independent of both tasks
- 3-channel mixer averages active samples to prevent DAC clipping
- Shared state protected with `portENTER_CRITICAL` / `portEXIT_CRITICAL`

---

## Audio Pipeline

```
WAV files → Audacity (8-bit unsigned, mono, 22050Hz) → Python → C byte arrays → samples.h
ESP32 Flash → Timer ISR → dacWrite(GPIO 25) → LM386 → 1000µF cap → Speaker
```

---

## Software

- **IDE:** Arduino IDE 2.x
- **Core:** ESP32 Arduino Core 3.x
- **Libraries:** Adafruit SSD1306, Adafruit GFX, FreeRTOS (built-in)

---

## Custom Samples

`samples.h` is generated from your own WAV files using the script in `/tools/wav_to_c.py`. To swap in your own drum sounds:

1. Export your WAV files from Audacity as **8-bit unsigned PCM, mono, 22050Hz**
2. Place them in the same folder as `wav_to_c.py`
3. Update the filenames in the script and run it:
```
python wav_to_c.py
```
4. Paste the output into `samples.h`, replacing the existing arrays

Any short one-shot drum sample works — keep them under 1 second to avoid flash memory issues.

---

## Repository Structure

```
CoreSynq32/
├── drum_mchn/
│   ├── drum_mchn.ino       # Main source
│   └── samples.h           # WAV sample arrays (kick, snare, hihat)
├── schematics/
│   ├── wokwi_diagram.png   # Full system schematic (ESP32 + OLED + buttons)
│   └── 386amp.kicad_sch    # LM386 amplifier circuit
├── tools/
│   └── wav_to_c.py         # Converts WAV files to C byte arrays for samples.h
├── demo/
│   └── drum_machine_demo.mp4
└── README.md
```

---

## Resume Keywords

FreeRTOS · Real-time task scheduling · Inter-task synchronization · Hardware timer ISR · PCM audio · DAC · I2C · ESP32 · Embedded C · KiCad
