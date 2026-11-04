"""打印控制当前 shape transform 与 shape 的世界尺寸。"""
import json

import unreal

ASSET = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'


def main():
    rig = unreal.load_asset(ASSET)
    h = rig.get_hierarchy()
    out = {}
    for name in ('CTRL_BROW_L', 'CTRL_BROW_R'):
        k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name=name)
        st = h.get_control_settings(k)
        tf = h.get_global_control_shape_transform(k)
        off = h.get_global_control_offset_transform(k)
        local_shape = h.get_local_control_shape_transform(k)
        out[name] = {
            'shape_name': str(st.shape_name),
            'settings_shape_scale': [round(v, 4) for v in (st.shape_transform.scale3d.x,
                                                          st.shape_transform.scale3d.y,
                                                          st.shape_transform.scale3d.z)],
            'local_shape_scale': [round(v, 4) for v in (local_shape.scale3d.x,
                                                        local_shape.scale3d.y,
                                                        local_shape.scale3d.z)],
            'global_shape_scale': [round(v, 4) for v in (tf.scale3d.x, tf.scale3d.y, tf.scale3d.z)],
            'global_shape_t': [round(v, 2) for v in (tf.translation.x, tf.translation.y, tf.translation.z)],
            'control_scale': [round(v, 3) for v in (off.scale3d.x, off.scale3d.y, off.scale3d.z)],
        }
    print('JSONSTART')
    print(json.dumps(out, ensure_ascii=False, indent=1))
    print('JSONEND')


if __name__ == '__main__':
    main()
