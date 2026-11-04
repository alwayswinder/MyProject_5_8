import unreal
sm = unreal.load_asset("/Game/_MyTest/Mouth/Mesh/Seven")
skel = sm.get_editor_property("skeleton")
print([m for m in dir(skel) if "curve" in m.lower()])
print("---")
print([m for m in dir(unreal.SkeletalMesh) if "morph" in m.lower()][:40])
