// SimpleFluidActor.cpp — 全量复刻 NinjaLive BP 的 Actor 逻辑

#include "SimpleFluidActor.h"
#include "MyNinjaLiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/MaterialBillboardComponent.h"
#include "Materials/MaterialInstance.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"

ASimpleFluidActor::ASimpleFluidActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Root);

	// --- 默认材质（对齐原版 BP 的 coreSimMaterials 数组） ---
	// [0]: MI_CollisionPainter_Dot
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CDF(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CollisionPainter_Dot.MI_CollisionPainter_Dot"));
	if (CDF.Succeeded()) CollisionPainterDotMat = CDF.Object;

	// [1]: MI_CollisionPainter_Line
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CLF(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CollisionPainter_Line.MI_CollisionPainter_Line"));
	if (CLF.Succeeded()) CollisionPainterLineMat = CLF.Object;

	// [2]: MI_CompositeAndGradient_HDnoise（原版 Pool_0 使用 HDnoise 变体实现自然扰动）
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CGF(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CompositeAndGradient_HDnoise.MI_CompositeAndGradient_HDnoise"));
	if (CGF.Succeeded()) CompositeGradientMat = CGF.Object;

	// [4]: MI_Advection
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> AF(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Advection.MI_Advection"));
	if (AF.Succeeded()) AdvectionMat = AF.Object;

	// [6]: MI_Divergence
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> DF(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Divergence.MI_Divergence"));
	if (DF.Succeeded()) DivergenceMat = DF.Object;

	// [8]: MI_Pressure_Solver2_Step1（压力求解初始化）
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PS1(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver2_Step1.MI_Pressure_Solver2_Step1"));
	if (PS1.Succeeded()) PressureSolverMat = PS1.Object;

	// [9]: MI_Pressure_Solver1（迭代求解器）
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PF(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver1.MI_Pressure_Solver1"));
	if (PF.Succeeded()) PressureSolverIterMat = PF.Object;

	// [12]: MI_Pressure_Solver2_Step2（梯度修正 = v -= ∇p）
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PS2(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver2_Step2.MI_Pressure_Solver2_Step2"));
	if (PS2.Succeeded()) PressureCorrectionMat = PS2.Object;

	// --- 显示材质（对齐原版 OutputMaterials[0]）---
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> DMF(
		TEXT("/Game/FluidNinjaLive/OutputMaterials/Instance_Buffers/MI_DensityBuffer_Red.MI_DensityBuffer_Red"));
	if (DMF.Succeeded()) DisplayMat = DMF.Object;

	// --- 预设 DataTable (对齐原版 DT_Usecase_Pool2) ---
	static ConstructorHelpers::FObjectFinder<UDataTable> PresetDT(
		TEXT("/Game/FluidNinjaLive/Presets/DT_Usecase_Pool2.DT_Usecase_Pool2"));
	if (PresetDT.Succeeded()) DefaultPreset = PresetDT.Object;

	// --- 输出材质数组 (对齐原版 OutputMaterials) ---
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OutMat0(
		TEXT("/Game/FluidNinjaLive/OutputMaterials/Instance_Buffers/MI_DensityBuffer_Red.MI_DensityBuffer_Red"));
	if (OutMat0.Succeeded()) { OutputMaterials.Add(OutMat0.Object); }

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OutMat4(
		TEXT("/Game/FluidNinjaLive/UseCases/007_SmallWater/MaterialsMisc/MI_TraceMesh_Pool_Fluorescent.MI_TraceMesh_Pool_Fluorescent"));
	if (OutMat4.Succeeded())
	{
		while (OutputMaterials.Num() < 5) OutputMaterials.Add(nullptr);
		OutputMaterials[4] = OutMat4.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OutMat5(
		TEXT("/Game/FluidNinjaLive/UseCases/007_SmallWater/MaterialsMisc/MI_TraceMesh_Pool_Fluorescent_SingleLayerWater.MI_TraceMesh_Pool_Fluorescent_SingleLayerWater"));
	if (OutMat5.Succeeded())
	{
		while (OutputMaterials.Num() < 6) OutputMaterials.Add(nullptr);
		OutputMaterials[5] = OutMat5.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OutMat6(
		TEXT("/Game/FluidNinjaLive/UseCases/007_SmallWater/MaterialsMisc/MI_TraceMesh_Pool_Fluorescent_SingleLayerWater_Caustics_v2.MI_TraceMesh_Pool_Fluorescent_SingleLayerWater_Caustics_v2"));
	if (OutMat6.Succeeded())
	{
		while (OutputMaterials.Num() < 7) OutputMaterials.Add(nullptr);
		OutputMaterials[6] = OutMat6.Object;
	}

	// --- 压力梯度修正材质 (MI_Pressure_Solver2_Step2, 可选) ---
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PCM(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver2_Step2.MI_Pressure_Solver2_Step2"));
	if (PCM.Succeeded()) PressureCorrectionMat = PCM.Object;

	// --- DisplayPlane (对齐原版 TraceMesh) ---
	DisplayPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayPlane"));
	DisplayPlane->SetupAttachment(Root);
	DisplayPlane->SetRelativeScale3D(TraceMeshSize);
	DisplayPlane->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DisplayPlane->SetCollisionObjectType(ECC_WorldStatic);
	DisplayPlane->SetCollisionResponseToAllChannels(ECR_Ignore);
	DisplayPlane->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	DisplayPlane->SetGenerateOverlapEvents(false);
	DisplayPlane->SetCastShadow(true);
	// NOTE: BP TraceMesh bVisible=false (output via RT_External to other meshes).
	// For C++ test we keep it visible to see the simulation output on the plane.
	DisplayPlane->bEvaluateWorldPositionOffsetInRayTracing = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PM(
		TEXT("/Game/FluidNinjaLive/Tutorial/Meshes/SM_plane_400x400.SM_plane_400x400"));
	if (PM.Succeeded()) DisplayPlane->SetStaticMesh(PM.Object);
	if (DisplayMat) DisplayPlane->SetMaterial(0, DisplayMat);

	// --- EditorIcon (对齐原版 MaterialBillboardComponent) ---
	EditorIcon = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("EditorIcon"));
	EditorIcon->SetupAttachment(Root);
	EditorIcon->SetRelativeLocation(FVector(-3.907386f, -2.253428f, 149.694519f));
	// EditorIcon: 仅占位，材质在编译后由用户分配或保留默认
	// 原版使用 MaterialBillboardComponent 作为编辑器中可见图标
	// (AddElement API 因引擎版本差异留空)

	// --- NinjaLiveComponent ---
	NinjaLiveComponent = CreateDefaultSubobject<UMyNinjaLiveComponent>(TEXT("NinjaLiveComponent"));
	NinjaLiveComponent->bCreateDefaultDisplayPlane = false;

	// --- 激活体积 (BoxExtent=4000x4000x2500 = 原版) ---
	ActivationVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationVolume"));
	ActivationVolume->SetupAttachment(Root);
	ActivationVolume->SetBoxExtent(FVector(4000.0f, 4000.0f, 2500.0f));
	ActivationVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivationVolume->SetCollisionObjectType(ECC_WorldDynamic);
	ActivationVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivationVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ActivationVolume->SetGenerateOverlapEvents(true);
	ActivationVolume->SetHiddenInGame(true);

	// --- 交互体积 (BoxExtent=1025x1025x5, MaxAngularVelocity=3600) ---
	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Root);
	InteractionVolume->SetBoxExtent(FVector(1025.0f, 1025.0f, 5.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionVolume->SetGenerateOverlapEvents(true);
	InteractionVolume->SetHiddenInGame(true);
	InteractionVolume->BodyInstance.MaxAngularVelocity = 3600.0f;

	// 默认重叠过滤 (原版 Pool_0: WorldDynamic, Pawn, PhysicsBody — 排除 WorldStatic)
	OverlapFilterInclusiveObjType = { ObjectTypeQuery2, ObjectTypeQuery3, ObjectTypeQuery4 };
	// 原版 Pool_0: 只响应脚踝骨骼碰撞（实现脚步涟漪效果）
	OverlapFilterInclusiveBoneNameExact = { TEXT("calf_r"), TEXT("calf_l") };
	ObjectTypes = { ObjectTypeQuery1 };
}

void ASimpleFluidActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (NinjaLiveComponent && DisplayPlane)
		NinjaLiveComponent->ExternalDisplayPlane = DisplayPlane;

	// 编辑器预览：直接将 DisplayMat 应用到平面（非 PIE 时 BeginPlay 不会执行）
#if WITH_EDITOR
	if (DisplayMat && DisplayPlane)
	{
		DisplayPlane->SetMaterial(0, DisplayMat);
	}
#endif

	// 对齐原版：体积缩放逻辑
	if (ActivationVolume)
		ActivationVolume->SetBoxExtent(FVector(
			ActivationVolumeSize.X * 50.0f, ActivationVolumeSize.Y * 50.0f, ActivationVolumeSize.Z * 50.0f));
	if (InteractionVolume)
		InteractionVolume->SetBoxExtent(FVector(
			InteractionVolumeSize.X * 50.0f, InteractionVolumeSize.Y * 50.0f, InteractionVolumeSize.Z * 50.0f));
	if (DisplayPlane)
		DisplayPlane->SetRelativeScale3D(TraceMeshSize);

	// 对齐原版 PivotOffset — AActor 内置属性，无需手动设置 Root 位置
	// (PivotOffset 只影响编辑器中的枢轴视觉偏移，不影响 Transform)

	SyncPropertiesToComponent();
}

void ASimpleFluidActor::BeginPlay()
{
	// 绑定重叠事件
	if (InteractionVolume)
	{
		InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ASimpleFluidActor::OnInteractionVolumeBeginOverlap);
		InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &ASimpleFluidActor::OnInteractionVolumeEndOverlap);
	}
	if (ActivationVolume)
	{
		ActivationVolume->OnComponentBeginOverlap.AddDynamic(this, &ASimpleFluidActor::OnActivationVolumeBeginOverlap);
		ActivationVolume->OnComponentEndOverlap.AddDynamic(this, &ASimpleFluidActor::OnActivationVolumeEndOverlap);
	}

	// 运行时显示
	// DisplayPlane visibility managed by SetupDisplay

	SyncPropertiesToComponent();
	Super::BeginPlay();
}

// ============================================================================
// 重叠事件
// ============================================================================

void ASimpleFluidActor::OnInteractionVolumeBeginOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool, const FHitResult&)
{
	if (!DoesOverlapPassFilter(Other, OtherComp, OtherBodyIndex)) return;
	if (!NinjaLiveComponent) return;

	// ★ 添加到跟踪列表：Tick 中会持续读取位置并绘制
	NinjaLiveComponent->AddInteractionTarget(Other);

	// 立即画一次，确保帧首有反应
	FVector WorldPos = Other->GetActorLocation();
	FVector2D UV = OverlapToSimUV(WorldPos);
	NinjaLiveComponent->AddInteractionPoint(UV, Other->GetVelocity());
}

void ASimpleFluidActor::OnInteractionVolumeEndOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32)
{
	if (!NinjaLiveComponent) return;
	NinjaLiveComponent->RemoveInteractionTarget(Other);
}

void ASimpleFluidActor::OnActivationVolumeBeginOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!NinjaLiveComponent) return;
	NinjaLiveComponent->AddActiveTarget(Other);
}

void ASimpleFluidActor::OnActivationVolumeEndOverlap(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32)
{
	if (!NinjaLiveComponent) return;
	NinjaLiveComponent->RemoveActiveTarget(Other);
}

bool ASimpleFluidActor::DoesOverlapPassFilter(AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) const
{
	if (!Other || !OtherComp) return false;

	// 1. ObjectType 过滤
	if (OverlapFilterInclusiveObjType.Num() > 0)
	{
		ECollisionChannel Channel = OtherComp->GetCollisionObjectType();
		bool bMatch = false;
		for (const auto& FT : OverlapFilterInclusiveObjType)
		{
			if (UEngineTypes::ConvertToCollisionChannel(FT.GetValue()) == Channel) { bMatch = true; break; }
		}
		if (!bMatch) return false;
	}

	// 2. 骨骼名过滤 (对齐原版 OverlapFilterInclusiveBoneNameExact)
	if (OverlapFilterInclusiveBoneNameExact.Num() > 0)
	{
		if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(OtherComp))
		{
			// OtherBodyIndex 对应物理体索引，与骨骼索引一一对应
			if (OtherBodyIndex >= 0 && OtherBodyIndex < SkelComp->GetNumBones())
			{
				FName HitBoneName = SkelComp->GetBoneName(OtherBodyIndex);
				bool bBoneMatch = false;
				for (const FName& FilterBone : OverlapFilterInclusiveBoneNameExact)
				{
					if (HitBoneName == FilterBone) { bBoneMatch = true; break; }
				}
				if (!bBoneMatch) return false;
			}
			else
			{
				// BodyIndex 不在骨骼范围内，拒绝
				return false;
			}
		}
		// 非骨骼网格体但有骨骼过滤条件：拒绝（只有骨骼网格体才可能通过骨骼过滤）
		else
		{
			return false;
		}
	}

	return true;
}

FVector2D ASimpleFluidActor::OverlapToSimUV(const FVector& WorldPos) const
{
	if (!NinjaLiveComponent) return FVector2D(0.5f, 0.5f);
	FVector Origin = GetActorLocation();
	float Half = PlaneWorldSize * 0.5f;
	return FVector2D(
		(WorldPos.X - (Origin.X - Half)) / PlaneWorldSize,
		(WorldPos.Y - (Origin.Y - Half)) / PlaneWorldSize);
}

void ASimpleFluidActor::SyncPropertiesToComponent()
{
	if (!NinjaLiveComponent) return;

	NinjaLiveComponent->ExternalDisplayPlane = DisplayPlane;
	NinjaLiveComponent->ResolutionX = ResolutionX;
	NinjaLiveComponent->ResolutionY = ResolutionY;
	NinjaLiveComponent->PressureIterations = PressureIterations;
	NinjaLiveComponent->Dissipation = Dissipation;
	NinjaLiveComponent->PlaneWorldSize = PlaneWorldSize;
	NinjaLiveComponent->MaxVelocity = MaxVelocity;
	NinjaLiveComponent->GlobalBrushScale = GlobalBrushScale;
	NinjaLiveComponent->UserInputBrushScale = UserInputBrushScale;
	NinjaLiveComponent->BrushVelocityNoiseFreq = BrushVelocityNoiseFreq;
	NinjaLiveComponent->DampenBrushBelowThisVelocity = DampenBrushBelowThisVelocity;
	NinjaLiveComponent->UseCustomTraceSource = UseCustomTraceSource;
	NinjaLiveComponent->CustomTraceSourcePosition = CustomTraceSourcePosition;
	NinjaLiveComponent->TraceDistance = TraceDistance;
	NinjaLiveComponent->ObjectTypes = ObjectTypes;
	NinjaLiveComponent->bActivatedByPawnProximity = SimActivatedByPawnProximity;

	// 新增属性同步
	NinjaLiveComponent->BrushScaledByInteractingObjSize = BrushScaledByInteractingObjSize;
	if (DefaultPreset) NinjaLiveComponent->DefaultPreset = DefaultPreset;
	NinjaLiveComponent->PresetNameFilterCriteria = PresetNameFilterCriteria;

	if (OutputMaterials.Num() > 0)
	{
		NinjaLiveComponent->OutputMaterials = OutputMaterials;
		NinjaLiveComponent->OutputMaterialSelected = OutputMaterialSelected;
	}

	if (CollisionPainterDotMat) NinjaLiveComponent->CollisionPainterDotMat = CollisionPainterDotMat;
	if (CollisionPainterLineMat) NinjaLiveComponent->CollisionPainterLineMat = CollisionPainterLineMat;
	if (AdvectionMat) NinjaLiveComponent->AdvectionMat = AdvectionMat;
	if (CompositeGradientMat) NinjaLiveComponent->CompositeGradientMat = CompositeGradientMat;
	if (DivergenceMat) NinjaLiveComponent->DivergenceMat = DivergenceMat;
	if (PressureSolverMat) NinjaLiveComponent->PressureSolverMat = PressureSolverMat;
	if (PressureSolverIterMat) NinjaLiveComponent->PressureSolverIterMat = PressureSolverIterMat;
	if (DisplayMat) NinjaLiveComponent->DisplayMat = DisplayMat;
	if (PressureCorrectionMat) NinjaLiveComponent->PressureCorrectionMat = PressureCorrectionMat;

	NinjaLiveComponent->bShowDebugMessages = bShowDebugMessages;
}
