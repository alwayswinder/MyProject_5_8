import json
import os
import subprocess
import sys
import wave

import numpy as np
import soundfile as sf

SRC = r"D:\AI\work\bili-audio\【刺客伍六七】伍六七语音包 320k.mp3"
TMP = os.path.join(os.environ["TEMP"], "aqi_pack_16k.f32.wav")
OUTDIR = r"E:\Unreal\Projects\MyProject_5_8\TestAudio\candidates"
FFMPEG = sys.argv[1] if len(sys.argv) > 1 else "ffmpeg"

os.makedirs(OUTDIR, exist_ok=True)

subprocess.run(
    [FFMPEG, "-y", "-hide_banner", "-loglevel", "error", "-i", SRC,
     "-ac", "1", "-ar", "16000", "-f", "wav", "-acodec", "pcm_f32le", TMP],
    check=True,
)

data, sr = sf.read(TMP, dtype="float32")
print(f"decoded: {len(data)/sr:.2f}s @ {sr}Hz, peak={np.max(np.abs(data)):.3f}")

# --- segment detection -------------------------------------------------------
frame = int(0.02 * sr)          # 20 ms
hop = frame
n = len(data) // frame
rms = np.array([np.sqrt(np.mean(data[i*hop:(i+1)*hop] ** 2)) for i in range(n)])
floor = np.percentile(rms, 10)
peak = np.percentile(rms, 99)
thr = max(floor * 1.8, peak * 0.06, 1e-4)
voiced = rms > thr

segments = []
start = None
for i, v in enumerate(voiced):
    if v and start is None:
        start = i
    elif not v and start is not None:
        segments.append((start, i))
        start = None
if start is not None:
    segments.append((start, n))

# merge gaps shorter than 0.45 s
merged = []
for s, e in segments:
    if merged and (s - merged[-1][1]) * 0.02 < 0.45:
        merged[-1] = (merged[-1][0], e)
    else:
        merged.append((s, e))

# keep clips of reasonable length, add small padding
clips = []
for s, e in merged:
    t0 = max(0.0, s * 0.02 - 0.12)
    t1 = min(len(data) / sr, e * 0.02 + 0.20)
    dur = t1 - t0
    if 1.2 <= dur <= 12.0:
        clips.append((t0, t1, dur))

clips.sort(key=lambda c: -c[2])
print(f"segments: {len(merged)}, usable clips: {len(clips)}")
for i, (t0, t1, d) in enumerate(clips[:12]):
    print(f"  #{i}: {t0:7.2f} - {t1:7.2f}  ({d:5.2f}s)")

# --- export the best few -----------------------------------------------------
def write_wav(path, a, sr_):
    a = np.clip(a, -1.0, 1.0)
    pcm = (a * 32767.0).astype("<i2")
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr_)
        w.writeframes(pcm.tobytes())

exported = []
for i, (t0, t1, d) in enumerate(clips[:6]):
    a = data[int(t0 * sr):int(t1 * sr)].copy()
    # short fades so the clip does not click
    fade = int(0.01 * sr)
    if len(a) > 2 * fade:
        a[:fade] *= np.linspace(0, 1, fade)
        a[-fade:] *= np.linspace(1, 0, fade)
    peak_a = np.max(np.abs(a)) or 1.0
    a *= (10 ** (-3 / 20)) / peak_a           # normalize to -3 dBFS
    path = os.path.join(OUTDIR, f"aqi_{i:02d}_{t0:07.2f}-{t1:07.2f}.wav")
    write_wav(path, a, sr)
    exported.append({"index": i, "start": round(t0, 2), "end": round(t1, 2),
                     "duration": round(d, 2), "file": path})

print(json.dumps(exported, ensure_ascii=False, indent=2))
