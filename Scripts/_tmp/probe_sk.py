import json
import unreal

sk = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven_Skeleton')
out = {'methods': [m for m in dir(sk) if 'bone' in m.lower() or 'ref' in m.lower()]}
bt = sk.get_editor_property('bone_tree')
out['bt_type'] = str(type(bt))
try:
    out['bones'] = [str(x) for x in bt.get_editor_property('bone_info')][:5]
except Exception as exc:
    out['bt_err'] = repr(exc)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
