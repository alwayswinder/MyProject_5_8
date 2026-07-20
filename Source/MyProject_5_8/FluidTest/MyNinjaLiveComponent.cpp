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
		ObjectTypeQuery2,  // WorldDynamic
		ObjectTypeQuery3,  // Pawn
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

	// ★ 诊断：打印所有关键材质状态
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] === Material Status ==="));
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent]   DisplayMat       = %s"), DisplayMat ? *DisplayMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent]   AdvectionMat     = %s"), AdvectionMat ? *AdvectionMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent]   CompositeGradient= %s"), CompositeGradientMat ? *CompositeGradientMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent]   ExternalPlane    = %s"), ExternalDisplayPlane ? *ExternalDisplayPlane->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent]   bActivatedByPawn = %d"), bActivatedByPawnProximity);
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent]   Resolution       = %dx%d"), ResolutionX, ResolutionY);
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] ========================"));

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

	// 设置显示（存储到 MID_ActiveDisplay）
	SetupDisplay();

	// 加载预设
	LoadPreset();

	// 初始化多目标数组
	SimTargets.SetNum(MaxTargets);

	// 排除 Owner
	if (AActor* Owner = GetOwner())
		TraceExcludeActors.AddUnique(Owner);

	bInitialized = true;

	// 确保仿真初始帧运行（不受激活状态限制）
	bPawnInsideBounds = true;

	// ★ 诊断：输出 Actor 位置
	if (AActor* Owner = GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] BeginPlay — ActorLocation=%s, PlaneWorldSize=%.0f"),
			*Owner->GetActorLocation().ToString(), PlaneWorldSize);
	}

	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] BeginPlay complete — Resolution=%dx%d, PlaneWorldSize=%.0f"),
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

	// LOD 检测（每帧更新距离判断）
	CheckLOD();

	// LOD2：降低采样 FPS
	if (bLOD2_ReduceSamplingFPS && CurrentLODLevel >= 2)
	{
		// 每4帧执行一次
		static int32 Lod2Counter = 0;
		Lod2Counter++;
		if (Lod2Counter % 4 != 0) return;
	}

	// FPS 限制
	if (SimInterval > 0.0f)
	{
		SimTimer += DeltaTime;
		if (SimTimer < SimInterval) return;
		DeltaTime = SimTimer;
		SimTimer = 0.0f;
	}

	// 诊断：前10帧输出状态
	if (TickFrameCount < 10)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] Tick frame %d — bPawnInsideBounds=%d, ActiveTargets=%d, LOD=%d"),
			TickFrameCount, bPawnInsideBounds, ActiveActivationTargets.Num(), CurrentLODLevel);
	}

	// 激活检测
	if (bActivatedByPawnProximity)
	{
		ActiveActivationTargets.RemoveAll([](const TObjectPtr<AActor>& Ptr) { return Ptr == nullptr; });
		if (ActiveActivationTargets.Num() > 0)
		{
			bPawnInsideBounds = true;
		}
	}

	// ---- 主仿真管线 ----
	if (TickFrameCount < 5)
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] Starting simulation pipeline (frame %d)"), TickFrameCount);
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
	GlobalBrushScale = FMath::Clamp(NewSize, 0.001f, 100.0f);
	UserInputBrushScale = FMath::Max(0.0f, NewStrength);
	BrushVelocityNoiseFreq = FMath::Clamp(NewHardness, 0.0f, 1.0f);
}

