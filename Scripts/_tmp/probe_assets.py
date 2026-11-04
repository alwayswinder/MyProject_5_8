import json
import unreal

AR = unreal.AssetRegistryHelpers.get_asset_registry()
assets = AR.get_assets_by_path('/Game/_MyTest/Mouth', recursive=True)
out = []
for a in assets:
    cls = str(a.asset_class_path.asset_name)
    if 'ControlRig' in cls:
        path = str(a.package_name)
        out.append(path)

detail = {}
for p in out:
    rig = unreal.load_asset(p)
    if not rig:
        detail[p] = 'LOADFAIL'
        continue
    try:
        h = rig.get_hierarchy()
        keys = h.get_all_keys()
        names = [str(k.name) for k in keys]
        sxm = [n for n in names if 'SXM' in n.upper() or 'K_' in n]
        detail[p] = {'count': len(keys), 'status': str(rig.get_editor_property('status')),
                     'sxm_like': sxm[:40], 'first': names[:25]}
    except Exception as exc:
        detail[p] = repr(exc)

# bones in skeletal mesh
mesh = unreal.load_asset('/Game/_MyTest/Mouth/Mesh/Seven')
bones = []
if mesh:
    try:
        bones = [str(b) for b in mesh.get_editor_property('skeleton').get_editor_property('bone_tree')]
        bones = [b.split(' ')[0] for b in bones]
    except Exception as exc:
        bones = ['ERR ' + repr(exc)]
detail['MESH_BONES_COUNT'] = len(bones)
detail['MESH_BONES_sxm'] = [b for b in bones if 'SXM' in b.upper() or b.startswith('K_')][:60]

print('JSONSTART')
print(json.dumps(detail, ensure_ascii=False, default=str, indent=1))
print('JSONEND')
