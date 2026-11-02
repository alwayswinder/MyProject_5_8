import json
import sys

sys.stdout.reconfigure(encoding="utf-8")
path = r"E:\Unreal\Projects\MyProject_5_8\Tools\aqi_transcript.json"
for i, x in enumerate(json.load(open(path, encoding="utf-8"))):
    dur = x["end"] - x["start"]
    print(f"{i:03d} {x['start']:7.2f}-{x['end']:7.2f} ({dur:5.2f}s) {x['text']}")
