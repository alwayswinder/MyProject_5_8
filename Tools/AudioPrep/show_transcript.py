import json
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8")
path = Path(__file__).resolve().parent / "aqi_transcript.json"
for i, x in enumerate(json.load(open(path, encoding="utf-8"))):
    dur = x["end"] - x["start"]
    print(f"{i:03d} {x['start']:7.2f}-{x['end']:7.2f} ({dur:5.2f}s) {x['text']}")
