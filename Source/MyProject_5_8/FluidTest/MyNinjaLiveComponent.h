// MyNinjaLiveComponent.h
// C++ 重现 FluidNinjaLive 核心 ActorComponent — 对齐 BP 参考管线
//
// GPU 流体模拟管线（正确顺序）：
//   碰撞绘制(CollisionPainter) → 外力合成(CompositeGradient)
//   → 平流(Advection) → 散度(Divergence) → 压力求解(PressureSolve)
//   → 压力梯度修正(PressureCorrection) → 显示(Display)
//
// RT 通道布局（对齐 BP CoreSim）：
//   RG = Velocity 场 (X/Y)  |  B = Density 场  |  A = 未使用
//
// RT 命名对齐 BP:
//   RT_Composite          — 外力合成输出 (velocity + collision force)
//   RT_Advection          — 平流结果 (advected density+velocity)
//   RT_Painter            — 碰撞绘制缓冲 (collision painter)
//   RT_PressureDivergence — 压力/散度场
//   RT_PressureDivergenceTemp — 压力求解迭代临时缓冲
//   RT_DensityInput       — 密度输入材质 (外部纹理/媒体输入)
//   RT_Output             — 最终显示输出

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "MyNinjaLiveComponent.generated.h"

class UTextureRenderTarget2D;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

// ---------------------------------------------------------------------------
// 碰撞绘制模式
// ---------------------------------------------------------------------------
UENUM(BlueprintType)
enum class EMyNinjaLiveBrushMode : uint8
{
	Dot     UMETA(DisplayName = "Dot"),
	Line    UMETA(DisplayName = "Line")
};

// ---------------------------------------------------------------------------
// 仿真目标信息 (Multi-Target)
// ---------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMySimTargetInfo
{
	GENERATED_BODY()

	UPROPERTY()
	UPrimitiveComponent* TargetComponent = nullptr;

	UPROPERTY()
	FTransform WorldTransform = FTransform::Identity;

	UPROPERTY()
	FVector2D LastUV = FVector2D(0.5f, 0.5f);

	UPROPERTY()
	FVector LastWorldPos = FVector::ZeroVector;

	UPROPERTY()
	float BrushScale = 1.0f;

	UPROPERTY()
	bool bActive = false;
};

// ---------------------------------------------------------------------------
// MyNinjaLiveComponent
// ---------------------------------------------------------------------------
UCLASS(ClassGroup=(FluidNinja), Blueprintable, BlueprintType,
	meta=(BlueprintSpawnableComponent, DisplayName="MyNinjaLiveComponent"))