void UMyNinjaLiveComponent::SetResolution(int32 NewX, int32 NewY)
{
	ResolutionX = FMath::Clamp(NewX, 64, 4096);
	ResolutionY = FMath::Clamp(NewY, 64, 4096);
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
	MID_PressureSolver        = PressureSolverMat
		? UMaterialInstanceDynamic::Create(PressureSolverMat, this)
		: nullptr;
	MID_PressureSolverIter    = PressureSolverIterMat
		? UMaterialInstanceDynamic::Create(PressureSolverIterMat, this)
		: MID_PressureSolver;
	MID_PressureCorrection    = PressureCorrectionMat
		? UMaterialInstanceDynamic::Create(PressureCorrectionMat, this)
		: nullptr;
	MID_Display               = DisplayMat
		? UMaterialInstanceDynamic::Create(DisplayMat, this)
		: nullptr;

	if (!MID_CollisionPainterDot || !MID_Advection ||
		!MID_Divergence || !MID_PressureSolver)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[MyNinjaLiveComponent] 核心动态材质实例创建失败"));
		return false;
	}

	// 诊断：打印可选的 MID 创建状态
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] MID_PressureCorrection=%s"),
		MID_PressureCorrection ? TEXT("created") : TEXT("NULL (will use CompositeGradient fallback)"));
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] MID_Display=%s"),
		MID_Display ? TEXT("created") : TEXT("NULL"));
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] MID_CompositeGradient=%s"),
		MID_CompositeGradient ? TEXT("created") : TEXT("NULL"));

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
// 显示设置 — 设置 Plane 材质并缓存到 MID_ActiveDisplay
// ============================================================================
void UMyNinjaLiveComponent::SetupDisplay()
{
	if (!ExternalDisplayPlane)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] SetupDisplay — ExternalDisplayPlane is null"));
		return;
	}

	// 选择输出材质：尝试用基础材质 M_NinjaOutput_Basic（绕过 MIC 的静态覆写）
	UMaterialInstance* BaseMat = DisplayMat;

	// 优先用 M_NinjaOutput_Basic 基础材质（不含静态覆写），确保纹理参数可控
	UMaterial* BaseOutputMat = LoadObject<UMaterial>(nullptr,
		TEXT("/Game/FluidNinjaLive/OutputMaterials/BaseMaterials/M_NinjaOutput_Basic.M_NinjaOutput_Basic"));
	if (BaseOutputMat)
	{
		MID_ActiveDisplay = UMaterialInstanceDynamic::Create(BaseOutputMat, this);
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] SetupDisplay — Using base material M_NinjaOutput_Basic"));
	}
	else if (BaseMat)
	{
		MID_ActiveDisplay = UMaterialInstanceDynamic::Create(BaseMat, this);
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] SetupDisplay — Fallback to '%s'"), *BaseMat->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] SetupDisplay — No display material available"));
		return;
	}

	ExternalDisplayPlane->SetMaterial(0, MID_ActiveDisplay);

	// 枚举所有纹理参数并绑定 RT_VelocityA
	TArray<FMaterialParameterInfo> TexParamInfos;
	TArray<FGuid> TexParamGuids;
	MID_ActiveDisplay->GetAllTextureParameterInfo(TexParamInfos, TexParamGuids);

	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] SetupDisplay — %d texture params found"), TexParamInfos.Num());

	for (const FMaterialParameterInfo& Info : TexParamInfos)
	{
		FName ParamName = Info.Name;
		UTextureRenderTarget2D* TargetRT = RT_VelocityA;
		if (ParamName == TEXT("VelocityDensityBuffer")) TargetRT = RT_VelocityA;
		else if (ParamName == TEXT("PaintBuffer")) TargetRT = RT_Collision;
		else if (ParamName == TEXT("PressureBuffer")) TargetRT = RT_Pressure;
		else if (ParamName == TEXT("DivergenceBuffer")) TargetRT = RT_Divergence;
		MID_ActiveDisplay->SetTextureParameterValue(ParamName, TargetRT);
		UE_LOG(LogTemp, Warning, TEXT("[SetupDisplay] %s = %s"), *ParamName.ToString(), *TargetRT->GetName());
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
	// 对齐原版: OffsetFromSimAreaMotion 在模拟区域运动时偏移 UV 计算
	FVector OffsetOrigin = Origin + FVector(OffsetFromSimAreaMotion, OffsetFromSimAreaMotion, 0.0f);
	float Half = PlaneWorldSize * 0.5f;
	return FVector2D(
		(WorldPos.X - (OffsetOrigin.X - Half)) / PlaneWorldSize,
		(WorldPos.Y - (OffsetOrigin.Y - Half)) / PlaneWorldSize
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
	MID->SetScalarParameterValue(TEXT("BrushSize"), GlobalBrushScale * 0.01f * UserInputBrushScale);
	MID->SetScalarParameterValue(TEXT("BrushStrength"), 1.0f);
	MID->SetScalarParameterValue(TEXT("BrushHardness"), 0.5f);
	MID->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);

	// 传递画笔噪声参数
	MID->SetScalarParameterValue(TEXT("BrushVeloNoise"), AdjustPainter_BrushVeloNoise);
	MID->SetScalarParameterValue(TEXT("VeloNoiseFreq"), BrushVelocityNoiseFreq);

	if (CollisionMask)
	{
		MID->SetTextureParameterValue(TEXT("CollisionMask"), CollisionMask);
	}

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
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushSize"), GlobalBrushScale * 0.01f * UserInputBrushScale);
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushStrength"), 1.0f);
	MID_DensityInject->SetScalarParameterValue(TEXT("BrushHardness"), 0.5f);
	MID_DensityInject->SetScalarParameterValue(TEXT("DeltaTime"), 0.016f);

	if (CollisionMask)
	{
		MID_DensityInject->SetTextureParameterValue(TEXT("CollisionMask"), CollisionMask);
	}

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityA,
		MID_DensityInject);
}

