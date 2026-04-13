# drum_mchn

Main firmware for CoreSynq32.

## Files

**`drum_mchn.ino`** — Main source. Contains FreeRTOS task definitions, audio engine, sequencer logic, and OLED UI.

**`samples.h`** — PCM sample arrays for kick, snare, and hi-hat. Generated from WAV files using `/tools/wav_to_c.py`. Not included in the repo — generate your own using the instructions in the root README.

## Dependencies

Install via Arduino IDE Library Manager:
- `Adafruit SSD1306`
- `Adafruit GFX Library`

FreeRTOS is built into the ESP32 Arduino core, no separate install needed.

## Board Setup

- Board: `ESP32 Dev Module`
- Core: ESP32 Arduino Core 3.x
- Flash size: 4MB
- Partition scheme: Default

## Notes

- `samples.h` must be present in this folder before compiling — the sketch won't build without it
- Timer is configured for 22050Hz sample rate — matches the WAV export settings in `/tools`
- DAC output is on GPIO 25 — connect to LM386 amp circuit (see `/schematics`)
