import json
import unreal

AR = unreal.AssetRegistryHelpers.get_asset_registry()
assets = AR.get_assets_by_path('/Game', recursive=True)
rigs = [str(a.package_name) for a in assets if 'ControlRigBlueprint' in str(a.asset_class_path.asset_name)]
out = {}
for p in rigs:
    try:
        rig = unreal.load_asset(p)
        h = rig.get_hierarchy()
        keys = h.get_all_keys()
        ctrls = [str(k.name) for k in keys if k.type == unreal.RigElementType.CONTROL]
        out[p] = {'total': len(keys), 'controls': len(ctrls), 'sample': ctrls[:6]}
    except Exception as exc:
        out[p] = repr(exc)
print('JSONSTART'); print(json.dumps(out, ensure_ascii=False, indent=1)); print('JSONEND')
