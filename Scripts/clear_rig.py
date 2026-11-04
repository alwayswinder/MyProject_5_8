"""清空 CR_Seven_MorphSliders：删掉 RigVM 图节点、控制、null，再从 Seven mesh 全量重导骨骼。

幂等：重复跑结果一致。
用法：python Scripts\\ue_run.py Scripts\\clear_rig.py
"""
import json
import traceback

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
MESH = '/Game/_MyTest/Mouth/Mesh/Seven'


def main():
    report = {}
    rig = unreal.load_asset(ASSET)
    model = rig.get_default_model()
    ctrl = rig.get_controller(model)
    h = rig.get_hierarchy()
    hc = rig.get_hierarchy_controller()

    old_auto = rig.get_auto_vm_recompile()
    rig.modify()
    with unreal.ScopedEditorTransaction('Clear rig for brow rebuild'):
        rig.set_auto_vm_recompile(False)
        try:
            # 1) 图节点（Begin 保留）
            removed_nodes = []
            for n in list(model.get_nodes()):
                name = n.get_name()
                if name == 'Begin':
                    continue
                ctrl.remove_node(n, False)
                removed_nodes.append(name)
            report['removed_nodes'] = removed_nodes

            # 2) 控制 / null（控制先删，避免父级删除报错）
            removed_controls, removed_nulls, failed = [], [], []
            for want in (unreal.RigElementType.CONTROL, unreal.RigElementType.NULL):
                for k in list(h.get_all_keys()):
                    if k.type != want:
                        continue
                    try:
                        hc.remove_element(k)
                        (removed_controls if want == unreal.RigElementType.CONTROL else removed_nulls).append(str(k.name))
                    except Exception as exc:
                        failed.append('%s(%s): %r' % (k.name, k.type, exc))
            report['removed_controls'] = removed_controls
            report['removed_nulls'] = removed_nulls
            report['remove_failed'] = failed

            # 3) 骨骼全量重导（replace + remove obsolete）
            keys = hc.import_bones_from_skeletal_mesh(
                unreal.load_asset(MESH), 'None', True, True, False, False, False)
            report['imported_bones'] = len(list(keys))
            report['bone_names'] = [str(k.name) for k in keys]

            rig.recompile_vm()
            report['status'] = str(rig.get_editor_property('status'))
        finally:
            rig.set_auto_vm_recompile(old_auto)

    unreal.EditorAssetLibrary.save_loaded_asset(rig, False)
    print('JSONSTART')
    print(json.dumps(report, ensure_ascii=False, indent=1))
    print('JSONEND')
    return report


if __name__ == '__main__':
    try:
        main()
        unreal.log('[ClearRig] saved')
    except Exception:
        unreal.log_error('[ClearRig] FAILED\n' + traceback.format_exc())
        raise
