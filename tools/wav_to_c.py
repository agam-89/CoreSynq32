import os

files = ["kick-dry.wav", "snare-electro.wav", "hihat-digital.wav"]
names = ["kick", "snare", "hihat"]

for file, name in zip(files, names):
    with open(file, "rb") as f:
        data = f.read()[44:]
    print(f"const uint8_t {name}[] = {{" + ",".join(str(b) for b in data) + "};")
    print(f"const int {name}_len = {len(data)};")
    print()