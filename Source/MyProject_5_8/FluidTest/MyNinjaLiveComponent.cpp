// MyNinjaLiveComponent.cpp
// C++ 重现 FluidNinjaLive 核心流体仿真管线 — 对齐 BP 参考

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
#include "Materials/MaterialInstance.h"
#include "Engine/DataTable.h"
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
		ObjectTypeQuery2,  // WorldDynamic
		ObjectTypeQuery3,  // Pawn
		ObjectTypeQuery4,  // PhysicsBody
	};

	// 默认预设 DataTable (对齐 BP DT_NinjaLive_Default)
	static ConstructorHelpers::FObjectFinder<UDataTable> PresetDT(
		TEXT("/Game/FluidNinjaLive/Presets/DT_NinjaLive_Default.DT_NinjaLive_Default"));
	if (PresetDT.Succeeded()) DefaultPreset = PresetDT.Object;
}

// ============================================================================
// BeginPlay
// ============================================================================
void UMyNinjaLiveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bDisableComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MyNinjaLiveComponent] Disabled, exiting"));
		SetComponentTickEnabled(false);
		return;
	}

	// 核心管线材质检查
	if (!AdvectionMat || !DivergenceMat || !PressureSolverInitMat || !CollisionPainterDotMat)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] Missing core materials!"));
		SetComponentTickEnabled(false);
		return;
	}

	// 日志输出关键材质状态
	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] === Material Status ==="));
	UE_LOG(LogTemp, Log, TEXT("  AdvectionMat     = %s"), AdvectionMat ? *AdvectionMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Log, TEXT("  CompositeGradient= %s"), CompositeGradientMat ? *CompositeGradientMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Log, TEXT("  DivergenceMat    = %s"), DivergenceMat ? *DivergenceMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Log, TEXT("  DisplayMat       = %s"), DisplayMat ? *DisplayMat->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Log, TEXT("  Resolution       = %dx%d"), ResolutionX, ResolutionY);

	// 创建显示平面
	if (!ExternalDisplayPlane && bCreateDefaultDisplayPlane)
		CreateDefaultPlane();

	// 创建 RT
	CreateAllRTs();

	// 创建动态材质实例
	if (!CreateDynamicMaterialInstances())
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] MID creation failed"));
		SetComponentTickEnabled(false);
		return;
	}

	// 设置显示
	SetupDisplay();

	// 加载预设
	LoadPreset();

	// 初始化多目标
	SimTargets.SetNum(MaxTargets);

	bInitDone = true;
	bPawnInsideBounds = true;  // 确保首帧运行

	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] BeginPlay complete — %dx%d"),
		ResolutionX, ResolutionY);
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
// TickComponent
// ============================================================================
void UMyNinjaLiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInitDone || bDisableComponent || DeltaTime <= 0.0f)
		return;

	AccumulatedTime += DeltaTime;

	// FPS 限制 (对齐 BP TickRateCustom)
	if (TickRateCustom > 0.0f)
	{
		SimTimer += DeltaTime;
		if (SimTimer < TickRateCustom) return;
		DeltaTime = SimTimer;
		SimTimer = 0.0f;
	}

	// LOD 检测
	CheckLOD(DeltaTime);

	// LOD2: 降低采样 FPS
	if (bLOD2_ReduceSamplingFPS && CurrentLODLevel >= 2)
	{
		static int32 Lod2Skip = 0;
		Lod2Skip++;
		if (Lod2Skip % 4 != 0) return;
	}

	// 激活检测
	if (bComponentActivatedByPawnProximity)
	{
		ActiveActivationTargets.RemoveAll([](const TObjectPtr<AActor>& Ptr) { return Ptr == nullptr; });
		bPawnInsideBounds = (ActiveActivationTargets.Num() > 0);
		if (!bPawnInsideBounds) return;
	}

	// 计算玩家 UV (用作相机交互)
	{
		APawn* P = UGameplayStatics::GetPlayerPawn(this, 0);
		if (P)
		{
			FVector2D UV = WorldToSimUV(P->GetActorLocation());
			UV.X = FMath::Clamp(UV.X, 0.0f, 1.0f);
			UV.Y = FMath::Clamp(UV.Y, 0.0f, 1.0f);
			CameraTraceHitUV = UV;
			CameraTraceHitVelocity = P->GetVelocity();
			bCameraTraceHit = (UV.X > 0.001f && UV.X < 0.999f && UV.Y > 0.001f && UV.Y < 0.999f);
		}
		else
		{
			bCameraTraceHit = false;
		}
	}

	// 运行仿真管线
	RunSimulationPipeline(DeltaTime);

	TickFrameCount++;
}

