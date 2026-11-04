import unreal
rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
sub = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
try:
    sub.close_all_editors_for_asset(rig)
    print("closed")
except Exception as exc:
    print("close err", repr(exc))
print("level:", unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name())
