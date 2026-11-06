import unreal, json
RIG = "/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders"
rig = unreal.load_asset(RIG)
ctrl = rig.get_controller(rig.get_default_model())
out = {}
for b in ("Bone027","Bone028","Bone029","Bone036","Bone037","Bone038"):
    rec = {}
    for pin in ("SQUAT_%s.Axis" % b, "SANG_%s.B" % b, "OQUAT_%s.Axis" % b, "OANG_%s.B" % b):
        try: rec[pin] = ctrl.get_pin_default_value("LID_" + pin)
        except Exception as e: rec[pin] = "ERR"
    out[b] = rec
print("JSONSTART")
print(json.dumps(out, ensure_ascii=False, indent=1))
print("JSONEND")
