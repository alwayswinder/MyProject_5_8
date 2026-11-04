import unreal
rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
sub = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
sub.open_editor_for_assets([rig])
inst = rig.get_debugged_control_rig()
print("preview:", inst)
print("hierarchy count:", len(rig.get_hierarchy().get_all_keys()))
