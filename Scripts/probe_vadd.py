import unreal
for n in ("RigVMFunction_MathVectorAdd", "RigVMFunction_MathVectorMake", "RigUnit_GetBoneTransform",
          "RigUnit_SetTranslation", "RigUnit_SetRelativeTranslation", "RigUnit_SetTransform"):
    print(("HAS " if getattr(unreal, n, None) else "NO  ") + n)
rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
ctrl = rig.get_controller(rig.get_default_model())
cls = getattr(unreal, "RigUnit_GetBoneTransform", None)
if cls:
    node = ctrl.add_unit_node_from_struct_path(cls.static_struct().get_path_name(), "Execute",
                                               unreal.Vector2D(0, 3000), "ZZ_bt", False)
    print([ (p.get_name() if hasattr(p,"get_name") else str(p)) for p in node.get_pins() ])
    ctrl.remove_node(node, False)
