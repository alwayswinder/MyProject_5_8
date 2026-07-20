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

	// --- 材质引用 (对齐 BP CoreSimMaterials + OutputMaterials) ---

	// CoreSimMaterials[0]: MI_CollisionPainter_Dot
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CPD(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CollisionPainter_Dot.MI_CollisionPainter_Dot"));
	UMaterialInstance* Mat_CollisionPainterDot = CPD.Succeeded() ? CPD.Object : nullptr;

	// CoreSimMaterials[1]: MI_CollisionPainter_Line
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CPL(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CollisionPainter_Line.MI_CollisionPainter_Line"));
	UMaterialInstance* Mat_CollisionPainterLine = CPL.Succeeded() ? CPL.Object : nullptr;

	// CoreSimMaterials[2]: MI_CompositeAndGradient
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CAG(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CompositeAndGradient.MI_CompositeAndGradient"));
	UMaterialInstance* Mat_CompositeGradient = CAG.Succeeded() ? CAG.Object : nullptr;

	// CoreSimMaterials[4]: MI_Advection
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> ADV(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Advection.MI_Advection"));
	UMaterialInstance* Mat_Advection = ADV.Succeeded() ? ADV.Object : nullptr;

	// CoreSimMaterials[6]: MI_Divergence
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> DIV(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Divergence.MI_Divergence"));
	UMaterialInstance* Mat_Divergence = DIV.Succeeded() ? DIV.Object : nullptr;

	// CoreSimMaterials[8]: MI_Pressure_Solver2_Step1
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PSI(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver2_Step1.MI_Pressure_Solver2_Step1"));
	UMaterialInstance* Mat_PressureInit = PSI.Succeeded() ? PSI.Object : nullptr;

	// CoreSimMaterials[9]: MI_Pressure_Solver1
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PS1(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver1.MI_Pressure_Solver1"));
	UMaterialInstance* Mat_PressureIter = PS1.Succeeded() ? PS1.Object : nullptr;

	// CoreSimMaterials[12]: MI_Pressure_Solver2_Step2
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> PSS(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_Pressure_Solver2_Step2.MI_Pressure_Solver2_Step2"));
	UMaterialInstance* Mat_PressureCorrection = PSS.Succeeded() ? PSS.Object : nullptr;

	// CoreSimMaterials[16]: MI_CollisionPainterOffset
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CPO(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CollisionPainterOffset.MI_CollisionPainterOffset"));
	UMaterialInstance* Mat_CollisionPainterOffset = CPO.Succeeded() ? CPO.Object : nullptr;

	// --- OutputMaterials (对齐 BP 9 个) ---
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OM0(
		TEXT("/Game/FluidNinjaLive/OutputMaterials/Instance_Buffers/MI_DensityBuffer_Red.MI_DensityBuffer_Red"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> OM1(
		TEXT("/Game/FluidNinjaLive/OutputMaterials/Instance_Buffers/MI_DensityBuffer_Translucent.MI_DensityBuffer_Translucent"));

	// --- 预设 DataTable (对齐 BP DT_NinjaLive_Default) ---
	static ConstructorHelpers::FObjectFinder<UDataTable> PresetDT(
		TEXT("/Game/FluidNinjaLive/Presets/DT_NinjaLive_Default.DT_NinjaLive_Default"));

	// --- 不活跃灰化材质 ---
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> InactiveMat(
		TEXT("/Game/FluidNinjaLive/Core/Materials/MI_FluidNinjaLive_TraceMesh_Inactive.MI_FluidNinjaLive_TraceMesh_Inactive"));

	// --- TraceMesh (DisplayPlane) ---
	DisplayPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TraceMesh"));
	DisplayPlane->SetupAttachment(Root);
	DisplayPlane->SetRelativeScale3D(TraceMeshSize);
	DisplayPlane->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DisplayPlane->SetCollisionObjectType(ECC_WorldStatic);
	DisplayPlane->SetCollisionResponseToAllChannels(ECR_Ignore);
	DisplayPlane->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	DisplayPlane->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	DisplayPlane->SetGenerateOverlapEvents(false);
	DisplayPlane->SetCastShadow(true);
	DisplayPlane->bEvaluateWorldPositionOffsetInRayTracing = true;

	// BP 使用 NinjaLiveTraceMesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TraceMeshAsset(
		TEXT("/Game/FluidNinjaLive/NinjaLiveTraceMesh.NinjaLiveTraceMesh"));
	if (TraceMeshAsset.Succeeded())
		DisplayPlane->SetStaticMesh(TraceMeshAsset.Object);
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackPlane(
			TEXT("/Game/FluidNinjaLive/Tutorial/Meshes/SM_plane_400x400.SM_plane_400x400"));
		if (FallbackPlane.Succeeded())
			DisplayPlane->SetStaticMesh(FallbackPlane.Object);
	}

	// --- EditorIcon ---
	EditorIcon = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("EditorIcon"));
	EditorIcon->SetupAttachment(Root);
	EditorIcon->SetRelativeLocation(FVector(0.0f, 25.0f, 25.0f));  // BP CDO

	// --- NinjaLiveComponent ---
	NinjaLiveComponent = CreateDefaultSubobject<UMyNinjaLiveComponent>(TEXT("NinjaLiveComponent"));
	NinjaLiveComponent->bCreateDefaultDisplayPlane = false;

	// 传递材质到组件
	if (Mat_Advection) NinjaLiveComponent->AdvectionMat = Mat_Advection;
	if (Mat_CompositeGradient) NinjaLiveComponent->CompositeGradientMat = Mat_CompositeGradient;
	if (Mat_Divergence) NinjaLiveComponent->DivergenceMat = Mat_Divergence;
	if (Mat_PressureInit) NinjaLiveComponent->PressureSolverInitMat = Mat_PressureInit;
	if (Mat_PressureIter) NinjaLiveComponent->PressureSolverIterMat = Mat_PressureIter;
	if (Mat_PressureCorrection) NinjaLiveComponent->PressureCorrectionMat = Mat_PressureCorrection;
	if (Mat_CollisionPainterDot) NinjaLiveComponent->CollisionPainterDotMat = Mat_CollisionPainterDot;
	if (Mat_CollisionPainterLine) NinjaLiveComponent->CollisionPainterLineMat = Mat_CollisionPainterLine;
	if (Mat_CollisionPainterOffset) NinjaLiveComponent->CollisionPainterOffsetMat = Mat_CollisionPainterOffset;
	if (OM0.Succeeded()) NinjaLiveComponent->DisplayMat = OM0.Object;

	// 添加 OutputMaterials (对齐 BP 9 个)
	if (OM0.Succeeded()) NinjaLiveComponent->OutputMaterials.Add(OM0.Object);
	if (OM1.Succeeded()) NinjaLiveComponent->OutputMaterials.Add(OM1.Object);
	NinjaLiveComponent->OutputMaterialSelected = 1;

	// 预设
	if (PresetDT.Succeeded()) NinjaLiveComponent->DefaultPreset = PresetDT.Object;
	NinjaLiveComponent->PresetNameFilterCriteria = TEXT("NinjaLive");
	NinjaLiveComponent->bForceAutoLoadPreset = true;

	// 不活跃材质
	if (InactiveMat.Succeeded()) InactiveGrayMaterial = InactiveMat.Object;

	// --- ActivationVolume ---
	ActivationVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationVolume"));
	ActivationVolume->SetupAttachment(Root);
	ActivationVolume->SetBoxExtent(ActivationVolumeSize * 50.0f);
	ActivationVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ActivationVolume->SetCollisionObjectType(ECC_WorldDynamic);
	ActivationVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ActivationVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ActivationVolume->SetGenerateOverlapEvents(true);
	ActivationVolume->SetHiddenInGame(true);

	// --- InteractionVolume ---
	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Root);
	InteractionVolume->SetBoxExtent(InteractionVolumeSize * 50.0f);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionVolume->SetGenerateOverlapEvents(true);
	InteractionVolume->SetHiddenInGame(true);
	InteractionVolume->BodyInstance.MaxAngularVelocity = 3600.0f;

	// 默认重叠过滤 (BP: 全部类型)
	OverlapFilterInclusiveObjType = { ObjectTypeQuery1, ObjectTypeQuery2, ObjectTypeQuery3, ObjectTypeQuery4 };
}

void ASimpleFluidActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (NinjaLiveComponent && DisplayPlane)
	{
		NinjaLiveComponent->ExternalDisplayPlane = DisplayPlane;

		// 在编辑器中预显示
		if (NinjaLiveComponent->DisplayMat)
			DisplayPlane->SetMaterial(0, NinjaLiveComponent->DisplayMat);
	}

	// 体积缩放 (BP CDO 50x → 实际)
	if (ActivationVolume)
		ActivationVolume->SetBoxExtent(FVector(
			ActivationVolumeSize.X * 50.0f, ActivationVolumeSize.Y * 50.0f, ActivationVolumeSize.Z * 50.0f));
	if (InteractionVolume)
		InteractionVolume->SetBoxExtent(FVector(
			InteractionVolumeSize.X * 50.0f, InteractionVolumeSize.Y * 50.0f, InteractionVolumeSize.Z * 50.0f));
	if (DisplayPlane)
		DisplayPlane->SetRelativeScale3D(TraceMeshSize);

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

	NinjaLiveComponent->AddInteractionTarget(Other);

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

	// 2. 骨骼名过滤
	if (OverlapFilterInclusiveBoneNameExact.Num() > 0)
	{
		if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(OtherComp))
		{
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
			else return false;
		}
		else return false;
	}

	return true;
}

FVector2D ASimpleFluidActor::OverlapToSimUV(const FVector& WorldPos) const
{
	if (!NinjaLiveComponent) return FVector2D(0.5f, 0.5f);
	return NinjaLiveComponent->WorldToSimUV(WorldPos);
}

void ASimpleFluidActor::SyncPropertiesToComponent()
{
	if (!NinjaLiveComponent) return;

	NinjaLiveComponent->ExternalDisplayPlane = DisplayPlane;
	NinjaLiveComponent->bComponentActivatedByPawnProximity = SimActivatedByPawnProximity;

	// 组件覆盖属性
	if (OverrideComponentVariables)
	{
		NinjaLiveComponent->ResolutionX = Override_ResolutionX;
		NinjaLiveComponent->ResolutionY = Override_ResolutionY;
		NinjaLiveComponent->GlobalBrushScale = Override_GlobalBrushScale;
		NinjaLiveComponent->OutputMaterialSelected = Override_OutputMaterialSelected;
		NinjaLiveComponent->bLOD1_ReduceSimQuality = Override_bLOD1_ReduceSimQuality;
		NinjaLiveComponent->bLOD2_ReduceSamplingFPS = Override_bLOD2_ReduceSamplingFPS;
		NinjaLiveComponent->LOD_NearBound = Override_LOD_NearBound;
		NinjaLiveComponent->LOD_FarBound = Override_LOD_FarBound;
	}
}