void UMyNinjaLiveComponent::StepAdvection(UTextureRenderTarget2D* Src,
	UTextureRenderTarget2D* DstWrite, float DeltaTime)
{
	if (!MID_Advection || !Src || !DstWrite || !RT_VelocityA) return;

	MID_Advection->SetTextureParameterValue(TEXT("Texture"), Src);
	MID_Advection->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);
	MID_Advection->SetScalarParameterValue(TEXT("Dissipation"), Dissipation);

	// CollisionMask 传递给平流材质（如果材质有此参数，否则静默忽略）
	if (CollisionMask)
	{
		MID_Advection->SetTextureParameterValue(TEXT("CollisionMask"), CollisionMask);
	}

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

	// 传递画笔噪声参数
	MID_CompositeGradient->SetScalarParameterValue(TEXT("BrushVeloNoise"),
		AdjustPainter_BrushVeloNoise);
	MID_CompositeGradient->SetScalarParameterValue(TEXT("VeloNoiseFreq"),
		BrushVelocityNoiseFreq);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityB,
		MID_CompositeGradient);
}

void UMyNinjaLiveComponent::StepPressureCorrection()
{
	// 目的：v = v - ∇p（减去压力梯度使速度场无散度）
	// 复用 MI_CompositeAndGradient，将 VeloPainter 设为 RT_Pressure（压力场）
	// 材质内部将压力场解释为梯度源，从速度场中减去
	if (!MID_CompositeGradient || !RT_VelocityA || !RT_VelocityB || !RT_Pressure)
		return;

	MID_CompositeGradient->SetTextureParameterValue(TEXT("VeloInputTexture"),
		RT_VelocityA);
	MID_CompositeGradient->SetTextureParameterValue(TEXT("VeloPainter"),
		RT_Pressure);
	MID_CompositeGradient->SetScalarParameterValue(TEXT("DeltaTime"), 1.0f);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityB,
		MID_CompositeGradient);
	SwapRT(RT_VelocityA, RT_VelocityB);
}

void UMyNinjaLiveComponent::StepCollisionClear()
{
	if (RT_Collision)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Collision, FLinearColor::Black);
	}
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

	// 对齐原版 BP: Step1 = MI_Pressure_Solver2_Step1 初始化
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Pressure, FLinearColor::Black);
	MID_PressureSolver->SetTextureParameterValue(TEXT("Divergence"), RT_Divergence);
	MID_PressureSolver->SetTextureParameterValue(TEXT("Texture"), RT_Pressure);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Pressure, MID_PressureSolver);

	// 对齐原版 BP: 迭代 = MI_Pressure_Solver1
	UMaterialInstanceDynamic* IterSolver = MID_PressureSolverIter ? MID_PressureSolverIter : MID_PressureSolver;
	for (int32 i = 0; i < PressureIterations; i++)
	{
		IterSolver->SetTextureParameterValue(TEXT("Texture"), RT_Pressure);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Pressure, IterSolver);
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("[MyNinjaLiveComponent] PressureSolve — Step1 + %d iterations"), PressureIterations);
}

void UMyNinjaLiveComponent::StepUpdateDisplay()
{
	if (!MID_ActiveDisplay) return;

	UTextureRenderTarget2D* SourceRT = RT_VelocityA;
	if (!SourceRT) return;

	// 更新 VelocityDensityBuffer 到最新的 SourceRT（速度+密度数据）
	MID_ActiveDisplay->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), SourceRT);
	// 更新 PaintBuffer 到 RT_Collision（碰撞力数据）
	if (RT_Collision)
		MID_ActiveDisplay->SetTextureParameterValue(TEXT("PaintBuffer"), RT_Collision);

	if (RT_External && MID_Display)
	{
		MID_Display->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), SourceRT);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_External, MID_Display);
	}
}

