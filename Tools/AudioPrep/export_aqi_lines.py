import os
import subprocess
import sys
import wave
from pathlib import Path

import numpy as np
import soundfile as sf

SRC = r"D:\AI\work\bili-audio\【刺客伍六七】伍六七语音包 320k.mp3"
TMP = os.path.join(os.environ["TEMP"], "aqi_pack_16k.f32.wav")
OUT = Path(__file__).resolve().parents[2] / "TestAudio"
FFMPEG = sys.argv[1]

CLIPS = {
    "SevenSpeech": (106.55, 113.20, "我是刺客伍六七，刺客排行榜第17369位"),
    "aqi_alt_memory": (226.10, 232.55, "为什么我没有过去的记忆，我以前到底是一个怎么样的人"),
    "aqi_alt_assassin": (184.00, 187.40, "就算是刺客，也会有想保护的人"),
    "aqi_alt_shadow": (48.30, 51.55, "我是玄武国的暗影刺客"),
}

if not os.path.exists(TMP):
    subprocess.run([FFMPEG, "-y", "-hide_banner", "-loglevel", "error", "-i", SRC,
                    "-ac", "1", "-ar", "16000", "-f", "wav", "-acodec", "pcm_f32le", TMP],
                   check=True)

data, sr = sf.read(TMP, dtype="float32")
os.makedirs(OUT, exist_ok=True)


def trim_to_speech(a, sr_):
    """Cut leading/trailing noise so the model is not fed hiss before/after the line."""
    frame = int(0.02 * sr_)
    count = len(a) // frame
    if count < 4:
        return a
    rms = np.array([float(np.sqrt(np.mean(a[i * frame:(i + 1) * frame] ** 2))) for i in range(count)])
    loud = rms > max(0.02, 0.15 * float(np.percentile(rms, 95)))
    idx = np.where(loud)[0]
    if len(idx) == 0:
        return a
    start = max(0, int((idx[0] * 0.02 - 0.12) * sr_))
    end = min(len(a), int(((idx[-1] + 1) * 0.02 + 0.18) * sr_))
    return a[start:end]


def write_wav(path, a, sr_):
    a = np.clip(a, -1.0, 1.0)
    pcm = (a * 32767.0).astype("<i2")
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(sr_)
        w.writeframes(pcm.tobytes())


for name, (t0, t1, desc) in CLIPS.items():
    a = data[int(t0 * sr):int(t1 * sr)].copy()
    a = trim_to_speech(a, sr)

    fade = int(0.012 * sr)
    if len(a) > 2 * fade:
        a[:fade] *= np.linspace(0, 1, fade)
        a[-fade:] *= np.linspace(1, 0, fade)

    peak = float(np.max(np.abs(a))) or 1.0
    a *= (10 ** (-3 / 20)) / peak
    path = os.path.join(OUT, f"{name}.wav")
    write_wav(path, a, sr)
    print(f"{path}  {len(a)/sr:.2f}s  {desc}")