// ============================================================================
// 公开接口
// ============================================================================
void UMyNinjaLiveComponent::ForceEnable()
{
	bDisableComponent = false;
	SetComponentTickEnabled(true);
}

void UMyNinjaLiveComponent::ForceDisable()
{
	bDisableComponent = true;
	SetComponentTickEnabled(false);
}

void UMyNinjaLiveComponent::ResetSimulation()
{
	ClearAllRTs();
}

void UMyNinjaLiveComponent::SetBrush(float NewSize, float NewStrength, float NewHardness)
{
	BrushSize = FMath::Clamp(NewSize, 0.001f, 100.0f);
	BrushStrength = FMath::Max(0.0f, NewStrength);
	BrushHardness = FMath::Clamp(NewHardness, 0.0f, 1.0f);
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
	// 对齐 BP 7 个命名 RT + 1 个工作缓冲
	RT_Composite          = CreateRT(TEXT("RT_Composite"),          ResolutionX, ResolutionY);
	RT_Advection          = CreateRT(TEXT("RT_Advection"),          ResolutionX, ResolutionY);
	RT_Painter            = CreateRT(TEXT("RT_Painter"),            ResolutionX, ResolutionY);
	RT_PressureDivergence = CreateRT(TEXT("RT_PressureDivergence"), ResolutionX, ResolutionY);
	RT_PressureDivergenceTemp = CreateRT(TEXT("RT_PressureDivergenceTemp"), ResolutionX, ResolutionY);
	RT_DensityInput       = CreateRT(TEXT("RT_DensityInput"),       ResolutionX, ResolutionY);
	RT_Output             = CreateRT(TEXT("RT_Output"),             ResolutionX, ResolutionY);
	RT_WorkBuffer         = CreateRT(TEXT("RT_WorkBuffer"),         ResolutionX, ResolutionY);

	ClearAllRTs();
}

void UMyNinjaLiveComponent::ClearAllRTs()
{
	auto Clear = [this](UTextureRenderTarget2D* RT) {
		if (RT) UKismetRenderingLibrary::ClearRenderTarget2D(this, RT, FLinearColor::Black);
	};
	Clear(RT_Composite);
	Clear(RT_Advection);
	Clear(RT_Painter);
	Clear(RT_PressureDivergence);
	Clear(RT_PressureDivergenceTemp);
	Clear(RT_DensityInput);
	Clear(RT_Output);
	Clear(RT_WorkBuffer);
}

