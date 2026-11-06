-- 导出当前场景的网格 + 骨骼 + Dummy 为 FBX
(
	FBXExporterSetParam "Animation" false
	FBXExporterSetParam "BakeAnimation" false
	FBXExporterSetParam "Triangulate" false
	FBXExporterSetParam "SmoothingGroups" true
	FBXExporterSetParam "PreserveInstances" false
	FBXExporterSetParam "EmbedTextures" false

	local objs = for o in objects where (classOf o == PolyMeshObject or classOf o == Editable_mesh or classOf o == BoneGeometry or classOf o == Dummy or classOf o == Biped_Object) collect o
	select objs
	exportFile "D:/Download/Seven_v2.fbx" #noPrompt selectedOnly:true
	"exported " + (objs.count as string) + " objects"
)
