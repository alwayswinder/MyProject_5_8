"""打印 RigUnit_GetTransform.Transform 与 MathTransformMake 的子引脚名。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def walk(pin, depth=0, out=None):
    if out is None:
        out = []
    for sp in pin.get_sub_pins():
        out.append('%s%s' % ('  ' * depth, sp.get_name()))
        walk(sp, depth + 1, out)
    return out


def main():
    rig = unreal.load_asset(ASSET)
    ctrl = rig.get_controller(rig.get_default_model())
    out = {}
    for name in ('RigUnit_GetTransform', 'RigVMFunction_MathTransformMake'):
        cls = getattr(unreal, name)
        path = cls.static_struct().get_path_name()
        node = ctrl.add_unit_node_from_struct_path(path, 'Execute', unreal.Vector2D(0, 4000), 'ZZP_' + name, False)
        info = {}
        for p in node.get_pins():
            subs = walk(p)
            info[p.get_name()] = subs
        out[name] = info
        ctrl.remove_node(node, False)
    rig.recompile_vm()
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
