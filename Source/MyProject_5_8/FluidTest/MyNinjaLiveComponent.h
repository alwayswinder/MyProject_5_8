// MyNinjaLiveComponent.h
// C++ 实现 FluidNinjaLive 核心 ActorComponent（与原版 Blueprint 组件不冲突）
//
// 提供基于 RenderTarget 的 GPU 流体模拟管线：
//   碰撞检测(Linetrace) → 碰撞绘制(CollisionPainter)
//   → 密度注入 → 平流(Advection) → 外力合成(CompositeGradient)
//   → 散度(Divergence) → 压力求解(PressureSolve) → 显示(Display)
//
// 用法：将本组件附加到任意 Actor，在细节面板中指定 FluidNinjaLive
// 材质引用和仿真参数即可。

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

	// 初始化与生命周期
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// =============================================================
	// 公开属性 —— 可在细节面板编辑
	// =============================================================

	// ---- 开关 ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Activation")
	bool bDisableComponent = false;

	/** 基于玩家距离自动激活/休眠 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Activation")
	bool bActivatedByPawnProximity = false;

	/** 激活距离 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Activation",
		meta=(EditCondition="bActivatedByPawnProximity", ClampMin=100.0f))
	float ActivationDistance = 2000.0f;

	// ---- 仿真分辨率 ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=64, ClampMax=1024))
	int32 ResolutionX = 256;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=64, ClampMax=1024))
	int32 ResolutionY = 256;

	/** 仿真 FPS（0 = 每帧执行） */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=0, ClampMax=120))
	int32 SimFPS = 0;

	/** 压力求解器迭代次数 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=1, ClampMax=32))
	int32 PressureIterations = 8;

	/** 密度消散率/帧 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Simulation",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float Dissipation = 0.995f;

	// ---- 碰撞检测 ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TEnumAsByte<ETraceTypeQuery> TraceChannel = TraceTypeQuery1;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing")
	TArray<AActor*> ActorsToIgnore;

	/** 追踪距离 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing",
		meta=(ClampMin=1.0f))
	float TraceDistance = 5000.0f;

	/** 每帧最大 LineTrace 数 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Tracing",
		meta=(ClampMin=1, ClampMax=128))
	int32 MaxLineTracePerFrame = 16;

	// ---- 画笔 ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushSize = 0.05f;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f))
	float BrushStrength = 1.0f;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float BrushHardness = 0.5f;

	/** UV 空间中最大追踪点数量(自动多目标) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Brush",
		meta=(ClampMin=1, ClampMax=16))
	int32 MaxTargets = 4;

	// ---- 材质引用 ----
	/** 碰撞绘制材质(点模式) — 在细节面板指定 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CollisionPainterDotMat = nullptr;

	/** 碰撞绘制材质(线模式) */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CollisionPainterLineMat = nullptr;

	/** 密度注入材质（可选，默认复用 CollisionPainterDot） */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* DensityInjectMat = nullptr;

	/** 平流材质 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* AdvectionMat = nullptr;

	/** 外力合成+梯度材质 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* CompositeGradientMat = nullptr;

	/** 散度计算材质 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* DivergenceMat = nullptr;

	/** 压力求解材质 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* PressureSolverMat = nullptr;

	/** 显示材质 — 应用到 ExternalRenderTarget 上 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Materials")
	UMaterialInstance* DisplayMat = nullptr;

	// ---- 输出目标 ----
	/** 外部显示平面（StaticMeshComponent）— 将模拟结果显示在此平面上 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output")
	UStaticMeshComponent* ExternalDisplayPlane = nullptr;

	/** 是否创建内置平面（当 ExternalDisplayPlane 未设置时） */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output")
	bool bCreateDefaultDisplayPlane = true;

	/** 平面世界空间尺寸 */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output",
		meta=(ClampMin=1.0f))
	float PlaneWorldSize = 2000.0f;

	/** 是否显示密度场（false=显示速度场） */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output")
	bool bShowDensity = true;

	/** 最大编码速度（用于速度材质编码） */
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Output",
		meta=(ClampMin=1.0f))
	float MaxVelocity = 500.0f;

	// ---- Debug ----
	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Debug")
	bool bShowDebugMessages = false;

	UPROPERTY(EditAnywhere, Category = "MyNinjaLive|Debug")
	float DebugMessageDuration = 5.0f;

	// =============================================================
	// 公开函数
	// =============================================================

	/** 强制启用组件 */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void ForceEnable();

	/** 强制禁用组件 */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void ForceDisable();

	/** 重置所有 RenderTarget（清空仿真） */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void ResetSimulation();

	/** 设置画笔参数 */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void SetBrush(float NewSize, float NewStrength, float NewHardness = 0.5f);

	/** 设置仿真分辨率（需要 ResetSimulation 生效） */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void SetResolution(int32 NewX, int32 NewY);

	/** 获取密度 RenderTarget */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	UTextureRenderTarget2D* GetDensityRT() const { return RT_DensityA; }

	/** 获取速度 RenderTarget */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	UTextureRenderTarget2D* GetVelocityRT() const { return RT_VelocityA; }

	/** 获取外部显示 RT（供外部材质使用） */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	UTextureRenderTarget2D* GetExternalRT() const { return RT_External; }

	/** 添加自定义交互 Actor（默认使用 GetPlayerPawn(0)） */
	UFUNCTION(BlueprintCallable, Category = "MyNinjaLive")
	void SetCustomInteractionActor(AActor* InActor) { CustomInteractionActor = InActor; }

