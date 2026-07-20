// MyNinjaLiveComponent.cpp
// C++ 实现 FluidNinjaLive 核心 ActorComponent

#include "MyNinjaLiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"

// ============================================================================
// 构造
// ============================================================================
UMyNinjaLiveComponent::UMyNinjaLiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bAutoActivate = true;

	// 默认 ObjectTypes：WorldStatic, WorldDynamic, Pawn, PhysicsBody
	ObjectTypes = {
		ObjectTypeQuery1,  // WorldStatic
		ObjectTypeQuery3,  // Pawn
		ObjectTypeQuery2,  // WorldDynamic
	};
}

// ============================================================================
// BeginPlay — 初始化 RT、材质实例、显示平面
// ============================================================================
void UMyNinjaLiveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bDisableComponent)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 核心管线材质（必须全部设置才能运行仿真）
	if (!AdvectionMat || !DivergenceMat || !PressureSolverMat || !CollisionPainterDotMat)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[MyNinjaLiveComponent] 缺少核心管线材质引用！请在细节面板中指定 AdvectionMat, DivergenceMat, PressureSolverMat, CollisionPainterDotMat。"));
		SetComponentTickEnabled(false);
		return;
	}

	// 以下为可选材质，缺失时仅打印警告，不影响主管线运行
	if (!CompositeGradientMat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MyNinjaLiveComponent] CompositeGradientMat 未设置，将跳过外力合成步骤。"));
	}

	if (!DisplayMat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MyNinjaLiveComponent] DisplayMat 未设置，仿真将运行但不会显示到平面。"
				 "请在细节面板中指定 DisplayMat（例如 MI_DensityBuffer_Red）。"));
	}

	// 仿真帧间隔
	if (SimFPS > 0)
		SimInterval = 1.0f / static_cast<float>(SimFPS);
	else
		SimInterval = 0.0f;

	// 创建内置平面
	if (!ExternalDisplayPlane && bCreateDefaultDisplayPlane)
		CreateDefaultPlane();

	// 创建 RT
	CreateAllRTs();

	// ★ 诊断：填充密度 RT 中间灰度 → 如果显示管线正确工作，平面应变红
	// 如果看到红色平面 = 显示管线 OK；如果仍是默认贴图 = 显示管线有问题
	// （确认管线正常后可删除此段代码）
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_DensityA,
			FLinearColor(0.3f, 0.0f, 0.0f, 1.0f));  // 30% 密度 → 可见红色
	}

	// 创建动态材质实例
	if (!CreateDynamicMaterialInstances())
	{
		SetComponentTickEnabled(false);
		return;
	}

	// 设置显示
	SetupDisplay();

	// 初始化多目标数组
	SimTargets.SetNum(MaxTargets);

	// 排除 Owner
	if (AActor* Owner = GetOwner())
		TraceExcludeActors.AddUnique(Owner);

	bInitialized = true;
	UE_LOG(LogTemp, Log,
		TEXT("[MyNinjaLiveComponent] 初始化完成 — %dx%d, 平面尺寸 %.0f"),
		ResolutionX, ResolutionY, PlaneWorldSize);
}

// ============================================================================
// EndPlay
// ============================================================================
void UMyNinjaLiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllRTs();
	Super::EndPlay(EndPlayReason);
}

// ============================================================================
// TickComponent — 主循环
// ============================================================================
void UMyNinjaLiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitialized || bDisableComponent || DeltaTime <= 0.0f)
		return;

	// FPS 限制
	if (SimInterval > 0.0f)
	{
		SimTimer += DeltaTime;
		if (SimTimer < SimInterval) return;
		DeltaTime = SimTimer;
		SimTimer = 0.0f;
	}

	// 玩家距离激活
	if (bActivatedByPawnProximity)
	{
		CheckPawnProximity();
		if (!bPawnInsideBounds) return;
	}

	// ---- 主仿真管线 ----
	RunSimulationPipeline(DeltaTime);
}

