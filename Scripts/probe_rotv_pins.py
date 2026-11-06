"""探测 RigVM RotateVector / VectorSub 等节点的引脚名（建完即删，不保存）。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'

rig = unreal.load_asset(ASSET)
ctrl = rig.get_controller(rig.get_default_model())
out = {}
names = {
    'ROTV': 'RigVMFunction_MathQuaternionRotateVector',
    'VSUB': 'RigVMFunction_MathVectorSub',
    'VADD': 'RigVMFunction_MathVectorAdd',
    'VSCALE': 'RigVMFunction_MathVectorScale',
    'FADD': 'RigVMFunction_MathFloatAdd',
}

created = []
for tag, cls in names.items():
    try:
        path = getattr(unreal, cls).static_struct().get_path_name()
        node = ctrl.add_unit_node_from_struct_path(path, 'Execute', unreal.Vector2D(6000, 6000), 'ZZ_PROBE_' + tag, False)
        n = node.get_name()
        created.append(n)
        pins = [p.get_name() for p in node.get_pins()]
        out[tag] = pins
    except Exception as exc:
        out[tag] = 'ERR %r' % exc

for n in created:
    try:
        ctrl.remove_node(n, False)
    except Exception:
        pass

print('JSONSTART')
print(json.dumps(out, ensure_ascii=False, indent=1))
print('JSONEND')