class MYPROJECT_5_8_API UMyNinjaLiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyNinjaLiveComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// =============================================================
	// 公开属性 —— 可在细节面板编辑
	// =============================================================

	// ---- 开关 (对齐 BP: DisableComponent, ComponentActivatedByPawnProximity) ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Activation")
	bool bDisableComponent = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Activation")
	bool bComponentActivatedByPawnProximity = false;

	/** BP TickRateCustom = 0.016 (60fps) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Activation",
		meta=(ClampMin=0.0f))
	float TickRateCustom = 0.016f;

	// ---- 仿真分辨率 (BP 默认 256×256) ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=64, ClampMax=4096))
	int32 ResolutionX = 256;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=64, ClampMax=4096))
	int32 ResolutionY = 256;

	/** BP MaxSamplingFPS = 60 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=1, ClampMax=120))
	int32 MaxSamplingFPS = 60;

	/** BP MinSamplingFPS = 30 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=1, ClampMax=120))
	int32 MinSamplingFPS = 30;

	/** BP UsePressureSolver1_(DefaultIs2) = false → 默认用 Solver2 (1次迭代+kernel reduction) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation")
	bool bUsePressureSolver1 = false;

	/** BP PressureSolver1_MaxIterations = 5 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=1, ClampMax=128, EditCondition="bUsePressureSolver1"))
	int32 PressureSolver1_MaxIterations = 5;

	/** BP PressureSolver2_MaxIterations = 1 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=1, ClampMax=32, EditCondition="!bUsePressureSolver1"))
	int32 PressureSolver2_MaxIterations = 1;

	/** BP PressureSolver2_KernelReduction = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=0.1f, ClampMax=10.0f, EditCondition="!bUsePressureSolver1"))
	float PressureSolver2_KernelReduction = 1.0f;

	// ---- LOD (对齐 BP: false/false, 2000/5000) ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|LOD")
	bool bLOD1_ReduceSimQuality = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|LOD")
	bool bLOD2_ReduceSamplingFPS = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|LOD")
	float LOD_NearBound = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|LOD")
	float LOD_FarBound = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|LOD")
	int32 LOD_Steps = 5;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|LOD")
	float LOD_StepRange = 300.0f;

	/** 密度消散率/帧 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float Dissipation = 0.99f;

	// ---- 碰撞检测 (对齐 BP TraceChannels) ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TEnumAsByte<ETraceTypeQuery> TraceChannel = TraceTypeQuery3;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TArray<AActor*> ActorsToIgnore;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing",
		meta=(ClampMin=1.0f))
	float TraceDistance = 5000.0f;

	/** BP UseCustomTraceSource = false (从玩家相机发射射线) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	bool UseCustomTraceSource = false;

	/** BP CustomTraceSourcePosition = (0,0,5000) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	FVector CustomTraceSourcePosition = FVector(0.0f, 0.0f, 5000.0f);

	// ---- 画笔 (对齐 BP Brush 体系) ----
	/** BP GlobalBrushScale = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float GlobalBrushScale = 1.0f;

	/** BP UserInputBrushScale = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float UserInputBrushScale = 1.0f;

	/** BP BrushSize = 0.1 (通过 SetBrush 设置) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushSize = 0.1f;

	/** BP BrushStrength = 0.5 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float BrushStrength = 0.5f;

	/** BP BrushHardness = 0.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float BrushHardness = 0.0f;

	/** BP AdjustPainter_V2_BrushStrength = 1.4 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float AdjustPainter_V2_BrushStrength = 1.4f;

	/** BP AdjustPainter_V2_BrushVeloNoise = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f, ClampMax=2.0f))
	float AdjustPainter_V2_BrushVeloNoise = 1.0f;

	/** BP AdjustPainter_V2_EdgeMask = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float AdjustPainter_V2_EdgeMask = 1.0f;

	/** BP BrushVelocityNoiseFreq = 0.1 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushVelocityNoiseFreq = 0.1f;

	/** BP BrushVelocityNoiseScale = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushVelocityNoiseScale = 1.0f;

	/** BP DampenBrushBelowThisVelocity = 0.0 (不阻尼) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float DampenBrushBelowThisVelocity = 0.0f;

	/** BP BrushVelocityPow = 1.6 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.1f, ClampMax=10.0f))
	float BrushVelocityPow = 1.6f;

	/** BP BrushVelocityClamp = 1.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushVelocityClamp = 1.0f;

	/** BP BrushScaledByInteractingObjSize = false */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush")
	bool BrushScaledByInteractingObjSize = false;

	/** BP BrushScaledInverselyByTraceMeshSize = true */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush")
	bool BrushScaledInverselyByTraceMeshSize = true;

	/** BP BrushPuncture = 0.1 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushPuncture = 0.1f;

	// ---- 预设 (对齐 BP: DT_NinjaLive_Default, "NinjaLive") ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Preset")
	TObjectPtr<class UDataTable> DefaultPreset = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Preset")
	FName PresetNameFilterCriteria = TEXT("NinjaLive");

	/** BP ForceAutoLoadPreset = true */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Preset")
	bool bForceAutoLoadPreset = true;

	// ---- 杂项 (对齐 BP) ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Misc")
	bool bAutoConnectToMemoryPool = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Misc")
	float OffsetFromSimAreaMotion = 1.0f;

	/** BP SimEdgeBouncyness = 0.5 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Misc",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float SimEdgeBouncyness = 0.5f;

	/** BP FadeDensityAtSimEdge = 0.0 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Misc",
		meta=(ClampMin=0.0f))
	float FadeDensityAtSimEdge = 0.0f;

	/** BP EdgeMaskWidth = 0.25 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Misc",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float EdgeMaskWidth = 0.25f;

	/** 碰撞遮罩纹理 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Misc")
	TObjectPtr<UTexture> CollisionMask = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=1, ClampMax=16))
	int32 MaxTargets = 4;

	// ---- PV2 Painter 系统 ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|PainterV2")
	bool bUsePAINTER_V2_ToTrackObjects = true;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|PainterV2")
	bool bPV2_Connect_TrackpointsWithLines = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|PainterV2")
	bool bPV2_Interpolation = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|PainterV2")
	float PV2_LineDrawingFailCooldownTime = 0.01f;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|PainterV2")
	float PV2_StopLineDrawingAboveThisVelocity = 500.0f;

	// ---- CoreSimMaterials (对齐 BP 17 材质) ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials",
		meta=(DisplayName="CoreSimMaterials"))
	TArray<TObjectPtr<UMaterialInstance>> CoreSimMaterials;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CollisionPainterDotMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CollisionPainterLineMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CollisionPainterOffsetMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CompositeGradientMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* AdvectionMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* DivergenceMat = nullptr;

	/** MI_Pressure_Solver2_Step1 — 压力初始化 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* PressureSolverInitMat = nullptr;

	/** MI_Pressure_Solver1 — Jacobi 迭代 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* PressureSolverIterMat = nullptr;

	/** MI_Pressure_Solver2_Step2 — 压力梯度修正 v -= ∇p */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* PressureCorrectionMat = nullptr;

	/** 显示材质 (映射到 DisplayPlane) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* DisplayMat = nullptr;

	/** 输出材质数组 (对齐 BP OutputMaterials — 9 个) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	TArray<TObjectPtr<UMaterialInstance>> OutputMaterials;

	/** BP OutputMaterialSelected = 1 (默认选第2个: MI_DensityBuffer_Translucent) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	int32 OutputMaterialSelected = 1;

	/** 二级输出材质 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	TArray<TObjectPtr<UMaterialInstance>> SecondaryOutputMaterials;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	int32 SecondaryOutputMaterialSelected = 0;

	/** 三级输出材质 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	TArray<TObjectPtr<UMaterialInstance>> TertiaryOutputMaterials;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	int32 TertiaryOutputMaterialSelected = 0;

	// ---- 输出目标 ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output")
	UStaticMeshComponent* ExternalDisplayPlane = nullptr;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output")
	bool bCreateDefaultDisplayPlane = true;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output",
		meta=(ClampMin=1.0f))
	float PlaneWorldSize = 8200.0f;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output")
	float MaxVelocity = 500.0f;

	// ---- Debug ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Debug")
	bool bShowDebugMessages = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Debug")
	float DebugMessageDuration = 4.0f;

	// =============================================================
	// 公开函数
	// =============================================================

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void ForceEnable();

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void ForceDisable();

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void ResetSimulation();

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void SetBrush(float NewSize, float NewStrength, float NewHardness = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void SetResolution(int32 NewX, int32 NewY);

	/** 获取输出 RT (RT_Output) */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	UTextureRenderTarget2D* GetOutputRT() const { return RT_Output; }

	/** 获取复合 RT (RT_Composite — velocity+density) */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	UTextureRenderTarget2D* GetCompositeRT() const { return RT_Composite; }

	/** 获取碰撞 RT (RT_Painter) */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	UTextureRenderTarget2D* GetPainterRT() const { return RT_Painter; }

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void SetCustomInteractionActor(AActor* InActor) { CustomInteractionActor = InActor; }

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void AddInteractionPoint(const FVector2D& UV, const FVector& WorldVelocity);

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void AddInteractionTarget(AActor* Target);

	/** 世界坐标 → 仿真 UV (公开供 ASimpleFluidActor 使用) */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	FVector2D WorldToSimUV(const FVector& WorldPos) const;

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void RemoveInteractionTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void AddActiveTarget(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void RemoveActiveTarget(AActor* Target);

protected:
	// =============================================================
	// RenderTarget 管理 (对齐 BP 7 RT 命名)
	// =============================================================
	UPROPERTY()
	UTextureRenderTarget2D* RT_Composite;       // 外力合成输出 (velocity+collision force)

	UPROPERTY()
	UTextureRenderTarget2D* RT_Advection;       // 平流结果 (advected density+velocity)

	UPROPERTY()
	UTextureRenderTarget2D* RT_Painter;          // 碰撞绘制缓冲

	UPROPERTY()
	UTextureRenderTarget2D* RT_PressureDivergence;     // 压力/散度场

	UPROPERTY()
	UTextureRenderTarget2D* RT_PressureDivergenceTemp; // 压力求解迭代临时缓冲

	UPROPERTY()
	UTextureRenderTarget2D* RT_DensityInput;    // 密度输入材质

	UPROPERTY()
	UTextureRenderTarget2D* RT_Output;           // 最终显示输出

	// 内部双缓冲（不与 BP 直接对应，用于内部管线）
	UPROPERTY()
	UTextureRenderTarget2D* RT_WorkBuffer;       // 工作缓冲

	// =============================================================
	// 动态材质实例
	// =============================================================
	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterDot;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterLine;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterOffset;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_CompositeGradient;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_Advection;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_Divergence;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_PressureSolverInit;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_PressureSolverIter;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_PressureCorrection;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_Display;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_ActiveDisplay = nullptr;

	/** 预设变量 (对齐 BP PresetMap) */
	float Preset_VeloStrength = 1.0f;
	float Preset_VeloOffsetX = 0.0f;
	float Preset_VeloOffsetY = 0.0f;
	float Preset_VeloRotate = 0.0f;
	float Preset_VeloAmpNoise = 1.0f;
	float Preset_VeloDirNoise = 1.0f;
	float Preset_Speed = 1.0f;
	float Preset_Divergence = 1.0f;
	float Preset_InputFeedback = 0.2f;
	float Preset_FlowFeedback = 0.5f;
	float Preset_VeloFromSimAreaMotion = 0.0f;
	float Preset_VeloFromBrushMotion = 1.0f;

	// =============================================================
	// 内部状态
	// =============================================================
	UPROPERTY()
	AActor* CustomInteractionActor = nullptr;

	UPROPERTY()
	UStaticMeshComponent* DefaultDisplayPlane = nullptr;

	UPROPERTY()
	TArray<AActor*> TraceExcludeActors;

	UPROPERTY()
	TArray<FMySimTargetInfo> SimTargets;

	/** 仿真帧计时 (对齐 BP TickRateCustom) */
	float SimTimer = 0.0f;

	/** LOD 计时 */
	float LODCheckTimer = 0.0f;
	float LODCheckFrequency = 0.5f;

	UPROPERTY()
	bool bInitDone = false;

	bool bPawnInsideBounds = false;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveInteractionTargets;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveActivationTargets;

	/** 相机 LineTrace 结果 */
	FVector2D CameraTraceHitUV = FVector2D(-1.0f, -1.0f);
	FVector CameraTraceHitVelocity = FVector::ZeroVector;
	bool bCameraTraceHit = false;
	FVector2D CameraTraceLastUV = FVector2D(-1.0f, -1.0f);

	int32 CurrentLODLevel = 0;
	int32 TickFrameCount = 0;
	bool bPresetLoaded = false;
	float AccumulatedTime = 0.0f;

	// =============================================================
	// 内部函数
	// =============================================================

	UTextureRenderTarget2D* CreateRT(const FName& Name, int32 SizeX, int32 SizeY);

	void CreateAllRTs();
	void ClearAllRTs();
	bool CreateDynamicMaterialInstances();
	void CreateDefaultPlane();
	void SetupDisplay();

	AActor* GetInteractionActor() const;
	FVector2D EncodeVelocity(const FVector& WorldVelocity) const;

	FHitResult PerformLineTrace(const FVector& Start, const FVector& End) const;
	void GetTraceSource(FVector& OutStart, FVector& OutEnd) const;
	void HandleCameraLineTrace(float DeltaTime);
	void CheckPawnProximity();
	void CheckLOD(float DeltaTime);
	void LoadPreset();

	/** 通过 Advection MID 复制 RT (pass-through, Δt=0, dissipation=1) */
	void CopyRT(UTextureRenderTarget2D* Src, UTextureRenderTarget2D* Dst);

	// ---- 仿真管线步骤 ----
	void StepCollisionClear();
	void StepCollisionPainter(const FVector2D& UV, const FVector2D& VelocityEncoded,
		float DeltaTime, int32 TargetIndex = 0);
	void StepCompositeGradient(float DeltaTime);
	void StepAdvection(float DeltaTime);
	void StepDivergence();
	void StepPressureSolve();
	void StepPressureCorrection();
	void StepInjectDensityCanvas(const FVector2D& UV, const FVector2D& VelocityEncoded,
		float DensityAmount = 1.0f);
	void StepUpdateDisplay();

	void RunSimulationPipeline(float DeltaTime);
	void PlayerStepInteraction(float DeltaTime);
	bool ShouldSimulationRun() const;
};