// ============================================================================
// 公开接口
// ============================================================================
void UMyNinjaLiveComponent::ForceEnable()
{
	if (bActivatedByPawnProximity)
	{
		CheckPawnProximity();
		if (!bPawnInsideBounds)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[MyNinjaLiveComponent] ForceEnable 失败：Pawn 不在激活范围内。"));
			return;
		}
	}
	bDisableComponent = false;
	SetComponentTickEnabled(true);
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] ForceEnable"));
}

void UMyNinjaLiveComponent::ForceDisable()
{
	bDisableComponent = true;
	SetComponentTickEnabled(false);
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] ForceDisable"));
}

void UMyNinjaLiveComponent::ResetSimulation()
{
	ClearAllRTs();
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] 仿真已重置"));
}

void UMyNinjaLiveComponent::SetBrush(float NewSize, float NewStrength, float NewHardness)
{
	BrushSize = FMath::Clamp(NewSize, 0.001f, 1.0f);
	BrushStrength = FMath::Max(0.0f, NewStrength);
	BrushHardness = FMath::Clamp(NewHardness, 0.0f, 1.0f);
}

void UMyNinjaLiveComponent::SetResolution(int32 NewX, int32 NewY)
{
	ResolutionX = FMath::Clamp(NewX, 64, 1024);
	ResolutionY = FMath::Clamp(NewY, 64, 1024);
}

// ============================================================================
// RenderTarget 创建
// ============================================================================
UTextureRenderTarget2D* UMyNinjaLiveComponent::CreateRT(
	const FName& Name, int32 SizeX, int32 SizeY)
{
	UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(this, Name);
	RT->RenderTargetFormat = RTF_RGBA16f;
	RT->ClearColor = FLinearColor::Black;
	RT->InitAutoFormat(SizeX, SizeY);
	RT->UpdateResourceImmediate(true);
	return RT;
}

void UMyNinjaLiveComponent::CreateAllRTs()
{
	RT_DensityA   = CreateRT(TEXT("RT_DensityA"),   ResolutionX, ResolutionY);
	RT_DensityB   = CreateRT(TEXT("RT_DensityB"),   ResolutionX, ResolutionY);
	RT_VelocityA  = CreateRT(TEXT("RT_VelocityA"),  ResolutionX, ResolutionY);
	RT_VelocityB  = CreateRT(TEXT("RT_VelocityB"),  ResolutionX, ResolutionY);
	RT_Pressure   = CreateRT(TEXT("RT_Pressure"),   ResolutionX, ResolutionY);
	RT_Divergence = CreateRT(TEXT("RT_Divergence"), ResolutionX, ResolutionY);
	RT_Collision  = CreateRT(TEXT("RT_Collision"),  ResolutionX, ResolutionY);
	RT_Collision2 = CreateRT(TEXT("RT_Collision2"), ResolutionX, ResolutionY);
	RT_External   = CreateRT(TEXT("RT_External"),   ResolutionX, ResolutionY);

	ClearAllRTs();
}

void UMyNinjaLiveComponent::SwapRT(UTextureRenderTarget2D*& A,
	UTextureRenderTarget2D*& B)
{
	UTextureRenderTarget2D* Tmp = A;
	A = B;
	B = Tmp;
}

void UMyNinjaLiveComponent::ClearAllRTs()
{
	auto Clear = [this](UTextureRenderTarget2D* RT) {
		if (RT) UKismetRenderingLibrary::ClearRenderTarget2D(this, RT, FLinearColor::Black);
	};
	Clear(RT_DensityA);
	Clear(RT_DensityB);
	Clear(RT_VelocityA);
	Clear(RT_VelocityB);
	Clear(RT_Pressure);
	Clear(RT_Divergence);
	Clear(RT_Collision);
	Clear(RT_Collision2);
	Clear(RT_External);
}

