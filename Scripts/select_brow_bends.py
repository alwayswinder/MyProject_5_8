"""选中两个弯曲控制，配合截图。"""
import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    hc = rig.get_hierarchy_controller()
    keys = [unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=n)
            for n in ('CTRL_BROW_L_BEND', 'CTRL_BROW_R_BEND')]
    hc.set_selection(keys)
    h = rig.get_hierarchy()
    out = []
    for k in keys:
        st = h.get_control_settings(k)
        off = h.get_global_control_offset_transform(k).translation
        out.append({'name': str(k.name), 'shape': str(st.shape_name), 't': [round(off.x, 2), round(off.y, 2), round(off.z, 2)]})
    print(out)


if __name__ == '__main__':
    main()
