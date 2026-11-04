import json
import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
model = rig.get_default_model()
ctrl = rig.get_controller(model)
out = {}
for n in model.get_nodes():
    name = n.get_name()
    if not name.startswith('BRW_'):
        continue
    info = {'class': n.get_class().get_name()}
    try:
        pins = n.get_pins()
        info['pins'] = []
        for p in pins:
            d = {'name': str(p.get_pin_name()), 'dir': str(p.get_direction())}
            try:
                d['default'] = str(p.get_default_value())
            except Exception:
                pass
            info['pins'].append(d)
    except Exception as exc:
        info['pin_err'] = repr(exc)
    out[name] = info
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