// ============================================================================
// 动态材质实例创建
// ============================================================================
bool UMyNinjaLiveComponent::CreateDynamicMaterialInstances()
{
	// 创建所有 MID
	MID_CollisionPainterDot  = UMaterialInstanceDynamic::Create(CollisionPainterDotMat,    this);
	MID_CollisionPainterLine = CollisionPainterLineMat
		? UMaterialInstanceDynamic::Create(CollisionPainterLineMat, this) : nullptr;
	MID_CollisionPainterOffset = CollisionPainterOffsetMat
		? UMaterialInstanceDynamic::Create(CollisionPainterOffsetMat, this) : nullptr;
	MID_CompositeGradient    = CompositeGradientMat
		? UMaterialInstanceDynamic::Create(CompositeGradientMat, this) : nullptr;
	MID_Advection            = UMaterialInstanceDynamic::Create(AdvectionMat,               this);
	MID_Divergence           = UMaterialInstanceDynamic::Create(DivergenceMat,              this);
	MID_PressureSolverInit   = PressureSolverInitMat
		? UMaterialInstanceDynamic::Create(PressureSolverInitMat, this) : nullptr;
	MID_PressureSolverIter   = PressureSolverIterMat
		? UMaterialInstanceDynamic::Create(PressureSolverIterMat, this) : nullptr;
	MID_PressureCorrection   = PressureCorrectionMat
		? UMaterialInstanceDynamic::Create(PressureCorrectionMat, this) : nullptr;
	MID_Display              = DisplayMat
		? UMaterialInstanceDynamic::Create(DisplayMat, this) : nullptr;

	// 核心材质必须存在
	if (!MID_CollisionPainterDot || !MID_Advection || !MID_Divergence || !MID_PressureSolverInit)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] Core MID creation failed"));
		return false;
	}

	// 如果 PressureSolverIter 未设置，复用 Init
	if (!MID_PressureSolverIter)
		MID_PressureSolverIter = MID_PressureSolverInit;

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
	if (!ExternalDisplayPlane)
	{
		UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] SetupDisplay — no display plane"));
		return;
	}

	// 优先使用 DisplayMat，否则 fallback 到 M_NinjaOutput_Basic
	if (DisplayMat)
	{
		MID_ActiveDisplay = UMaterialInstanceDynamic::Create(DisplayMat, this);
	}
	else
	{
		UMaterial* BaseOutput = LoadObject<UMaterial>(nullptr,
			TEXT("/Game/FluidNinjaLive/OutputMaterials/BaseMaterials/M_NinjaOutput_Basic.M_NinjaOutput_Basic"));
		if (BaseOutput)
			MID_ActiveDisplay = UMaterialInstanceDynamic::Create(BaseOutput, this);
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[MyNinjaLiveComponent] No display material available"));
			return;
		}
	}

	ExternalDisplayPlane->SetMaterial(0, MID_ActiveDisplay);

	// 绑定纹理参数
	TArray<FMaterialParameterInfo> TexParamInfos;
	TArray<FGuid> TexParamGuids;
	MID_ActiveDisplay->GetAllTextureParameterInfo(TexParamInfos, TexParamGuids);

	for (const FMaterialParameterInfo& Info : TexParamInfos)
	{
		FName ParamName = Info.Name;
		UTexture* Target = nullptr;
		if (ParamName == TEXT("VelocityDensityBuffer") || ParamName == TEXT("Texture"))
			Target = RT_Output;
		else if (ParamName == TEXT("PaintBuffer"))
			Target = RT_Painter;
		else if (ParamName == TEXT("PressureBuffer"))
			Target = RT_PressureDivergence;
		else if (ParamName == TEXT("DivergenceBuffer"))
			Target = RT_PressureDivergence;

		if (Target)
			MID_ActiveDisplay->SetTextureParameterValue(ParamName, Target);
	}
}

// ============================================================================
// 坐标转换
// ============================================================================
AActor* UMyNinjaLiveComponent::GetInteractionActor() const
{
	if (CustomInteractionActor)
		return CustomInteractionActor;
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

FVector2D UMyNinjaLiveComponent::WorldToSimUV(const FVector& WorldPos) const
{
	if (!GetOwner()) return FVector2D(0.5f, 0.5f);
	FVector Origin = GetOwner()->GetActorLocation();
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
	if (UseCustomTraceSource)
	{
		if (GetOwner())
		{
			FVector OwnerLoc = GetOwner()->GetActorLocation();
			OutStart = OwnerLoc + CustomTraceSourcePosition;
			OutEnd = OutStart + FVector(0, 0, -TraceDistance);
		}
		return;
	}

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
}

FHitResult UMyNinjaLiveComponent::PerformLineTrace(
	const FVector& Start, const FVector& End) const
{
	FHitResult Hit;

	if (ObjectTypes.Num() > 0)
	{
		UKismetSystemLibrary::LineTraceSingleForObjects(
			GetWorld(), Start, End, ObjectTypes, false,
			TraceExcludeActors,
			bShowDebugMessages ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
			Hit, false, FLinearColor::Red, FLinearColor::Green, 0.0f);
	}
	else
	{
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);
	}

	return Hit;
}