// ============================================================================
// 动态材质实例创建
// ============================================================================
bool UMyNinjaLiveComponent::CreateDynamicMaterialInstances()
{
	MID_CollisionPainterDot   = UMaterialInstanceDynamic::Create(CollisionPainterDotMat,   this);
	MID_CollisionPainterLine  = CollisionPainterLineMat
		? UMaterialInstanceDynamic::Create(CollisionPainterLineMat, this)
		: nullptr;
	MID_DensityInject         = DensityInjectMat
		? UMaterialInstanceDynamic::Create(DensityInjectMat, this)
		: MID_CollisionPainterDot;
	MID_Advection             = UMaterialInstanceDynamic::Create(AdvectionMat,             this);
	MID_CompositeGradient     = CompositeGradientMat
		? UMaterialInstanceDynamic::Create(CompositeGradientMat, this)
		: nullptr;  // 可选
	MID_Divergence            = UMaterialInstanceDynamic::Create(DivergenceMat,            this);
	MID_PressureSolver        = UMaterialInstanceDynamic::Create(PressureSolverMat,        this);
	MID_Display               = DisplayMat
		? UMaterialInstanceDynamic::Create(DisplayMat, this)
		: nullptr;  // 可选

	if (!MID_CollisionPainterDot || !MID_Advection ||
		!MID_Divergence || !MID_PressureSolver)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[MyNinjaLiveComponent] 核心动态材质实例创建失败"));
		return false;
	}
	return true;
}

// ============================================================================
// 内置平面创建
// ============================================================================
void UMyNinjaLiveComponent::CreateDefaultPlane()
{
	if (!GetOwner()) return;

	DefaultDisplayPlane = NewObject<UStaticMeshComponent>(GetOwner(),
		TEXT("MyNinjaLiveDisplayPlane"));
	if (DefaultDisplayPlane)
	{
		DefaultDisplayPlane->SetupAttachment(GetOwner()->GetRootComponent());
		DefaultDisplayPlane->RegisterComponent();
		DefaultDisplayPlane->SetRelativeScale3D(FVector(5.0f, 5.0f, 1.0f));

		// 使用引擎内置平面网格
		static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
			TEXT("/Engine/BasicShapes/Plane.Plane"));
		if (PlaneMesh.Succeeded())
			DefaultDisplayPlane->SetStaticMesh(PlaneMesh.Object);

		ExternalDisplayPlane = DefaultDisplayPlane;
	}
}

// ============================================================================
// 显示设置
// ============================================================================
void UMyNinjaLiveComponent::SetupDisplay()
{
	if (MID_Display && ExternalDisplayPlane)
	{
		ExternalDisplayPlane->SetMaterial(0, MID_Display);
	}
}

// ============================================================================
// 交互 Actor 获取
// ============================================================================
AActor* UMyNinjaLiveComponent::GetInteractionActor() const
{
	if (CustomInteractionActor)
		return CustomInteractionActor;
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

// ============================================================================
// 坐标转换
// ============================================================================
FVector2D UMyNinjaLiveComponent::WorldToSimUV(const FVector& WorldPos) const
{
	if (!GetOwner()) return FVector2D(0.5f, 0.5f);

	FVector Origin = GetOwner()->GetActorLocation();
	float Half = PlaneWorldSize * 0.5f;
	return FVector2D(
		(WorldPos.X - (Origin.X - Half)) / PlaneWorldSize,
		(WorldPos.Y - (Origin.Y - Half)) / PlaneWorldSize
	);
}

FVector2D UMyNinjaLiveComponent::EncodeVelocity(const FVector& WorldVelocity) const
{
	if (MaxVelocity <= 0.0f) return FVector2D(0.5f, 0.5f);
	return FVector2D(
		FMath::Clamp(WorldVelocity.X / MaxVelocity * 0.5f + 0.5f, 0.0f, 1.0f),
		FMath::Clamp(WorldVelocity.Y / MaxVelocity * 0.5f + 0.5f, 0.0f, 1.0f)
	);
}

// ============================================================================
// LineTrace
// ============================================================================
void UMyNinjaLiveComponent::GetTraceSource(FVector& OutStart, FVector& OutEnd) const
{
	// 优先从玩家相机发射
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);
		OutStart = CamLoc;
		OutEnd = CamLoc + CamRot.Vector() * TraceDistance;
	}
	else if (AActor* Interactor = GetInteractionActor())
	{
		OutStart = Interactor->GetActorLocation();
		OutEnd = OutStart + FVector(0, 0, -TraceDistance);
	}
	else
	{
		OutStart = FVector::ZeroVector;
		OutEnd = FVector::ZeroVector;
	}
}

