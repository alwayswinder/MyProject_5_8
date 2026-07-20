// SimpleFluidActor.cpp
// FluidNinjaLive Actor — 包装 UMyNinjaLiveComponent

#include "SimpleFluidActor.h"
#include "MyNinjaLiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstance.h"
#include "Engine/World.h"

// ============================================================================
// 构造
// ============================================================================
ASimpleFluidActor::ASimpleFluidActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// --- 根 SceneComponent ---
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// --- 预先加载默认材质（供后续 DisplayPlane 和 NinjaLiveComponent 使用）---
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> DisplayMatFinder(
		TEXT("/Game/FluidNinjaLive/OutputMaterials/Instance_Buffers/MI_DensityBuffer_Red.MI_DensityBuffer_Red"));
	if (DisplayMatFinder.Succeeded())
		DisplayMat = DisplayMatFinder.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CompGradientMatFinder(
		TEXT("/Game/FluidNinjaLive/Core/FluidSim/MI_Float/MI_CompositeAndGradient_HDnoise.MI_CompositeAndGradient_HDnoise"));
	if (CompGradientMatFinder.Succeeded())
		CompositeGradientMat = CompGradientMatFinder.Object;

	// --- 显示平面 / TraceMesh ---
	// 注意：BP 中可能覆盖了 StaticMesh（例如 SM_plane_400x400）。
	// PlaneWorldSize 默认值从网格包围盒计算得出。
	// 碰撞：只响应 Visibility 追踪（不阻挡玩家/物理）
	DisplayPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayPlane"));
	DisplayPlane->SetupAttachment(Root);
	DisplayPlane->SetRelativeScale3D(FVector(20.0f, 20.0f, 1.0f));
	DisplayPlane->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DisplayPlane->SetCollisionObjectType(ECC_WorldStatic);
	DisplayPlane->SetCollisionResponseToAllChannels(ECR_Ignore);
	DisplayPlane->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	DisplayPlane->SetGenerateOverlapEvents(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
		DisplayPlane->SetStaticMesh(PlaneMesh.Object);

	// 给 DisplayPlane 设置默认材质（编辑器视口中可见，PIE 中被 MID_Display 替换）
	if (DisplayMat)
		DisplayPlane->SetMaterial(0, DisplayMat);

	// --- MyNinjaLiveComponent ---
	NinjaLiveComponent = CreateDefaultSubobject<UMyNinjaLiveComponent>(TEXT("NinjaLiveComponent"));
	NinjaLiveComponent->bCreateDefaultDisplayPlane = false; // 使用自己的 DisplayPlane

	// --- Default simulation parameters ---
	// 分辨率提升到 512 以获得可见效果；画笔增大
	ResolutionX = 512;
	ResolutionY = 512;
	BrushSize = 0.02f;   // 2% UV 空间 → 512×512 RT 上约 10px 半径
	Dissipation = 0.99f;  // 密度消散稍慢以便观察

	// --- 激活体积 ---
	// BoxExtent 在 OnConstruction 中根据 PlaneWorldSize 自动计算
	ActivationVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationVolume"));
	ActivationVolume->SetupAttachment(Root);
	ActivationVolume->SetBoxExtent(ActivationVolumeExtent);
	ActivationVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ActivationVolume->SetHiddenInGame(true);

	// --- 交互体积 ---
	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(Root);
	InteractionVolume->SetBoxExtent(InteractionVolumeExtent);
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionVolume->SetHiddenInGame(true);

	// 默认 ObjectTypes（WorldStatic 即可跟踪 ECC_Visibility）
	ObjectTypes = {
		ObjectTypeQuery1,  // WorldStatic
	};
}