// ============================================================================
// 全仿真管线
// ============================================================================
void UMyNinjaLiveComponent::RunSimulationPipeline(float DeltaTime)
{
	TickFrameCount++;
	AccumulatedTime += DeltaTime;

	// ===== M-fM-^PM-bM-^NM-^QM-eM-$M-^KM-uM-^M-^Y: M-dM-^AM-^MM-eM-^OM-^AM-^AM-^IM-eM-^M-^MM-^G M-eM-^AM-^Q Canvas M-gM-^[M-^AM-^MM-^G M-eM-^OM-^AM-^MM-^G M-fM-^AM-^MM-^G RT M-mM-8M-^AM-^C Canvas ^I^TM-^IM-gM-^PM-^OM-^M-^E M-fM-^LM-^OM-^MM-^A M-^TM-^AM-^MM-^HM-^M-^H)

	if (TickFrameCount <= 3)
	{

		UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr,

	TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));

		UCanvas* Canvas = nullptr;

		FVector2D CanvasSize = FVector2D((float)ResolutionX, (float)ResolutionY);

		FDrawToRenderTargetContext Context;

		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
			this, RT_VelocityA, Canvas, CanvasSize, Context);
		if (Canvas && WhiteTex)
		{
			for (int32 i = 0; i < 5; i++)
			{
				float Phase = (float)i * 1.256f + AccumulatedTime * 0.5f;
				float NormX = 0.5f + FMath::Sin(Phase * 0.7f) * 0.35f;
				float NormY = 0.5f + FMath::Cos(Phase * 0.5f) * 0.35f;
				float Size = CanvasSize.X * 0.04f;
				FVector2D Pos(NormX * CanvasSize.X - Size * 0.5f,
							  NormY * CanvasSize.Y - Size * 0.5f);
				Canvas->K2_DrawTexture(WhiteTex, Pos, FVector2D(Size, Size),
					FVector2D(0, 0), FVector2D(1, 1), FLinearColor::White,
					BLEND_Additive);
			}
		}
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);

		StepUpdateDisplay();

		if (TickFrameCount == 3)

			UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] Seed frames complete"));

		return;
	}
	// ===== 1. 清除碰撞 RT =====
	StepCollisionClear();
		// ===== 1.5 M-eM-^AM-^QM-gM-^KM-^MM-^AM-^M M-mM-^MM-^KM-^FM-^MM-^A: Canvas M-mM-^AM-^MM-^IM-^BM-^AM-^M M-eM-^EM-^SM-^MM-^AM-^M M-cM-^TM-^EM-^IM-^LM-^HM-^EM-^M-^M M-eM-^LM-^M-^MM-^A M-mM-^IM-^LM-^AM-^M M-eM-^MM-^TM-^AM-^M M-vM-^AM-^MM-=

		UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr,

			TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));

		UCanvas* Canvas = nullptr;

		FVector2D CanvasSize = FVector2D((float)ResolutionX, (float)ResolutionY);

		FDrawToRenderTargetContext Context;

		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(

			this, RT_VelocityA, Canvas, CanvasSize, Context);

		if (Canvas && WhiteTex)

		{

			for (int32 i = 0; i < 8; i++)

			{

				float Phase = (float)i * 0.785f + AccumulatedTime * 0.5f;

				float NormX = 0.5f + FMath::Sin(Phase * 0.7f) * 0.4f;

				float NormY = 0.5f + FMath::Cos(Phase * 0.5f) * 0.4f;

				float Size = CanvasSize.X * 0.03f;

				FVector2D Pos(NormX * CanvasSize.X - Size * 0.5f,

							  NormY * CanvasSize.Y - Size * 0.5f);

				Canvas->K2_DrawTexture(WhiteTex, Pos, FVector2D(Size, Size),

					FVector2D(0, 0), FVector2D(1, 1), FLinearColor::White,

					BLEND_Additive);

			}

		}

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
		StepUpdateDisplay();
		return;
	}