FHitResult UMyNinjaLiveComponent::PerformLineTrace(
	const FVector& Start, const FVector& End) const
{
	FHitResult Hit;

	if (ObjectTypes.Num() > 0)
	{
		UKismetSystemLibrary::LineTraceSingleForObjects(
			GetWorld(),
			Start, End,
			ObjectTypes,
			false,
			TraceExcludeActors,
			bShowDebugMessages ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
			Hit,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			0.0f);
	}
	else
	{
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);
	}

	return Hit;
}

// ============================================================================
// 玩家距离检测
// ============================================================================
void UMyNinjaLiveComponent::CheckPawnProximity()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !GetOwner())
	{
		bPawnInsideBounds = false;
		return;
	}

	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(),
		GetOwner()->GetActorLocation());
	bPawnInsideBounds = Dist <= ActivationDistance;
}

// ============================================================================
// 仿真管线各步骤
// ============================================================================
void UMyNinjaLiveComponent::StepCollisionPainter(const FVector2D& UV,
	const FVector2D& VelocityEncoded, float DeltaTime, int32 TargetIndex)
{
	UMaterialInstanceDynamic* MID = MID_CollisionPainterDot;
	UTextureRenderTarget2D* RT = RT_Collision;

	if (!MID || !RT) return;

	MID->SetVectorParameterValue(TEXT("Position"),
		FLinearColor(UV.X, UV.Y, 0.0f, 1.0f));
	MID->SetVectorParameterValue(TEXT("Velocity"),
		FLinearColor(VelocityEncoded.X, VelocityEncoded.Y, 0.0f, 1.0f));
	MID->SetScalarParameterValue(TEXT("BrushSize"), BrushSize);
	MID->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
	MID->SetScalarParameterValue(TEXT("BrushHardness"), BrushHardness);
	MID->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT, MID);
}

void UMyNinjaLiveComponent::StepInjectDensity(const FVector2D& UV,
	const FVector2D& VelocityEncoded)
{
	if (!MID_DensityInject || !RT_DensityA) return;

	MID_DensityInject->SetVectorParameterValue(TEXT("Position"),
		FLinearColor(UV.X, UV.Y, 0.0f, 1.0f));
	MID_DensityInject->SetVectorParameterValue(TEXT("Velocity"),
		FLinearColor(VelocityEncoded.X, VelocityEncoded.Y, 0.0f, 1.0f));
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushSize"), BrushSize);
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushHardness"), BrushHardness);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_DensityA,
		MID_DensityInject);
}

void UMyNinjaLiveComponent::StepAdvection(UTextureRenderTarget2D* Src,
	UTextureRenderTarget2D* DstWrite, float DeltaTime)
{
	if (!MID_Advection || !Src || !DstWrite || !RT_VelocityA) return;

	// M_Advection expects the input as "Texture" (velocity+density combined)
	// We pass the source density RT; velocity is referenced internally
	MID_Advection->SetTextureParameterValue(TEXT("Texture"), Src);
	MID_Advection->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);
	MID_Advection->SetScalarParameterValue(TEXT("Dissipation"), Dissipation);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, DstWrite, MID_Advection);
}

void UMyNinjaLiveComponent::StepCompositeGradient(float DeltaTime)
{
	// 可选步骤：未设置 CompositeGradientMat 时跳过
	if (!MID_CompositeGradient)
		return;

	if (!RT_VelocityA || !RT_VelocityB || !RT_Collision)
		return;

	MID_CompositeGradient->SetTextureParameterValue(TEXT("VeloInputTexture"),
		RT_VelocityA);
	MID_CompositeGradient->SetTextureParameterValue(TEXT("VeloPainter"),
		RT_Collision);
	MID_CompositeGradient->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityB,
		MID_CompositeGradient);
}

