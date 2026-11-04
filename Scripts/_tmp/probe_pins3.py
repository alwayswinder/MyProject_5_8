import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
model = rig.get_default_model()
out = {}
for n in model.get_nodes():
    name = n.get_name()
    if name not in ('BRW_GET', 'BRW_SET_meibu_L', 'BRW_DVAL'):
        continue
    rows = []
    for p in n.get_pins():
        try:
            pname = p.get_editor_property('name')
        except Exception:
            pname = str(p)
        try:
            dv = p.get_default_value()
        except Exception as exc:
            dv = 'ERR'
        try:
            dirs = str(p.get_editor_property('direction'))
        except Exception:
            dirs = ''
        rows.append([str(pname), str(dirs), str(dv)])
    out[name] = rows
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
