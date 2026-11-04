import json
import unreal
seq = unreal.load_asset('/Game/CR_Seven_MorphSliders_Take9.CR_Seven_MorphSliders_Take9')
out = {'class': seq.get_class().get_name() if seq else None}
if seq:
    out['methods'] = [m for m in dir(seq) if 'scene' in m.lower() or 'track' in m.lower() or 'bind' in m.lower()]
    for name in ('movie_scene', 'MovieScene'):
        try:
            out[name] = str(seq.get_editor_property(name))
        except Exception as exc:
            out[name] = 'ERR ' + repr(exc)[:120]
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