protected:
	// =============================================================
	// RenderTarget 管理
	// =============================================================
	UPROPERTY()
	UTextureRenderTarget2D* RT_DensityA;

	UPROPERTY()
	UTextureRenderTarget2D* RT_DensityB;

	UPROPERTY()
	UTextureRenderTarget2D* RT_VelocityA;

	UPROPERTY()
	UTextureRenderTarget2D* RT_VelocityB;

	UPROPERTY()
	UTextureRenderTarget2D* RT_Pressure;

	UPROPERTY()
	UTextureRenderTarget2D* RT_Divergence;

	UPROPERTY()
	UTextureRenderTarget2D* RT_Collision;

	UPROPERTY()
	UTextureRenderTarget2D* RT_External;

	// 附加碰撞 RT（用于多目标）
	UPROPERTY()
	UTextureRenderTarget2D* RT_Collision2;

	// =============================================================
	// 动态材质实例
	// =============================================================
	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterDot;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_CollisionPainterLine;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_DensityInject;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_Advection;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_CompositeGradient;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_Divergence;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_PressureSolver;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_Display;

	// =============================================================
	// 内部状态
	// =============================================================
	UPROPERTY()
	AActor* CustomInteractionActor = nullptr;

	/** 内置显示平面（当 ExternalDisplayPlane 未设置时创建） */
	UPROPERTY()
	UStaticMeshComponent* DefaultDisplayPlane = nullptr;

	/** 追踪排除列表 */
	UPROPERTY()
	TArray<AActor*> TraceExcludeActors;

	/** 多目标信息 */
	UPROPERTY()
	TArray<FMySimTargetInfo> SimTargets;

	/** 仿真帧计时 */
	float SimTimer = 0.0f;
	float SimInterval = 0.0f;

	/** 上一帧玩家位置 */
	FVector LastPlayerWorldPos = FVector::ZeroVector;

	/** 组件是否已初始化 */
	bool bInitialized = false;

	/** 玩家是否在激活范围内 */
	bool bPawnInsideBounds = false;

	// =============================================================
	// 内部函数
	// =============================================================

	/** 创建 RenderTarget */
	UTextureRenderTarget2D* CreateRT(const FName& Name, int32 SizeX, int32 SizeY);

	/** 创建所有 RT 并清除 */
	void CreateAllRTs();

	/** 交换双缓冲 RT */
	void SwapRT(UTextureRenderTarget2D*& A, UTextureRenderTarget2D*& B);

	/** 清除所有 RT */
	void ClearAllRTs();

	/** 创建动态材质实例 */
	bool CreateDynamicMaterialInstances();

	/** 创建内置显示平面 */
	void CreateDefaultPlane();

	/** 设置显示材质 */
	void SetupDisplay();

	/** 获取交互 Actor */
	AActor* GetInteractionActor() const;

	/** 世界坐标 → 仿真 UV */
	FVector2D WorldToSimUV(const FVector& WorldPos) const;

	/** 世界速度 → 编码到 0~1（0.5 = 零速度） */
	FVector2D EncodeVelocity(const FVector& WorldVelocity) const;

	/** 执行 LineTrace，返回命中结果 */
	FHitResult PerformLineTrace(const FVector& Start, const FVector& End) const;

	/** 获取追踪起点（相机或 Actor 位置） */
	void GetTraceSource(FVector& OutStart, FVector& OutEnd) const;

	/** 检查玩家距离 */
	void CheckPawnProximity();

	// ---- 仿真管线步骤 ----
	void StepCollisionPainter(const FVector2D& UV, const FVector2D& VelocityEncoded,
		float DeltaTime, int32 TargetIndex = 0);
	void StepInjectDensity(const FVector2D& UV, const FVector2D& VelocityEncoded);
	void StepAdvection(UTextureRenderTarget2D* Src,
		UTextureRenderTarget2D* DstWrite, float DeltaTime);
	void StepCompositeGradient(float DeltaTime);
	void StepDivergence();
	void StepPressureSolve();
	void StepUpdateDisplay();

	/** 全仿真管线 */
	void RunSimulationPipeline(float DeltaTime);
};