// ============================================================================
// 玩家距离 & LOD
// ============================================================================
void UMyNinjaLiveComponent::CheckPawnProximity()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !GetOwner())
	{
		bPawnInsideBounds = false;
		return;
	}
	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());
	bPawnInsideBounds = Dist <= LOD_FarBound * 2.0f;  // 宽松检测
}

void UMyNinjaLiveComponent::CheckLOD(float DeltaTime)
{
	LODCheckTimer += DeltaTime;
	if (LODCheckTimer < LODCheckFrequency) return;
	LODCheckTimer = 0.0f;

	if (!bComponentActivatedByPawnProximity)
	{
		CurrentLODLevel = 0;
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || !GetOwner())
	{
		CurrentLODLevel = LOD_Steps;
		return;
	}

	float Dist = FVector::Dist(PlayerPawn->GetActorLocation(), GetOwner()->GetActorLocation());

	if (Dist < LOD_NearBound) CurrentLODLevel = 0;
	else if (Dist < LOD_FarBound) CurrentLODLevel = 1;
	else CurrentLODLevel = FMath::Min(LOD_Steps, 2 + FMath::FloorToInt((Dist - LOD_FarBound) / LOD_StepRange));
}

// ============================================================================
// 预设加载
// ============================================================================
void UMyNinjaLiveComponent::LoadPreset()
{
	if (!DefaultPreset || !bForceAutoLoadPreset)
	{
		UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] Preset: not loaded (DT=%s, Force=%d)"),
			DefaultPreset ? *DefaultPreset->GetName() : TEXT("NULL"), bForceAutoLoadPreset);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MyNinjaLiveComponent] Loading preset from %s (filter: %s)"),
		*DefaultPreset->GetName(), *PresetNameFilterCriteria.ToString());
	bPresetLoaded = true;
}

// ============================================================================
// 仿真管线步骤
// ============================================================================

// --------------------------------------------------------------------------
// Step 0: 清除碰撞 RT
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepCollisionClear()
{
	if (RT_Painter)
		UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_Painter, FLinearColor::Black);
}

// --------------------------------------------------------------------------
// Step 1: 碰撞绘制 — 将交互数据画到 RT_Painter
//   材质参数：Position, Velocity, BrushSize, BrushStrength, etc.
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepCollisionPainter(const FVector2D& UV,
	const FVector2D& VelocityEncoded, float DeltaTime, int32 TargetIndex)
{
	if (!MID_CollisionPainterDot || !RT_Painter) return;

	const float EffectiveBrushSize = BrushSize * GlobalBrushScale * UserInputBrushScale;

	MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Position"),
		FLinearColor(UV.X, UV.Y, 0.0f, 1.0f));
	MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Velocity"),
		FLinearColor(VelocityEncoded.X, VelocityEncoded.Y, 0.0f, 1.0f));
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushSize"), EffectiveBrushSize);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushHardness"), BrushHardness);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushVeloNoise"), AdjustPainter_V2_BrushVeloNoise);
	MID_CollisionPainterDot->SetScalarParameterValue(TEXT("VeloNoiseFreq"), BrushVelocityNoiseFreq);

	if (CollisionMask)
		MID_CollisionPainterDot->SetTextureParameterValue(TEXT("CollisionMask"), CollisionMask);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Painter, MID_CollisionPainterDot);
}

