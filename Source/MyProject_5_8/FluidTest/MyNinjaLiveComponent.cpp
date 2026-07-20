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
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] bDisableComponent=true, 退出"));
		SetComponentTickEnabled(false);
		return;
	}

	// 核心管线材质（必须全部设置才能运行仿真）
	if (!AdvectionMat || !DivergenceMat || !PressureSolverMat || !CollisionPainterDotMat)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[MyNinjaLiveComponent] 缺少核心管线材质引用！"));
		SetComponentTickEnabled(false);
		return;
	}

	if (!DisplayMat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MyNinjaLiveComponent] DisplayMat=NULL，仿真将运行但平面看不到效果！"));
	}

	if (!CompositeGradientMat)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MyNinjaLiveComponent] CompositeGradientMat=NULL，跳过外力合成。"));
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
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] RTs created"));

	// 创建动态材质实例
	if (!CreateDynamicMaterialInstances())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] CreateDynamicMaterialInstances FAILED"));
		SetComponentTickEnabled(false);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] MIDs created"));

	// 诊断代码已移除 — 显示管线确认工作正常
	// 问题已定位：碰撞绘制材质画全屏quad，黑色背景覆盖了RT内容
	// 解决方案：密度注入需用加法混合，只添加画刷不覆盖背景

	// 设置显示
	SetupDisplay();


	// 初始化多目标数组
	SimTargets.SetNum(MaxTargets);

	// 排除 Owner
	if (AActor* Owner = GetOwner())
		TraceExcludeActors.AddUnique(Owner);

	bInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] BeginPlay complete"));
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

		// Density injection uses additive blending: brush accumulates instead of replacing RT
		if (MID_DensityInject)
		{
			MID_DensityInject->BasePropertyOverrides.bOverride_BlendMode = true;
			MID_DensityInject->BasePropertyOverrides.BlendMode = BLEND_Additive;
		}
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
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] SetupDisplay — MID_Display=%s, ExternalDisplayPlane=%s, RT_DensityA=%s"),
		MID_Display ? TEXT("VALID") : TEXT("NULL"),
		ExternalDisplayPlane ? TEXT("VALID") : TEXT("NULL"),
		RT_DensityA ? TEXT("VALID") : TEXT("NULL"));

	if (MID_Display && ExternalDisplayPlane)
	{
		ExternalDisplayPlane->SetMaterial(0, MID_Display);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] SetupDisplay — FAILED: MID_Display or ExternalDisplayPlane is null"));
	}

	// ★ 关键：初始化 VelocityDensityBuffer 参数，指向 RT_VelocityA
	if (MID_Display && RT_VelocityA)
	{
		MID_Display->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), RT_VelocityA);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] SetupDisplay — FAILED: cannot set VelocityDensityBuffer"));
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
	if (!MID_DensityInject || !RT_VelocityA) return;

	MID_DensityInject->SetVectorParameterValue(TEXT("Position"),
		FLinearColor(UV.X, UV.Y, 0.0f, 1.0f));
	MID_DensityInject->SetVectorParameterValue(TEXT("Velocity"),
		FLinearColor(VelocityEncoded.X, VelocityEncoded.Y, 0.0f, 1.0f));
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushSize"), BrushSize);
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushHardness"), BrushHardness);

		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityA,
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
	if (!MID_Display)
	{
		return;
	}

		UTextureRenderTarget2D* SourceRT = bShowDensity ? RT_VelocityA : RT_VelocityA;  // density in B channel of velocity RT

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

	// 2. 清空碰撞 RT（每帧重新绘制交互数据）
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Collision,
		FLinearColor::Black);

	// 3. 玩家交互：追踪 + 密度/碰撞注入
	if (Interactor)
	{
		FVector PlayerPos = Interactor->GetActorLocation();
		FVector PlayerVel = (PlayerPos - LastPlayerWorldPos) / DeltaTime;
		LastPlayerWorldPos = PlayerPos;

		// 相机射线检测
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

		// ★ 密度注入到 RT_VelocityA（RG=速度, B=密度）
		// 加法混合 → 密度累积
		FVector2D PlayerUV = WorldToSimUV(PlayerPos);
		FVector2D Velo = EncodeVelocity(PlayerVel);
		StepInjectDensity(PlayerUV, Velo);
		StepCollisionPainter(PlayerUV, Velo, DeltaTime);
	}

	// 4. 外力合成 — CompositeGradient 从噪声+碰撞生成速度（产生自然流动！）
	if (MID_CompositeGradient)
	{
		StepCompositeGradient(DeltaTime);
		SwapRT(RT_VelocityA, RT_VelocityB);
	}

	// 5. 平流 — 同时平流速度(RG)和密度(B)
	// RT_VelocityA: RG=速度, B=密度 → M_Advection 通过 "Texture" 参数读取
	StepAdvection(RT_VelocityA, RT_VelocityB, DeltaTime);
	SwapRT(RT_VelocityA, RT_VelocityB);

	// 6. 散度计算
	StepDivergence();

	// 7. 压力求解（迭代）
	StepPressureSolve();

	// 8. 更新显示 — 读取 RT_VelocityA（B 通道=密度）
	StepUpdateDisplay();
}
