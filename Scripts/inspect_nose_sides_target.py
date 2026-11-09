import unreal
bp=unreal.load_asset('/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation');h=bp.get_hierarchy();m=bp.get_default_model()
for n in ('bijian','bitou','Bone026','Bone034','CTRL_NOSE'):
 kind=unreal.RigElementType.CONTROL if n.startswith('CTRL') else unreal.RigElementType.BONE;k=unreal.RigElementKey(type=kind,name=n);print(n,h.get_global_transform(k,True))
print('NOSEMAKE',[(str(l.get_source_pin().get_pin_path()),str(l.get_target_pin().get_pin_path())) for l in m.get_links() if str(l.get_target_pin().get_pin_path()).startswith('NOSE_MAKE_bijian.')])