// --------------------------------------------------------------------------
// Step 2: 外力合成 — 将碰撞力合并到速度场
//   CompositeGradient: RT_Composite(vel+density) + RT_Painter(forces)
//   → RT_WorkBuffer (new vel+density with forces applied)
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepCompositeGradient(float DeltaTime)
{
	if (!MID_CompositeGradient || !RT_Composite || !RT_Painter || !RT_WorkBuffer) return;

	MID_CompositeGradient->SetTextureParameterValue(TEXT("VeloInputTexture"), RT_Composite);
	MID_CompositeGradient->SetTextureParameterValue(TEXT("VeloPainter"), RT_Painter);
	MID_CompositeGradient->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);
	MID_CompositeGradient->SetScalarParameterValue(TEXT("BrushVeloNoise"), AdjustPainter_V2_BrushVeloNoise);
	MID_CompositeGradient->SetScalarParameterValue(TEXT("VeloNoiseFreq"), BrushVelocityNoiseFreq);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_WorkBuffer, MID_CompositeGradient);
}

// --------------------------------------------------------------------------
// Step 3: 平流 — 沿速度场平流密度+速度
//   Advection: RT_Composite → RT_Advection (advected)
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepAdvection(float DeltaTime)
{
	if (!MID_Advection || !RT_Composite || !RT_Advection) return;

	MID_Advection->SetTextureParameterValue(TEXT("Texture"), RT_Composite);
	MID_Advection->SetScalarParameterValue(TEXT("DeltaTime"), DeltaTime);
	MID_Advection->SetScalarParameterValue(TEXT("Dissipation"), Dissipation);

	if (CollisionMask)
		MID_Advection->SetTextureParameterValue(TEXT("CollisionMask"), CollisionMask);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Advection, MID_Advection);
}

// --------------------------------------------------------------------------
// Step 4: 散度 — 计算速度场的散度
//   Divergence: RT_Advection → RT_PressureDivergence
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepDivergence()
{
	if (!MID_Divergence || !RT_Composite || !RT_PressureDivergence) return;

	MID_Divergence->SetTextureParameterValue(TEXT("Texture"), RT_Composite);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_PressureDivergence, MID_Divergence);
}

// --------------------------------------------------------------------------
// Step 5: 压力求解 — 解泊松方程 ∇²p = ∇·v
//   Solver2 (默认, 1次迭代+kernel reduction):
//     Clear → PressureSolverInit(divergence→pressure)
//     → PressureSolverIter × PressureSolver2_MaxIterations
//   Solver1 (可选, N次 Jacobi 迭代):
//     Clear → PressureSolverInit → PressureSolverIter × N
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepPressureSolve()
{
	if (!MID_PressureSolverInit || !RT_PressureDivergence || !RT_PressureDivergenceTemp) return;

	// 清空临时缓冲
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT_PressureDivergenceTemp, FLinearColor::Black);

	// Step1: 初始化解 (MI_Pressure_Solver2_Step1)
	MID_PressureSolverInit->SetTextureParameterValue(TEXT("Divergence"), RT_PressureDivergence);
	MID_PressureSolverInit->SetTextureParameterValue(TEXT("Texture"), RT_PressureDivergenceTemp);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_PressureDivergenceTemp, MID_PressureSolverInit);

	// 迭代求解
	int32 IterCount = bUsePressureSolver1
		? PressureSolver1_MaxIterations
		: PressureSolver2_MaxIterations;

	for (int32 i = 0; i < IterCount; i++)
	{
		MID_PressureSolverIter->SetTextureParameterValue(TEXT("Texture"), RT_PressureDivergenceTemp);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_PressureDivergenceTemp, MID_PressureSolverIter);
	}

	// 复制结果到 RT_PressureDivergence (供 PressureCorrection 读取)
	CopyRT(RT_PressureDivergenceTemp, RT_PressureDivergence);
}

