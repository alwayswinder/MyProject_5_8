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
        out.append(str(nm))
    return out


for name in ('RigVMFunction_MathFloatMul', 'RigVMFunction_MathFloatSub',
             'RigVMFunction_MathVectorMake', 'RigUnit_SetCurveValue',
             'RigVMFunction_MathQuaternionFromAxisAndAngle', 'RigVMFunction_MathFloatDiv'):
    cls = getattr(unreal, name, None)
    if cls is None:
        print("NO", name)
        continue
    path = cls.static_struct().get_path_name()
    node = ctrl.add_unit_node_from_struct_path(path, 'Execute', unreal.Vector2D(0, 2200), 'ZZ_' + name, False)
    print("=== %s %s" % (name, pin_info(node)))
    ctrl.remove_node(node, False)

# 控件偏移读取
h = rig.get_hierarchy()
for n in ('CTRL_1_happy', 'CTRL_1_jing', 'MorphBoard'):
    kind = unreal.RigElementType.NULL if n == 'MorphBoard' else unreal.RigElementType.CONTROL
    k = unreal.RigElementKey(type=kind, name=n)
    if h.contains(k):
        if kind == unreal.RigElementType.NULL:
            print("null %s global=%s" % (n, h.get_global_transform(k).translation))
        else:
            off = h.get_global_control_offset_transform(k)
            print("ctrl %s off_t=%s off_s=%s" % (n, off.translation, off.scale3d))
