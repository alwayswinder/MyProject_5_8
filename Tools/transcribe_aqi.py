import json
import os
import sys

from faster_whisper import WhisperModel
import soundfile as sf

TMP = os.path.join(os.environ["TEMP"], "aqi_pack_16k.f32.wav")
model_name = sys.argv[1] if len(sys.argv) > 1 else "small"

# Feed a float32 numpy array so faster-whisper does not need PyAV to decode.
audio, sr = sf.read(TMP, dtype="float32")

model = WhisperModel(model_name, device="cpu", compute_type="int8", download_root=os.path.join(os.environ["TEMP"], "fw_models"))
segments, info = model.transcribe(
    audio,
    language="zh",
    beam_size=5,
    vad_filter=True,
    vad_parameters=dict(min_silence_duration_ms=350),
    condition_on_previous_text=False,
)

lines = []
for s in segments:
    lines.append({"start": round(s.start, 2), "end": round(s.end, 2), "text": s.text.strip()})

print(json.dumps(lines, ensure_ascii=False, indent=1))
out = os.path.join(r"E:\Unreal\Projects\MyProject_5_8\Tools", "aqi_transcript.json")
with open(out, "w", encoding="utf-8") as f:
    json.dump(lines, f, ensure_ascii=False, indent=1)
print("written", out, len(lines))