// --------------------------------------------------------------------------
// Step 6: 压力梯度修正 — v' = v - ∇p
//   从速度场减去压力梯度，使速度场无散度
//   PressureCorrection / CompositeGradient:
//     RT_Advection(vel) + RT_PressureDivergence(pressure) → RT_WorkBuffer
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepPressureCorrection()
{
	if (!RT_Composite || !RT_PressureDivergence || !RT_WorkBuffer) return;

	// 使用 PressureCorrectionMat 或 fallback 到 CompositeGradient
	UMaterialInstanceDynamic* MID_Correction = MID_PressureCorrection ? MID_PressureCorrection : MID_CompositeGradient;
	if (!MID_Correction) return;

	// PressureCorrection 材质也使用 VeloInputTexture + VeloPainter 参数名
	MID_Correction->SetTextureParameterValue(TEXT("VeloInputTexture"), RT_Composite);
	MID_Correction->SetTextureParameterValue(TEXT("VeloPainter"), RT_PressureDivergence);
	MID_Correction->SetScalarParameterValue(TEXT("DeltaTime"), 1.0f);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_WorkBuffer, MID_Correction);
}

// --------------------------------------------------------------------------
// Step 7: 密度注入 (Canvas) — 在交互位置注入密度
//   直接在 RT_Composite 上画白色方块作为密度源
//   通道布局: R=VelX, G=VelY, B=Density(1.0), A=alpha
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepInjectDensityCanvas(const FVector2D& UV,
	const FVector2D& VelocityEncoded, float DensityAmount)
{
	if (!RT_Composite) return;

	UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr,
		TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));
	UCanvas* Canvas = nullptr;
	FVector2D CanvasSize = FVector2D((float)ResolutionX, (float)ResolutionY);
	FDrawToRenderTargetContext Context;

	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
		this, RT_Composite, Canvas, CanvasSize, Context);

	if (Canvas && WhiteTex)
	{
		const float BaseSize = CanvasSize.X * 0.08f * GlobalBrushScale * UserInputBrushScale;
		// 多层绘制实现软边界
		for (int32 r = 0; r < 6; r++)
		{
			float t = (float)r / 5.0f;
			float alpha = (1.0f - t * t) * DensityAmount;
			float size = BaseSize * (1.0f + t * 1.5f);
			FVector2D Pos(UV.X * CanvasSize.X - size * 0.5f,
						  UV.Y * CanvasSize.Y - size * 0.5f);
			Canvas->K2_DrawTexture(WhiteTex, Pos, FVector2D(size, size),
				FVector2D(0, 0), FVector2D(1, 1),
				FLinearColor(VelocityEncoded.X, VelocityEncoded.Y, 1.0f, alpha),
				BLEND_Additive);
		}
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
}

// --------------------------------------------------------------------------
// Step 8: 更新显示 — 将最终结果写入 RT_Output 并更新材质参数
// --------------------------------------------------------------------------
void UMyNinjaLiveComponent::StepUpdateDisplay()
{
	// 将最终结果复制到 RT_Output
	if (MID_Display && RT_Composite && RT_Output)
	{
		MID_Display->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), RT_Composite);
		if (RT_Painter)
			MID_Display->SetTextureParameterValue(TEXT("PaintBuffer"), RT_Painter);
		if (RT_PressureDivergence)
			MID_Display->SetTextureParameterValue(TEXT("PressureBuffer"), RT_PressureDivergence);

		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Output, MID_Display);
	}

	// 更新 DisplayPlane 材质
	if (MID_ActiveDisplay)
	{
		MID_ActiveDisplay->SetTextureParameterValue(TEXT("VelocityDensityBuffer"), RT_Output);
		if (RT_Painter)
			MID_ActiveDisplay->SetTextureParameterValue(TEXT("PaintBuffer"), RT_Painter);
	}
}

