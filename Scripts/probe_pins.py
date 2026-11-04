import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
ctrl = rig.get_controller(rig.get_default_model())


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
        out.append((nm, str(ct)))
    return out


for name in ('RigUnit_SetBoneRotation', 'RigUnit_SetScale', 'RigUnit_SetTransform',
             'RigUnit_GetBoneTransform', 'RigVMFunction_MathQuaternionFromAxisAndAngle'):
    cls = getattr(unreal, name, None)
    if cls is None:
        continue
    try:
        path = cls.static_struct().get_path_name()
        node = ctrl.add_unit_node_from_struct_path(path, 'Execute', unreal.Vector2D(0, 2000), 'ZZ_' + name, False)
        print("=== %s\n    %s" % (name, pin_info(node)))
        ctrl.remove_node(node, False)
    except Exception as exc:
        print("ERR", name, repr(exc))
