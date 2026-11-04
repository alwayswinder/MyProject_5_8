"""把 CR_Seven_MorphSliders 清成空壳：删掉所有滑条控件 / 轨道 / 曲线 / 骨骼 / 图节点。

保留 Begin 节点。执行前已把 uasset 备份到 Saved/Codex/Backup。
"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
h = rig.get_hierarchy()
hc = rig.get_hierarchy_controller()
ctrl = rig.get_controller(rig.get_default_model())
model = rig.get_default_model()

report = {}

# 清理前：谁引用了这个 rig
try:
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(ASSET, False)
    report['referencers'] = [str(r) for r in refs]
except Exception as exc:
    report['referencers'] = 'err: %r' % (exc,)

before = [str(k.type).split('.')[-1] + ':' + str(k.name) for k in h.get_all_keys()]
report['before'] = before

# 1) 删节点（保留 Begin）
removed_nodes = []
with unreal.ScopedEditorTransaction('Clean all sliders from Seven MorphSliders rig'):
    for node in list(model.get_nodes()):
        name = node.get_name()
        if name == 'Begin':
            continue
        ctrl.remove_node(node, False)
        removed_nodes.append(name)

    # 2) 删元素：先控件、曲线、骨骼，最后 null（父级）
    order = [unreal.RigElementType.CONTROL,
             unreal.RigElementType.CURVE,
             unreal.RigElementType.BONE,
             unreal.RigElementType.NULL]
    removed_elements = []
    for kind in order:
        keys = [k for k in h.get_all_keys() if k.type == kind]
        # 反向删除，子级先走
        for k in reversed(keys):
            hc.remove_element(k, True, True)
            removed_elements.append(str(kind).split('.')[-1] + ':' + str(k.name))

    rig.recompile_vm()

report['removed_nodes'] = removed_nodes
report['removed_elements'] = removed_elements
report['status'] = str(rig.get_editor_property('status'))
report['after_elements'] = [str(k.type).split('.')[-1] + ':' + str(k.name) for k in h.get_all_keys()]
report['after_nodes'] = [n.get_name() for n in model.get_nodes()]

unreal.EditorAssetLibrary.save_loaded_asset(rig, False)
report['saved'] = True
print(json.dumps(report, ensure_ascii=False, indent=1))
