# tools

Utility scripts for CoreSynq32.

## wav_to_c.py

Converts WAV drum samples into C byte arrays for use in `samples.h`.

### Usage

```
python wav_to_c.py
```

### Requirements

- Python 3.x
- WAV files exported from Audacity as **8-bit unsigned PCM, mono, 22050Hz**

### Output

Prints C declarations for all 3 samples directly to terminal:

```c
const uint8_t kick[] = {128, 145, ...};
const int kick_len = 5978;

const uint8_t snare[] = {128, 130, ...};
const int snare_len = 3658;

const uint8_t hihat[] = {128, 132, ...};
const int hihat_len = 3978;
```

Copy the output and paste it into `drum_mchn/samples.h` between the `#ifndef` and `#endif` guards.

### Changing Sample Files

Edit the filenames at the top of the script to match your WAV files:

```python
files = ["your-kick.wav", "your-snare.wav", "your-hihat.wav"]
names = ["kick", "snare", "hihat"]
```
