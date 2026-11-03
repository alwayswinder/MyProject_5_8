"""补完 CR_Seven_FaceMorphs 的 morph 驱动链路，并做静态校验。

1. Skeleton 的 7 条曲线标记 Morph Target
2. 校验：曲线标记结果 + rig 里 Get/Set 节点数量与连线
"""
import unreal

RIG_PATH   = "/Game/_MyTest/Mouth/Rig/CR_Seven_FaceMorphs"
MESH_PATH  = "/Game/_MyTest/Mouth/Mesh/Seven"
CURVES = ["1_happy", "1_happy_l_es", "1_happy_r_es",
          "1_hellow", "1_hou", "1_jian", "1_jing"]


def log(m):  unreal.log("[MorphRig] " + str(m))


def main():
    # ---------- 1. 曲线标记 ----------
    mesh = unreal.load_asset(MESH_PATH)
    skel = mesh.get_editor_property("Skeleton")
    for name in CURVES:
        n = unreal.Name(name)
        try:
            skel.add_curve_meta_data(n, True)
        except Exception as e:
            log("add_curve_meta_data %s : %s" % (name, e))
        try:
            skel.set_curve_meta_data_morph_target(n, True)
        except Exception as e:
            log("set_morph_target %s : %s" % (name, e))
    unreal.EditorAssetLibrary.save_loaded_asset(skel, False)
    log("-- 曲线标记结果 --")
    for name in CURVES:
        log("  %-16s morph_target=%s" % (name, skel.get_curve_meta_data_morph_target(unreal.Name(name))))

    # ---------- 2. rig 图校验 ----------
    rig = unreal.load_asset(RIG_PATH)
    ctrl = rig.get_controller_by_name("RigVMModel")
    graph = ctrl.get_graph()
    nodes = graph.get_nodes()
    gets, sets = [], []
    for node in nodes:
        try:
            ss = node.get_script_struct()
            sname = ss.get_name() if ss else ""
        except Exception:
            sname = ""
        if "GetControlFloat" in sname:
            gets.append(node)
        elif "SetCurveValue" in sname:
            sets.append(node)
    log("-- rig 图 --")
    log("  节点总数=%d  GetControlFloat=%d  SetCurveValue=%d" % (len(nodes), len(gets), len(sets)))

    def pin_default(node, pin):
        try:
            for p in node.get_pins():
                if p.get_name() == pin:
                    return p.get_default_value()
        except Exception as e:
            return "ERR:%s" % e
        return None

    for node in sets:
        log("  curve=%s" % pin_default(node, "Curve"))
    for node in gets:
        log("  control=%s" % pin_default(node, "Control"))

    # ---------- 3. 曲线容器 ----------
    try:
        h = rig.get_hierarchy()
        names = [c.get_name() for c in h.get_curves()] if hasattr(h, "get_curves") else []
        log("rig curves = %s" % names)
    except Exception as e:
        log("曲线容器读取失败: %s" % e)


main()
