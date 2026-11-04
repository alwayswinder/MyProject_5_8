import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
ctrl = rig.get_controller(rig.get_default_model())
model = rig.get_default_model()

info = []
for n in model.get_nodes():
    nm = n.get_name()
    if not nm.startswith('BROW_'):
        continue
    pins = {}
    for p in n.get_pins():
        try:
            pname = p.get_name()
        except AttributeError:
            pname = str(p)
        try:
            dv = p.get_default_value()
        except Exception:
            dv = None
        try:
            src = [str(s) for s in p.get_sources()]
        except Exception:
            src = None
        pins[pname] = {'default': dv, 'sources': src}
    info.append({'node': nm, 'pins': pins})

print(json.dumps(info, ensure_ascii=False, indent=1))
