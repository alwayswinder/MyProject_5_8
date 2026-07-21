我正在进行一项工作，将著名的流体仿真插件FluidNinjaLive的核心部分，
/Script/Engine.Blueprint'/Game/FluidNinjaLive/NinjaLive.NinjaLive'和/Script/Engine.Blueprint'/Game/FluidNinjaLive/NinjaLiveComponent.NinjaLiveComponent'
在c++中复刻，
要求完整对应，参数、功能完全一致，
目前遇到了一些问题，无法看到正确的效果；
你可以在E:\Unreal\Projects\MyProject_5_8\docs\FluidBpInfo下获取原版的技术细节，也可以通过mcp查看实际蓝图实现；
对比蓝图帮我看看c++代码有哪些问题吗
以下是原版的在场景中演示的一个NinjaLive actor配置示例，c++中默认也是用一样的配置，最终希望达成一致的效果；
Begin Map
   Begin Level
      Begin Actor Class=/Game/FluidNinjaLive/NinjaLive.NinjaLive_C Name=Pool_0 Archetype="/Game/FluidNinjaLive/NinjaLive.NinjaLive_C'/Game/FluidNinjaLive/NinjaLive.Default__NinjaLive_C'" ExportPath="/Game/FluidNinjaLive/NinjaLive.NinjaLive_C'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0'"
         Begin Object Class=/Script/Engine.SceneComponent Name="Root" Archetype="/Script/Engine.SceneComponent'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:Root_GEN_VARIABLE'" ExportPath="/Script/Engine.SceneComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.Root'"
         End Object
         Begin Object Class=/Script/Engine.StaticMeshComponent Name="TraceMesh" Archetype="/Script/Engine.StaticMeshComponent'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:TraceMesh_GEN_VARIABLE'" ExportPath="/Script/Engine.StaticMeshComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.TraceMesh'"
         End Object
         Begin Object Class=/Script/Engine.MaterialBillboardComponent Name="EditorIcon" Archetype="/Script/Engine.MaterialBillboardComponent'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:EditorIcon_GEN_VARIABLE'" ExportPath="/Script/Engine.MaterialBillboardComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.EditorIcon'"
         End Object
         Begin Object Class=/Script/Engine.BoxComponent Name="ActivationVolume" Archetype="/Script/Engine.BoxComponent'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:ActivationVolume_GEN_VARIABLE'" ExportPath="/Script/Engine.BoxComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.ActivationVolume'"
            Begin Object Class=/Script/Engine.BodySetup Name="BodySetup_0" Archetype="/Script/Engine.BodySetup'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:ActivationVolume_GEN_VARIABLE.BodySetup_0'" ExportPath="/Script/Engine.BodySetup'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.ActivationVolume.BodySetup_0'"
            End Object
         End Object
         Begin Object Class=/Script/Engine.BoxComponent Name="InteractionVolume" Archetype="/Script/Engine.BoxComponent'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:InteractionVolume_GEN_VARIABLE'" ExportPath="/Script/Engine.BoxComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.InteractionVolume'"
            Begin Object Class=/Script/Engine.BodySetup Name="BodySetup_0" Archetype="/Script/Engine.BodySetup'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:InteractionVolume_GEN_VARIABLE.BodySetup_0'" ExportPath="/Script/Engine.BodySetup'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.InteractionVolume.BodySetup_0'"
            End Object
         End Object
         Begin Object Class=/Game/FluidNinjaLive/NinjaLiveComponent.NinjaLiveComponent_C Name="NinjaLiveComponent" Archetype="/Game/FluidNinjaLive/NinjaLiveComponent.NinjaLiveComponent_C'/Game/FluidNinjaLive/NinjaLive.NinjaLive_C:NinjaLiveComponent_GEN_VARIABLE'" ExportPath="/Game/FluidNinjaLive/NinjaLiveComponent.NinjaLiveComponent_C'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.NinjaLiveComponent'"
         End Object
         Begin Object Name="Root" ExportPath="/Script/Engine.SceneComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.Root'"
            RelativeLocation=(X=-128.054688,Y=927.440430,Z=-39.306030)
            UCSSerializationIndex=0
            bNetAddressable=True
            CreationMethod=SimpleConstructionScript
         End Object
         Begin Object Name="TraceMesh" ExportPath="/Script/Engine.StaticMeshComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.TraceMesh'"
            StaticMesh="/Script/Engine.StaticMesh'/Game/FluidNinjaLive/Tutorial/Meshes/SM_plane_400x400.SM_plane_400x400'"
            bEvaluateWorldPositionOffsetInRayTracing=True
            CastShadow=True
            AttachParent="Root"
            RelativeScale3D=(X=20.500000,Y=20.500000,Z=1.000000)
            bVisible=False
            UCSSerializationIndex=0
            bNetAddressable=True
            CreationMethod=SimpleConstructionScript
         End Object
         Begin Object Name="EditorIcon" ExportPath="/Script/Engine.MaterialBillboardComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.EditorIcon'"
            AttachParent="Root"
            RelativeLocation=(X=-3.907386,Y=-2.253428,Z=149.694519)
            UCSSerializationIndex=0
            bNetAddressable=True
            CreationMethod=SimpleConstructionScript
         End Object
         Begin Object Name="ActivationVolume" ExportPath="/Script/Engine.BoxComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.ActivationVolume'"
            Begin Object Name="BodySetup_0" ExportPath="/Script/Engine.BodySetup'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.ActivationVolume.BodySetup_0'"
               AggGeom=(BoxElems=((X=8000.000000,Y=8000.000000,Z=5000.000000)))
            End Object
            BoxExtent=(X=4000.000000,Y=4000.000000,Z=2500.000000)
            AttachParent="Root"
            bVisible=False
            UCSSerializationIndex=0
            bNetAddressable=True
            CreationMethod=SimpleConstructionScript
         End Object
         Begin Object Name="InteractionVolume" ExportPath="/Script/Engine.BoxComponent'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.InteractionVolume'"
            Begin Object Name="BodySetup_0" ExportPath="/Script/Engine.BodySetup'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.InteractionVolume.BodySetup_0'"
               AggGeom=(BoxElems=((X=2050.000000,Y=2050.000000,Z=10.000000)))
            End Object
            BoxExtent=(X=1025.000000,Y=1025.000000,Z=5.000000)
            BodyInstance=(MaxAngularVelocity=3599.999756)
            AttachParent="Root"
            bVisible=False
            UCSSerializationIndex=0
            bNetAddressable=True
            CreationMethod=SimpleConstructionScript
         End Object
         Begin Object Name="NinjaLiveComponent" ExportPath="/Game/FluidNinjaLive/NinjaLiveComponent.NinjaLiveComponent_C'/Game/_MyTest/FninjaTest/M_SmallWater02.M_SmallWater02:PersistentLevel.Pool_0.NinjaLiveComponent'"
            AutoConnectToMemoryPool-IF-Found=True
            ResolutionX=1600
            ResolutionY=1600
            LOD1-ReduceSimQuality=True
            LOD2-ReduceSamplingFPS=True
            LOD-NearBound=4000.000000
            LOD-FarBound=8000.000000
            DefaultPreset="/Script/Engine.DataTable'/Game/FluidNinjaLive/Presets/DT_Usecase_Pool2.DT_Usecase_Pool2'"
            PresetNameFilterCriteria="Usecase"
            OutputMaterialSelected=0
            OutputMaterials(4)="/Script/Engine.MaterialInstanceConstant'/Game/FluidNinjaLive/UseCases/007_SmallWater/MaterialsMisc/MI_TraceMesh_Pool_Fluorescent.MI_TraceMesh_Pool_Fluorescent'"
            OutputMaterials(5)="/Script/Engine.MaterialInstanceConstant'/Game/FluidNinjaLive/UseCases/007_SmallWater/MaterialsMisc/MI_TraceMesh_Pool_Fluorescent_SingleLayerWater.MI_TraceMesh_Pool_Fluorescent_SingleLayerWater'"
            OutputMaterials(6)="/Script/Engine.MaterialInstanceConstant'/Game/FluidNinjaLive/UseCases/007_SmallWater/MaterialsMisc/MI_TraceMesh_Pool_Fluorescent_SingleLayerWater_Caustics_v2.MI_TraceMesh_Pool_Fluorescent_SingleLayerWater_Caustics_v2'"
            OutputMaterials.RemoveIndex(8)
            OutputMaterials.RemoveIndex(7)
            OffsetFromSimAreaMotion=0.000000
            PV2_Connect_TrackpointsWithLines=True
            UseCustomTraceSource=True
            CustomTraceSourcePosition=(X=200.000000,Y=200.000000,Z=5000.000000)
            BrushScaledByInteractingObjSize=True
            GlobalBrushScale=4.000000
            UserInputBrushScale=1.200000
            BrushVelocityNoiseFreq=0.100000
            CollisionMask=None
            DampenBrushBelowThisVelocity=0.010000
            CoreSimMaterials(2)="/Script/Engine.MaterialInstanceConstant'/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CompositeAndGradient_HDnoise.MI_CompositeAndGradient_HDnoise'"
            AdjustPainter_V2_BrushVeloNoise=0.500000
            UCSSerializationIndex=0
            bNetAddressable=True
            CreationMethod=SimpleConstructionScript
         End Object
         NinjaLiveComponent="NinjaLiveComponent"
         InteractionVolume="InteractionVolume"
         ActivationVolume="ActivationVolume"
         EditorIcon="EditorIcon"
         TraceMesh="TraceMesh"
         Root="Root"
         ShowTraceMeshInEditor=False
         TraceMeshSize=(X=20.500000,Y=20.500000,Z=1.000000)
         SimActivatedByPawnProximity=True
         ActivationVolumeSize=(X=80.000000,Y=80.000000,Z=50.000000)
         InteractionVolumeSize=(X=20.500000,Y=20.500000,Z=0.100000)
         InteractionVolumeTemplate="InteractionVolume"
         OverlapFilterInclusiveObjType(0)=ObjectTypeQuery2
         OverlapFilterInclusiveObjType(1)=ObjectTypeQuery3
         OverlapFilterInclusiveObjType(2)=ObjectTypeQuery4
         OverlapFilterInclusiveObjType.RemoveIndex(3)
         OverlapFilterInclusiveBoneNameExact(0)="calf_r"
         OverlapFilterInclusiveBoneNameExact(1)="calf_l"
         TraceMeshInactiveBehaviour=NewEnumerator0
         RootComponent="Root"
         PivotOffset=(X=0.000000,Y=0.000000,Z=-0.787567)
         ActorLabel="Pool"
         FolderPath="Pool"
      End Actor
   End Level
Begin Surface
End Surface
End Map
