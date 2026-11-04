import unreal

rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
hc = rig.get_hierarchy_controller()

print("HC_METHODS:", sorted([m for m in dir(hc) if not m.startswith('_')]))

for name in ('RigUnit_SetBoneRotation', 'RigUnit_SetBoneScale', 'RigUnit_SetBoneTransform',
             'RigUnit_GetBoneTransform', 'RigUnit_SetCurveValue', 'RigUnit_GetControlFloat',
             'RigVMFunction_MathFloatMul', 'RigVMFunction_MathFloatAdd',
             'RigVMFunction_MathFloatLerp', 'RigVMFunction_MathFloatRemap',
             'RigUnit_SetRelativeRotationForItem', 'RigUnit_SetTranslation'):
    cls = getattr(unreal, name, None)
    if cls is None:
        print("MISSING", name)
        continue
    try:
        sp = cls.static_struct().get_path_name()
    except Exception as exc:
        sp = "err:%r" % (exc,)
    print("OK", name, sp)

print("VERSION", unreal.SystemLibrary.get_engine_version())
