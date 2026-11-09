import unreal,json
bp=unreal.load_asset('/Game/_MyTest/Mouth/LidDiagnostic/CR_Seven_LidWorldRotation');live=bp.get_debugged_control_rig();h=live.get_hierarchy()
print('LIVE',live.get_path_name())
for n in ('CTRL_EYE_EXPR_QI_R','CTRL_EYE_EXPR_NU_L'):
 k=unreal.RigElementKey(type=unreal.RigElementType.CONTROL,name=n);print('CTRL',n,h.get_vector_from_control_value(h.get_control_value(k)))
print('SCOPE',[(k,str(globals().get(k))) for k in ('_eyeexpr_frame','_eyeexpr_handle')])
for c in unreal.ObjectIterator(unreal.SkeletalMeshComponent):
 if c.get_skinned_asset()==bp.get_preview_mesh() and 'AnimationEditorPreviewActor' in c.get_path_name():print('COMP',c.get_path_name(),c.get_anim_instance().get_curve_value('1_qi_l_es'))
