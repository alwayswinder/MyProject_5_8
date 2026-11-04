import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
rig = unreal.load_asset(ASSET)

# 打开资产编辑器
try:
    sub = unreal.get_editor_subsystem(unreal.AssetEditorSubsystem)
    sub.open_editor_for_assets([rig])
    print("opened editor")
except Exception as exc:
    print("open err", repr(exc))

for fn in ('get_debugged_control_rig', 'get_preview_instance', 'get_preview_control_rig'):
    f = getattr(rig, fn, None)
    print(fn, "->", (f() if callable(f) else None))

inst = None
for fn in ('get_debugged_control_rig', 'get_preview_instance'):
    f = getattr(rig, fn, None)
    if f:
        try:
            inst = f()
        except Exception:
            inst = None
        if inst:
            break
print("instance:", inst, inst.get_class().get_name() if inst else None)
