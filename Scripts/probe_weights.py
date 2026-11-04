import unreal
sm = unreal.load_asset("/Game/_MyTest/Mouth/Mesh/Seven")
print("SM:", [m for m in dir(sm) if "weight" in m.lower() or "influ" in m.lower() or "skin" in m.lower()])
print("HAS SkinWeightLibrary:", hasattr(unreal, "SkinWeightLibrary"))
print("HAS SkeletalMeshEditorSubsystem:", hasattr(unreal, "SkeletalMeshEditorSubsystem"))
try:
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    print("sub methods:", [m for m in dir(sub) if not m.startswith("_")][:40])
except Exception as e:
    print("sub err", repr(e))
