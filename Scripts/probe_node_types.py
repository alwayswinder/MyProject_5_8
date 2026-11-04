import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)
ctrl = rig.get_controller(rig.get_default_model())

CANDIDATES = [
    'RigUnit_SetBoneRotation', 'RigUnit_GetBoneRotation',
    'RigUnit_SetBoneTransform', 'RigUnit_GetBoneTransform',
    'RigUnit_SetBoneScale', 'RigUnit_GetBoneScale',
    'RigUnit_SetScale', 'RigUnit_GetScale',
    'RigUnit_SetCurveValue', 'RigUnit_GetControlFloat',
    'RigVMFunction_MathFloatMul', 'RigVMFunction_MathFloatSub',
    'RigVMFunction_MathFloatRemap', 'RigVMFunction_MathFloatDiv',
    'RigVMFunction_MathQuaternionFromAxisAndAngle',
    'RigVMFunction_MathQuaternionMul',
    'RigVMFunction_MathTransformFromLocationRotationScale',
    'RigVMFunction_MathTransformMul',
    'RigVMFunction_MathVectorMake',
    'RigVMFunction_MathQuaternionRotationOrder',
]

for name in CANDIDATES:
    cls = getattr(unreal, name, None)
    print(("HAS " if cls else "NO  ") + name)
