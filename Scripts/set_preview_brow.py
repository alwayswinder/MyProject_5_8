import json
import pathlib

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
STATE = pathlib.Path(unreal.Paths.project_saved_dir()) / 'Codex' / 'brow_state.json'

rig = unreal.load_asset(ASSET)
inst = rig.get_debugged_control_rig()
if not inst:
    raise RuntimeError('no preview instance opened in the CR editor')

state = json.loads(STATE.read_text()) if STATE.exists() else {}
print("target state:", state)

comp = inst
applied = {}
for name, value in state.items():
    try:
        comp.set_control_float(name, float(value))
        applied[name] = float(value)
    except Exception as exc:
        print("set_control_float failed for", name, repr(exc))

print("has set_control_float:", hasattr(comp, 'set_control_float'))

h = comp.get_control_rig() if hasattr(comp, 'get_control_rig') else None
if h is None:
    try:
        h = comp.get_editor_property('control_rig')
    except Exception:
        h = None
print("inner rig:", h)

rh = comp.get_hierarchy() if hasattr(comp, 'get_hierarchy') else None
if rh:
    def show(n):
        k = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=n)
        if not rh.contains(k):
            return (n, 'missing')
        t = rh.get_global_transform(k)
        return (n, round(t.translation.x, 2), round(t.translation.y, 2), round(t.translation.z, 2),
                round(t.scale3d.x, 3), round(t.scale3d.y, 3), round(t.scale3d.z, 3))
    print(show('meibu_L'), show('meibu_R'))
    for c in ('browInnerUp', 'browDownLeft', 'browDownRight'):
        k = unreal.RigElementKey(type=unreal.RigElementType.CURVE, name=c)
        if rh.contains(k):
            print(c, rh.get_curve_value(k))
