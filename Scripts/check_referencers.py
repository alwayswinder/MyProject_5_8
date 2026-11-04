import json
import unreal

out = {}
eal = unreal.EditorAssetLibrary
for p in ("/Game/CR_Seven_MorphSliders_Take1", "/Game/CR_Seven_MorphSliders_Take2"):
    try:
        a = eal.load_asset(p)
        info = {"class": a.get_class().get_name() if a else None, "path": a.get_path_name() if a else None}
        if a:
            try:
                info["seq"] = str(a.get_editor_property("sequence")) if hasattr(a, "get_editor_property") else None
            except Exception:
                pass
            try:
                info["tracks"] = [str(t.get_class().get_name()) for t in a.get_editor_property("tracks")]
            except Exception as exc:
                info["tracks"] = "n/a: %r" % (exc,)
        out[p] = info
    except Exception as exc:
        out[p] = "err %r" % (exc,)

rig = unreal.load_asset("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders")
h = rig.get_hierarchy()
out["rig"] = {
    "elements": len(h.get_all_keys()),
    "nodes": [n.get_name() for n in rig.get_default_model().get_nodes()],
    "status": str(rig.get_editor_property("status")),
    "package_dirty": unreal.EditorAssetLibrary.does_asset_exist("/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders"),
}
print(json.dumps(out, ensure_ascii=False, indent=1))
