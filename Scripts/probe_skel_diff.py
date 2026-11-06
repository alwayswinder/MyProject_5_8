"""对比新旧 SkeletalMesh 的骨骼集合，找出新骨架缺失/新增的骨（尤其眼球骨）。"""
import json

import unreal

OLD = '/Game/_MyTest/Mouth/Mesh/Seven_Skeleton'
NEW = '/Game/_MyTest/Mouth/TMp/Seven_Skeletal_2026-11-06_Skeleton'


def bone_names(skel_path):
    sk = unreal.load_asset(skel_path)
    names = []
    try:
        ref = sk.get_editor_property('reference_skeleton')
    except Exception:
        ref = None
    # Skeleton 通过 SkeletalMesh 的骨骼树读更稳；这里用 Skeleton 的 preview mesh
    try:
        mesh = sk.get_editor_property('preview_mesh') or sk
    except Exception:
        mesh = sk
    # 通用：Skeleton 有 get_bone_tree? 用 SkeletalMesh 的骨骼名 API
    try:
        names = [str(n) for n in sk.get_editor_property('bone_names')]
    except Exception:
        names = []
    return names


out = {}
for tag, path in (('old', OLD), ('new', NEW)):
    sk = unreal.load_asset(path)
    rec = {'path': str(sk)}
    for attr in ('bone_names', 'skeleton_guid'):
        try:
            v = sk.get_editor_property(attr)
            rec[attr] = str(v)[:200]
        except Exception as exc:
            rec[attr] = 'ERR'
    # 通过 Skeleton 的 GetBoneNames? 试常用 API
    for fn in ('get_bone_names', 'get_reference_skeleton'):
        if hasattr(sk, fn):
            try:
                rec[fn] = str(getattr(sk, fn)())[:200]
            except Exception as exc:
                rec[fn] = 'ERR %r' % exc
    out[tag] = rec

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
