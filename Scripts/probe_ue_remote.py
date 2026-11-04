import unreal

unreal.log("[BrowCtl] remote hello")
print("engine =", unreal.SystemLibrary.get_engine_version())
print("project =", unreal.Paths.project_dir())
rig = unreal.load_asset('/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders')
print("rig =", rig, rig.get_class().get_name() if rig else None)
