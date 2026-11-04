import unreal
rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
ctrl = rig.get_controller(rig.get_default_model())
removed = []
with unreal.ScopedEditorTransaction("Remove probe nodes"):
    for n in list(rig.get_default_model().get_nodes()):
        if n.get_name().startswith("ZZ_"):
            ctrl.remove_node(n, False)
            removed.append(n.get_name())
    rig.recompile_vm()
print("removed", removed)
print("status", rig.get_editor_property("status"))
names = [n.get_name() for n in rig.get_default_model().get_nodes()]
print("left", len(names), [n for n in names if not (n.startswith("BROW_") or n.startswith("Get_") or n.startswith("Set_") or n == "Begin")])
unreal.EditorAssetLibrary.save_loaded_asset(rig, False)
print("saved")