// 相机 LineTrace 交互
// ============================================================================
void UMyNinjaLiveComponent::HandleCameraLineTrace(float DeltaTime)
{
	if (UseCustomTraceSource) return;
	FVector TraceStart, TraceEnd;
	GetTraceSource(TraceStart, TraceEnd);
	if (TraceStart.IsZero() && TraceEnd.IsZero())
	{
		bCameraTraceHit = false;
		return;
	}
	FHitResult Hit = PerformLineTrace(TraceStart, TraceEnd);
	bCameraTraceHit = Hit.bBlockingHit;
	if (bCameraTraceHit)
	{
		CameraTraceHitUV = WorldToSimUV(Hit.Location);
		CameraTraceHitVelocity = FVector::ZeroVector;
		if (AActor* HitActor = Hit.GetActor())
		{
			CameraTraceHitVelocity = HitActor->GetVelocity();
		}
		FVector2D VelEnc = EncodeVelocity(CameraTraceHitVelocity);
		StepCollisionPainter(CameraTraceHitUV, VelEnc, DeltaTime);
		AddInteractionPoint(CameraTraceHitUV, CameraTraceHitVelocity);
	}
}
// ============================================================================
// LOD 检测
// ============================================================================
void UMyNinjaLiveComponent::CheckLOD()
{
	if (!bActivatedByPawnProximity)
	{
		CurrentLODLevel = 0;
		return;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !GetOwner())
	{
		CurrentLODLevel = 2;
		return;
	}
	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
	if (Dist < LOD_NearBound)
		CurrentLODLevel = 0;
	else if (Dist < LOD_FarBound)
		CurrentLODLevel = 1;
	else
		CurrentLODLevel = 2;
}
// ============================================================================
// 预设加载
// ============================================================================
void UMyNinjaLiveComponent::LoadPreset()
{
	if (!DefaultPreset)
	{
		UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] No DefaultPreset assigned, skipping preset load"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] Preset DataTable assigned: %s (criteria: %s)"),
		*DefaultPreset->GetName(), *PresetNameFilterCriteria);
	bPresetLoaded = true;
}
// ============================================================================
// 激活检测
// ============================================================================
bool UMyNinjaLiveComponent::ShouldSimulationRun() const
{
	if (!bInitialized || bDisableComponent)
		return false;
	if (!bActivatedByPawnProximity)
		return true;
	if (ActiveActivationTargets.Num() > 0)
		return true;
	return bPawnInsideBounds;
}
// ============================================================================
// 重叠交互 — 对齐原版 OverlapFilter 逻辑
// ============================================================================
void UMyNinjaLiveComponent::AddInteractionPoint(const FVector2D& UV, const FVector& WorldVelocity)
{
	if (!bInitialized || bDisableComponent) return;
	// 对齐原版: 低速阻尼过滤
	float Speed = WorldVelocity.Size();
	if (Speed < DampenBrushBelowThisVelocity) return;
	// 对齐原版: GlobalBrushScale + UserInputBrushScale → 实际画笔尺寸
	float EffectiveBrushSize = GlobalBrushScale * 0.01f * UserInputBrushScale;
	FVector2D VelEnc = EncodeVelocity(WorldVelocity);
	// ---- 密度注入 (对齐原版 Painter → Density inject) ----
	if (MID_DensityInject && RT_VelocityA)
	{
		MID_DensityInject->SetVectorParameterValue(TEXT("Position"), FLinearColor(UV.X, UV.Y, 0, 1));
		MID_DensityInject->SetVectorParameterValue(TEXT("Velocity"), FLinearColor(VelEnc.X, VelEnc.Y, 0, 1));
		MID_DensityInject->SetScalarParameterValue(TEXT("BrushSize"), EffectiveBrushSize);
		MID_DensityInject->SetScalarParameterValue(TEXT("BrushStrength"), 1.0f);
		MID_DensityInject->SetScalarParameterValue(TEXT("BrushHardness"), 0.5f);
		MID_DensityInject->SetScalarParameterValue(TEXT("DeltaTime"), 0.016f);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_VelocityA, MID_DensityInject);
	}
	// ---- 碰撞绘制 (供 CompositeGradient 读取) ----
	if (MID_CollisionPainterDot && RT_Collision)
	{
		MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Position"), FLinearColor(UV.X, UV.Y, 0, 1));
		MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Velocity"), FLinearColor(VelEnc.X, VelEnc.Y, 0, 1));
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushSize"), EffectiveBrushSize);
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushStrength"), 1.0f);
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushHardness"), 0.5f);
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("DeltaTime"), 0.016f);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Collision, MID_CollisionPainterDot);
	}
}
void UMyNinjaLiveComponent::AddInteractionTarget(AActor* Target)
{
	if (Target)
	{
		ActiveInteractionTargets.AddUnique(Target);
	}
}
void UMyNinjaLiveComponent::RemoveInteractionTarget(AActor* Target)
{
	ActiveInteractionTargets.Remove(Target);
}
void UMyNinjaLiveComponent::AddActiveTarget(AActor* Target)
{
	ActiveActivationTargets.AddUnique(Target);
	bPawnInsideBounds = true;
}
void UMyNinjaLiveComponent::RemoveActiveTarget(AActor* Target)
{
	ActiveActivationTargets.Remove(Target);
	bPawnInsideBounds = ActiveActivationTargets.Num() > 0;
}