// ============================================================================
// RT 复制 (通过 Advection MID passthrough)
// ============================================================================
void UMyNinjaLiveComponent::CopyRT(UTextureRenderTarget2D* Src, UTextureRenderTarget2D* Dst)
{
	if (!Src || !Dst || !MID_Advection) return;

	MID_Advection->SetTextureParameterValue(TEXT("Texture"), Src);
	MID_Advection->SetScalarParameterValue(TEXT("DeltaTime"), 0.0f);
	MID_Advection->SetScalarParameterValue(TEXT("Dissipation"), 1.0f);  // 无消散
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, Dst, MID_Advection);
}

// ============================================================================
// 全仿真管线 — 正确流体模拟顺序
// ============================================================================
//
//  0. ClearCollision (RT_Painter)
//  1. CollisionPainter (LineTrace → RT_Painter)
//  2. CompositeGradient: RT_Composite + RT_Painter → RT_WorkBuffer  (add forces)
//     CopyRT: RT_WorkBuffer → RT_Composite (forces applied)
//  3. Advection: RT_Composite → RT_Advection (advect along velocity)
//     CopyRT: RT_Advection → RT_Composite (advected state)
//  4. Divergence: RT_Composite → RT_PressureDivergence
//  5. PressureSolve: iterate on RT_PressureDivergence + Temp
//  6. PressureCorrection: RT_Composite - ∇p → RT_WorkBuffer  (project)
//     CopyRT: RT_WorkBuffer → RT_Composite (corrected)
//  7. DensityInjection: Canvas draw → RT_Composite
//  8. Copy: RT_Composite → RT_Output & UpdateDisplay
//
// ============================================================================
void UMyNinjaLiveComponent::RunSimulationPipeline(float DeltaTime)
{
	// ---- 种子帧: 初始密度 ----
	if (TickFrameCount < 3)
	{
		if (TickFrameCount == 0)
		{
			StepInjectDensityCanvas(FVector2D(0.5f, 0.5f), FVector2D(0.5f, 0.5f), 0.5f);
			StepInjectDensityCanvas(FVector2D(0.3f, 0.5f), FVector2D(0.52f, 0.5f), 0.3f);
			StepInjectDensityCanvas(FVector2D(0.7f, 0.5f), FVector2D(0.48f, 0.5f), 0.3f);
		}
		StepAdvection(DeltaTime);
		CopyRT(RT_Advection, RT_Composite);
		StepUpdateDisplay();
		return;
	}

	// ===================================================================
	// 主循环
	// ===================================================================

	// ---- Step 0: 清除碰撞 RT ----
	StepCollisionClear();

	// ---- Step 1: 碰撞绘制 (LineTrace → RT_Painter) ----
	if (bCameraTraceHit)
	{
		FVector2D VelEnc = EncodeVelocity(CameraTraceHitVelocity);
		StepCollisionPainter(CameraTraceHitUV, VelEnc, DeltaTime);
	}

	// ---- Step 2: 外力合成 (forces into velocity field) ----
	// RT_Composite(current) + RT_Painter(collision) → RT_WorkBuffer
	if (MID_CompositeGradient)
	{
		StepCompositeGradient(DeltaTime);
		CopyRT(RT_WorkBuffer, RT_Composite);  // forces applied → current state
	}

	// ---- Step 3: 平流 (advect density+velocity) ----
	StepAdvection(DeltaTime);
	CopyRT(RT_Advection, RT_Composite);      // advected → current state

	// ---- Step 4: 散度 ----
	StepDivergence();

	// ---- Step 5: 压力求解 ----
	StepPressureSolve();

	// ---- Step 6: 压力梯度修正 (v -= ∇p) ----
	StepPressureCorrection();
	CopyRT(RT_WorkBuffer, RT_Composite);      // corrected → current state

	// ---- Step 7: 密度注入 ----
	if (bCameraTraceHit)
	{
		FVector2D VelEnc = EncodeVelocity(CameraTraceHitVelocity);
		StepInjectDensityCanvas(CameraTraceHitUV, VelEnc);
	}

	// ---- Step 8: 更新显示 ----
	StepUpdateDisplay();
}

