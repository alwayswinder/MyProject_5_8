"""删掉我刚才重复添加的 7 组节点（保留原有的那一套），再校验。"""
import unreal

RIG_PATH = "/Game/_MyTest/Mouth/Rig/CR_Seven_FaceMorphs"


def log(m):  unreal.log("[MorphRig] " + str(m))


rig = unreal.load_asset(RIG_PATH)
ctrl = rig.get_controller_by_name("RigVMModel")
graph = ctrl.get_graph()

removed, failed = 0, 0
for node in list(graph.get_nodes()):
    try:
        name = node.get_name()
    except Exception:
        continue
    if name.startswith("GetControl_") or name.startswith("SetCurve_"):
        try:
            ctrl.remove_node(node)
            removed += 1
        except Exception:
            try:
                ctrl.remove_node(unreal.Name(name))
                removed += 1
            except Exception as e:
                failed += 1
                log("删不掉 %s : %s" % (name, e))

log("removed=%d failed=%d" % (removed, failed))

# 重新统计
nodes = ctrl.get_graph().get_nodes()
gets = sum(1 for n in nodes if "GetControlFloat" in (n.get_script_struct().get_name() if n.get_script_struct() else ""))
sets = sum(1 for n in nodes if "SetCurveValue" in (n.get_script_struct().get_name() if n.get_script_struct() else ""))
log("节点总数=%d GetControlFloat=%d SetCurveValue=%d" % (len(nodes), gets, sets))

try:
    rig.recompile_control_rig()
except Exception as e:
    log("recompile: %s" % e)
unreal.EditorAssetLibrary.save_loaded_asset(rig, False)
log("saved")
