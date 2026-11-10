import sys
import wave
from pathlib import Path

import numpy as np

path = Path(__file__).resolve().parents[2] / "TestAudio" / "SevenSpeech.wav"
with wave.open(path, "rb") as w:
    n = w.getnframes()
    sr = w.getframerate()
    data = np.frombuffer(w.readframes(n), dtype="<i2").astype(np.float32) / 32768.0

frame = int(0.05 * sr)
count = len(data) // frame
rms = np.array([float(np.sqrt(np.mean(data[i * frame:(i + 1) * frame] ** 2))) for i in range(count)])

print(f"duration {len(data)/sr:.2f}s, frames {count}")
print("first 30 frames (0.05s each):")
for i in range(min(30, count)):
    print(f"  t={i*0.05:5.2f}  rms={rms[i]:.4f}  {'#' * int(min(60, rms[i] * 400))}")
print("last 30 frames:")
for i in range(max(0, count - 30), count):
    print(f"  t={i*0.05:5.2f}  rms={rms[i]:.4f}  {'#' * int(min(60, rms[i] * 400))}")

# speech level reference
print(f"\nmedian rms of loud frames: {np.median(rms[rms > np.percentile(rms, 60)]):.4f}")
print(f"max rms: {rms.max():.4f}, p10: {np.percentile(rms, 10):.4f}")

thr = 0.02 * rms.max()
above = np.where(rms > thr)[0]
if len(above):
    print(f"first frame above {thr:.4f}: t={above[0]*0.05:.2f}s, last: t={above[-1]*0.05:.2f}s")