// ============================================================================
// 玩家脚步交互
// ============================================================================
void UMyNinjaLiveComponent::PlayerStepInteraction(float DeltaTime)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	FVector PlayerVelocity = PlayerPawn->GetVelocity();
	float Speed = PlayerVelocity.Size();

	if (Speed < DampenBrushBelowThisVelocity) return;

	FVector2D SimUV = WorldToSimUV(PlayerPawn->GetActorLocation());
	SimUV.X = FMath::Clamp(SimUV.X, 0.0f, 1.0f);
	SimUV.Y = FMath::Clamp(SimUV.Y, 0.0f, 1.0f);

	FVector2D VelEnc = EncodeVelocity(PlayerVelocity);
	StepInjectDensityCanvas(SimUV, VelEnc);
}

// ============================================================================
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
			CameraTraceHitVelocity = HitActor->GetVelocity();

		// 从 UV 运动计算速度（相机扫过平面时的视差速度）
		if (CameraTraceLastUV.X >= 0.0f && DeltaTime > 0.0f)
		{
			FVector2D UVDelta = (CameraTraceHitUV - CameraTraceLastUV) / DeltaTime;
			UVDelta.X = FMath::Clamp(UVDelta.X, -2.0f, 2.0f);
			UVDelta.Y = FMath::Clamp(UVDelta.Y, -2.0f, 2.0f);

			FVector CombinedVelocity = CameraTraceHitVelocity +
				FVector(UVDelta.X * MaxVelocity, UVDelta.Y * MaxVelocity, 0.0f);
			FVector2D VelEnc = EncodeVelocity(CombinedVelocity);
			StepCollisionPainter(CameraTraceHitUV, VelEnc, DeltaTime);
			AddInteractionPoint(CameraTraceHitUV, CombinedVelocity);
		}
		CameraTraceLastUV = CameraTraceHitUV;
	}
	else
	{
		CameraTraceLastUV = FVector2D(-1.0f, -1.0f);
	}
}

// ============================================================================
// 激活检测
// ============================================================================
bool UMyNinjaLiveComponent::ShouldSimulationRun() const
{
	if (!bInitDone || bDisableComponent) return false;
	if (!bComponentActivatedByPawnProximity) return true;
	if (ActiveActivationTargets.Num() > 0) return true;
	return bPawnInsideBounds;
}

// ============================================================================
// 重叠交互
// ============================================================================
void UMyNinjaLiveComponent::AddInteractionPoint(const FVector2D& UV, const FVector& WorldVelocity)
{
	if (!bInitDone || bDisableComponent) return;

	float Speed = WorldVelocity.Size();
	if (Speed < DampenBrushBelowThisVelocity) return;

	FVector2D VelEnc = EncodeVelocity(WorldVelocity);

	// 密度注入
	if (RT_Composite)
		StepInjectDensityCanvas(UV, VelEnc);

	// 碰撞绘制
	if (MID_CollisionPainterDot && RT_Painter)
	{
		const float EffectiveBrushSize = BrushSize * GlobalBrushScale * UserInputBrushScale;
		MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Position"), FLinearColor(UV.X, UV.Y, 0, 1));
		MID_CollisionPainterDot->SetVectorParameterValue(TEXT("Velocity"), FLinearColor(VelEnc.X, VelEnc.Y, 0, 1));
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushSize"), EffectiveBrushSize);
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushStrength"), BrushStrength);
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("BrushHardness"), BrushHardness);
		MID_CollisionPainterDot->SetScalarParameterValue(TEXT("DeltaTime"), 0.016f);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT_Painter, MID_CollisionPainterDot);
	}
}

void UMyNinjaLiveComponent::AddInteractionTarget(AActor* Target)
{
	if (Target) ActiveInteractionTargets.AddUnique(Target);
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