void UMyNinjaLiveComponent::StepDivergence()
{
	if (!MID_Divergence || !RT_VelocityA || !RT_Divergence) return;

	MID_Divergence->SetTextureParameterValue(TEXT("Texture"), RT_VelocityA);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Divergence,
		MID_Divergence);
}

void UMyNinjaLiveComponent::StepPressureSolve()
{
	if (!MID_PressureSolver || !RT_Pressure || !RT_Divergence) return;

	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Pressure,
		FLinearColor::Black);

	for (int32 i = 0; i < PressureIterations; i++)
	{
		MID_PressureSolver->SetTextureParameterValue(
			TEXT("Texture"), RT_Pressure);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Pressure,
			MID_PressureSolver);
	}
}

void UMyNinjaLiveComponent::StepUpdateDisplay()
{
	// 可选步骤：未设置 DisplayMat 时跳过
	if (!MID_Display)
		return;

	UTextureRenderTarget2D* SourceRT = bShowDensity ? RT_DensityA : RT_VelocityA;

	// M_NinjaOutput_Basic 材质的纹理参数名为 VelocityDensityBuffer
	if (RT_External && SourceRT)
	{
		MID_Display->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), SourceRT);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_External,
			MID_Display);
	}
	else if (SourceRT)
	{
		MID_Display->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), SourceRT);
	}
}

// ============================================================================
// 全仿真管线
// ============================================================================
void UMyNinjaLiveComponent::RunSimulationPipeline(float DeltaTime)
{
	// 1. 获取交互源
	AActor* Interactor = GetInteractionActor();
	if (!Interactor) return;

	// 2. 计算速度
	FVector PlayerPos = Interactor->GetActorLocation();
	FVector PlayerVel = (PlayerPos - LastPlayerWorldPos) / DeltaTime;
	LastPlayerWorldPos = PlayerPos;

	// 3. 清空碰撞 RT
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Collision,
		FLinearColor::Black);

	// 4. 进行 LineTrace（从相机向前发射，用于指向交互）
	{
		FVector TraceStart, TraceEnd;
		GetTraceSource(TraceStart, TraceEnd);
		FHitResult Hit = PerformLineTrace(TraceStart, TraceEnd);
		if (Hit.bBlockingHit)
		{
			FVector2D HitUV = WorldToSimUV(Hit.Location);
			FVector2D VeloEncoded = EncodeVelocity(PlayerVel);
			StepCollisionPainter(HitUV, VeloEncoded, DeltaTime);
		}
	}

	// 5. ★ 始终将玩家位置映射到 UV 并注入密度（行走/站立产生波纹）
	FVector2D PlayerUV = WorldToSimUV(PlayerPos);
	FVector2D Velo = EncodeVelocity(PlayerVel);
	StepInjectDensity(PlayerUV, Velo);
	StepCollisionPainter(PlayerUV, Velo, DeltaTime);

	// 6. 平流 — 密度
	StepAdvection(RT_DensityA, RT_DensityB, DeltaTime);
	SwapRT(RT_DensityA, RT_DensityB);

	// 7. 平流 — 速度（自平流）
	StepAdvection(RT_VelocityA, RT_VelocityB, DeltaTime);
	SwapRT(RT_VelocityA, RT_VelocityB);

	// 8. 外力合成（可选 — 当 CompositeGradientMat 未设置时跳过）
	if (MID_CompositeGradient)
	{
		StepCompositeGradient(DeltaTime);
		SwapRT(RT_VelocityA, RT_VelocityB);
	}

	// 9. 散度计算
	StepDivergence();

	// 10. 压力求解（迭代）
	StepPressureSolve();

	// 11. 更新显示
	StepUpdateDisplay();
}
