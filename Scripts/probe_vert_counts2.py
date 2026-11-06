import unreal, json
sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
out = {}
for tag, path in (("old", "/Game/_MyTest/Mouth/Mesh/Seven"), ("new", "/Game/_MyTest/Mouth/TMp/Seven_Skeletal_2026-11-06")):
    m = unreal.load_asset(path)
    rec = {}
    try: rec["num_verts"] = sub.get_num_verts(m, 0)
    except Exception as e: rec["num_verts"] = "ERR %r" % e
    try: rec["num_sections"] = sub.get_num_sections(m, 0)
    except Exception as e: rec["num_sections"] = "ERR %r" % e
    try: rec["morphs"] = len(m.get_editor_property("morph_targets") or [])
    except Exception as e: rec["morphs"] = "ERR"
    out[tag] = rec
print("JSONSTART")
print(json.dumps(out, ensure_ascii=False, indent=1))
print("JSONEND")
