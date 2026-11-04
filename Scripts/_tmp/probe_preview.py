import json
import unreal
rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
out = {}
for prop in ('preview_mode', 'PreviewMode', 'bPreviewMode', 'preview_behavior'):
    try:
        out[prop] = str(rig.get_editor_property(prop))
    except Exception:
        out[prop] = 'n/a'
out['props'] = [p for p in dir(rig) if 'preview' in p.lower()]
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
