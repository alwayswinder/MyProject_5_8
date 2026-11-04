import json
import unreal

ASSET = "/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders"
rig = unreal.load_asset(ASSET)
inst = rig.create_control_rig()
inst.request_init()
h = inst.get_hierarchy()
k = lambda n, t=unreal.RigElementType.CONTROL: unreal.RigElementKey(type=t, name=n)
f = unreal.RigHierarchy.make_control_value_from_float

def run(state):
    for n, v in state.items():
        h.set_control_value(k(n), f(v))
    inst.execute("Forwards Solve")
    return {
        "curves": {c: round(h.get_curve_value(k(c, unreal.RigElementType.CURVE)), 4) for c in ("browInnerUp", "browDownLeft", "browDownRight")},
        "scaleL": round(h.get_global_transform(k("meibu_L", unreal.RigElementType.BONE)).scale3d.x, 3),
        "scaleR": round(h.get_global_transform(k("meibu_R", unreal.RigElementType.BONE)).scale3d.x, 3),
        "rotL": str(h.get_local_transform(k("meibu_L", unreal.RigElementType.BONE)).rotation).split("{")[-1][:60],
    }

print("neutral:", json.dumps(run({"CTRL_BROW_BEND": 0, "CTRL_BROW_ROT": 0.5, "CTRL_BROW_HIDE_L": 0, "CTRL_BROW_HIDE_R": 0}), ensure_ascii=False))
print("bend1  :", json.dumps(run({"CTRL_BROW_BEND": 1, "CTRL_BROW_ROT": 0.5, "CTRL_BROW_HIDE_L": 0, "CTRL_BROW_HIDE_R": 0}), ensure_ascii=False))
print("rot1   :", json.dumps(run({"CTRL_BROW_BEND": 0, "CTRL_BROW_ROT": 1, "CTRL_BROW_HIDE_L": 0, "CTRL_BROW_HIDE_R": 0}), ensure_ascii=False))
print("hideL1 :", json.dumps(run({"CTRL_BROW_BEND": 0, "CTRL_BROW_ROT": 0.5, "CTRL_BROW_HIDE_L": 1, "CTRL_BROW_HIDE_R": 0}), ensure_ascii=False))
print("hideR1 :", json.dumps(run({"CTRL_BROW_BEND": 0, "CTRL_BROW_ROT": 0.5, "CTRL_BROW_HIDE_L": 0, "CTRL_BROW_HIDE_R": 1}), ensure_ascii=False))
