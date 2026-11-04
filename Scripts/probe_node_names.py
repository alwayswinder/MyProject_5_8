import unreal
rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
names = [n.get_name() for n in rig.get_default_model().get_nodes()]
print("count", len(names))
print([n for n in names if not (n.startswith("BROW_") or n.startswith("Get_") or n.startswith("Set_") or n == "Begin")])
print("brow", len([n for n in names if n.startswith("BROW_")]))
