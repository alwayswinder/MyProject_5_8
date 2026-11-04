"""列出本次要用的 RigVM 节点引脚，以及 ControlRigBlueprint 的 preview 属性名。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
NODES = [
    'RigUnit_GetControlVector',
    'RigUnit_GetTransform',
    'RigUnit_SetTransform',
    'RigUnit_SetTranslation',
    'RigVMFunction_MathVectorScale',
    'RigVMFunction_MathVectorAdd',
    'RigVMFunction_MathTransformMake',
]


def pin_info(node):
    out = []
    for p in node.get_pins():
        try:
            nm = p.get_name()
        except AttributeError:
            nm = str(p)
        try:
            ct = p.get_cpp_type().get_name()
        except Exception:
            ct = '?'
        try:
            dirn = str(p.get_direction())
        except Exception:
            dirn = '?'
        out.append('%s(%s,%s)' % (nm, ct, dirn))
    return out


def main():
    rig = unreal.load_asset(ASSET)
    ctrl = rig.get_controller(rig.get_default_model())
    out = {'pins': {}, 'preview_props': []}

    for name in NODES:
        cls = getattr(unreal, name, None)
        if cls is None:
            out['pins'][name] = '<missing class>'
            continue
        try:
            path = cls.static_struct().get_path_name()
            node = ctrl.add_unit_node_from_struct_path(path, 'Execute', unreal.Vector2D(0, 3000), 'ZZ_' + name, False)
            out['pins'][name] = pin_info(node)
            ctrl.remove_node(node, False)
        except Exception as exc:
            out['pins'][name] = 'ERR %r' % exc

    try:
        rig.recompile_vm()
    except Exception:
        pass

    for p in dir(rig):
        if 'preview' in p.lower() or 'mesh' in p.lower():
            out['preview_props'].append(p)

    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
