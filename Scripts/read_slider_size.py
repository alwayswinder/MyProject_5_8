import unreal

RIG = '/Game/_MyTest/Mouth/Rig/CR_Seven_MorphSliders'
MORPHS = ['1_happy', '1_happy_l_es', '1_happy_r_es', '1_hellow',
          '1_hou', '1_jian', '1_jing']

bp = unreal.load_asset(RIG)
h = bp.get_hierarchy()

lib = unreal.load_asset('/ControlRig/Controls/DefaultGizmoLibraryNormalized')
definition = next(s for s in lib.get_editor_property('shapes') if str(s.shape_name) == 'Sphere_Solid')
b = definition.get_editor_property('shape_proxy').get_bounds()
ds = definition.get_editor_property('transform').scale3d
unit = 2 * max(b.box_extent.x * abs(ds.x), b.box_extent.y * abs(ds.y), b.box_extent.z * abs(ds.z))

print('gizmo unit diameter = %.2f cm' % unit)
prev_z = None
for m in MORPHS:
    k = unreal.RigElementKey(type=unreal.RigElementType.CONTROL, name='CTRL_' + m)
    off = h.get_local_transform(k, True)
    shp = h.get_control_shape_transform(k, True)
    dia = unit * abs(shp.scale.x) * abs(off.scale.x)
    dz = '' if prev_z is None else ' dz=%.1f' % (prev_z - off.translation.z)
    prev_z = off.translation.z
    print('%-16s handle=%.2f cm  scale=%.3f  z=%.1f%s' % (m, dia, off.scale.x, off.translation.z, dz))