// ============================================================================
// OnConstruction — 编辑器放置/修改时同步
// ============================================================================
void ASimpleFluidActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 将 DisplayPlane 设为组件的外部显示平面
	if (NinjaLiveComponent && DisplayPlane)
	{
		NinjaLiveComponent->ExternalDisplayPlane = DisplayPlane;
	}

	// 从网格包围盒自动计算 PlaneWorldSize
	// （BP 中可能覆盖 StaticMesh 为 SM_plane_400x400 等，自动适配）
	if (DisplayPlane && DisplayPlane->GetStaticMesh())
	{
		const FBoxSphereBounds Bounds = DisplayPlane->GetStaticMesh()->GetBounds();
		const float MeshWidth = Bounds.BoxExtent.X * 2.0f;  // 基础网格全宽
		const float ScaleXY = DisplayPlane->GetComponentScale().X;
		const float ComputedSize = MeshWidth * ScaleXY;

		// 只在计算值与当前值差异 >10% 时自动更新（避免覆盖用户手动设置）
		if (FMath::Abs(ComputedSize - PlaneWorldSize) > PlaneWorldSize * 0.1f)
		{
			PlaneWorldSize = ComputedSize;
		}
	}
	else
	{
		// 无网格时使用默认值（BasicShapes/Plane = 100 单位，Scale 20 = 2000）
		PlaneWorldSize = 2000.0f;
	}

	// 同步体积尺寸：XY 跟随 PlaneWorldSize
	const float HalfSize = PlaneWorldSize * 0.5f;
	ActivationVolumeExtent = FVector(HalfSize, HalfSize, FMath::Max(500.0f, HalfSize * 0.25f));
	InteractionVolumeExtent = FVector(HalfSize, HalfSize, 5.0f);

	if (ActivationVolume)
	{
		ActivationVolume->SetBoxExtent(ActivationVolumeExtent);
	}
	if (InteractionVolume)
	{
		InteractionVolume->SetBoxExtent(InteractionVolumeExtent);
	}

	// 给 DisplayPlane 应用默认显示材质（编辑器视口预览）
	if (DisplayMat && DisplayPlane)
	{
		DisplayPlane->SetMaterial(0, DisplayMat);
	}

	SyncPropertiesToComponent();
}

// ============================================================================
// BeginPlay
// ============================================================================
void ASimpleFluidActor::BeginPlay()
{
	// 运行时也同步一次体积尺寸
	if (ActivationVolume)
	{
		ActivationVolume->SetBoxExtent(ActivationVolumeExtent);
	}
	if (InteractionVolume)
	{
		InteractionVolume->SetBoxExtent(InteractionVolumeExtent);
	}

	SyncPropertiesToComponent();
	Super::BeginPlay();
}

// ============================================================================
// Tick
// ============================================================================
void ASimpleFluidActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ============================================================================
// 属性同步
// ============================================================================
void ASimpleFluidActor::SyncPropertiesToComponent()
{
	if (!NinjaLiveComponent) return;

	// ---- 激活 ----
	NinjaLiveComponent->bDisableComponent = bDisableComponent;
	NinjaLiveComponent->bActivatedByPawnProximity = bActivatedByPawnProximity;
	NinjaLiveComponent->ActivationDistance = ActivationDistance;

	// ---- 仿真 ----
	NinjaLiveComponent->ResolutionX = ResolutionX;
	NinjaLiveComponent->ResolutionY = ResolutionY;
	NinjaLiveComponent->SimFPS = SimFPS;
	NinjaLiveComponent->PressureIterations = PressureIterations;
	NinjaLiveComponent->Dissipation = Dissipation;

	// ---- 画笔 ----
	NinjaLiveComponent->BrushSize = BrushSize;
	NinjaLiveComponent->BrushStrength = BrushStrength;
	NinjaLiveComponent->BrushHardness = BrushHardness;

	// ---- 追踪 ----
	NinjaLiveComponent->ObjectTypes = ObjectTypes;
	NinjaLiveComponent->TraceDistance = TraceDistance;
	NinjaLiveComponent->PlaneWorldSize = PlaneWorldSize;
	NinjaLiveComponent->MaxVelocity = MaxVelocity;

	// ---- 材质 ----
	if (CollisionPainterDotMat) NinjaLiveComponent->CollisionPainterDotMat = CollisionPainterDotMat;
	if (CollisionPainterLineMat) NinjaLiveComponent->CollisionPainterLineMat = CollisionPainterLineMat;
	if (AdvectionMat) NinjaLiveComponent->AdvectionMat = AdvectionMat;
	if (CompositeGradientMat) NinjaLiveComponent->CompositeGradientMat = CompositeGradientMat;
	if (DivergenceMat) NinjaLiveComponent->DivergenceMat = DivergenceMat;
	if (PressureSolverMat) NinjaLiveComponent->PressureSolverMat = PressureSolverMat;
	if (DisplayMat) NinjaLiveComponent->DisplayMat = DisplayMat;

	// ---- Debug ----
	NinjaLiveComponent->bShowDebugMessages = bShowDebugMessages;
}
