// SimpleFluidActor.h
// FluidNinjaLive Actor — 包装 UMyNinjaLiveComponent，提供场景中的放置点
//
// 参考蓝图 /Game/FluidNinjaLive/NinjaLive.NinjaLive 结构：
//   - Root (SceneComponent)
//   - DisplayPlane / TraceMesh (StaticMeshComponent)
//   - MyNinjaLiveComponent
//   - 激活/交互体积 (BoxComponent)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleFluidActor.generated.h"

class UMyNinjaLiveComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS(Blueprintable)
class MYPROJECT_5_8_API ASimpleFluidActor : public AActor
{
	GENERATED_BODY()

public:
	ASimpleFluidActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// =============================================================
	// 组件
	// =============================================================

	/** 根 SceneComponent */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	/** 显示平面 / TraceMesh — 流体模拟输出在这个平面上 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* DisplayPlane;

	/** C++ 版 NinjaLiveComponent */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UMyNinjaLiveComponent* NinjaLiveComponent;

	/** 激活体积（可选）— 用于玩家距离激活检测 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* ActivationVolume;

	/** 交互体积（可选）— 用于重叠检测 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* InteractionVolume;

	// ---- 体积尺寸 ----

	/** 激活体积半尺寸（XY 应与 DisplayPlane 覆盖范围匹配） */
	UPROPERTY(EditAnywhere, Category = "Components|Volumes")
	FVector ActivationVolumeExtent = FVector(1000.0f, 1000.0f, 500.0f);

	/** 交互体积半尺寸（XY 应与 DisplayPlane 范围匹配，Z 为表面交互厚度） */
	UPROPERTY(EditAnywhere, Category = "Components|Volumes")
	FVector InteractionVolumeExtent = FVector(1000.0f, 1000.0f, 5.0f);

	// =============================================================
	// 转发属性 — 映射到 UMyNinjaLiveComponent 的对应属性
	// =============================================================

	// ---- 激活 ----
	UPROPERTY(EditAnywhere, Category = "Activation",
		meta=(InlineEditConditionToggle))
	bool bOverride_Activation = false;

	UPROPERTY(EditAnywhere, Category = "Activation",
		meta=(EditCondition="bOverride_Activation"))
	bool bDisableComponent = false;

	UPROPERTY(EditAnywhere, Category = "Activation",
		meta=(EditCondition="bOverride_Activation"))
	bool bActivatedByPawnProximity = false;

	UPROPERTY(EditAnywhere, Category = "Activation",
		meta=(EditCondition="bOverride_Activation", ClampMin=100.0f))
	float ActivationDistance = 2000.0f;

	// ---- 仿真 ----
	UPROPERTY(EditAnywhere, Category = "Simulation",
		meta=(InlineEditConditionToggle))
	bool bOverride_Simulation = false;

	UPROPERTY(EditAnywhere, Category = "Simulation",
		meta=(EditCondition="bOverride_Simulation", ClampMin=64, ClampMax=2048))
	int32 ResolutionX = 512;

	UPROPERTY(EditAnywhere, Category = "Simulation",
		meta=(EditCondition="bOverride_Simulation", ClampMin=64, ClampMax=2048))
	int32 ResolutionY = 512;

	UPROPERTY(EditAnywhere, Category = "Simulation",
		meta=(EditCondition="bOverride_Simulation", ClampMin=0, ClampMax=120))
	int32 SimFPS = 0;

	UPROPERTY(EditAnywhere, Category = "Simulation",
		meta=(EditCondition="bOverride_Simulation", ClampMin=1, ClampMax=32))
	int32 PressureIterations = 8;

	UPROPERTY(EditAnywhere, Category = "Simulation",
		meta=(EditCondition="bOverride_Simulation", ClampMin=0.0f, ClampMax=1.0f))
	float Dissipation = 0.99f;

	// ---- 画笔 ----
	UPROPERTY(EditAnywhere, Category = "Brush",
		meta=(InlineEditConditionToggle))
	bool bOverride_Brush = false;

	UPROPERTY(EditAnywhere, Category = "Brush",
		meta=(EditCondition="bOverride_Brush", ClampMin=0.0f))
	float BrushSize = 0.15f;  // 15% UV (原 0.05)

	UPROPERTY(EditAnywhere, Category = "Brush",
		meta=(EditCondition="bOverride_Brush", ClampMin=0.0f))
	float BrushStrength = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Brush",
		meta=(EditCondition="bOverride_Brush", ClampMin=0.0f, ClampMax=1.0f))
	float BrushHardness = 0.5f;

	// ---- 追踪 ----
	UPROPERTY(EditAnywhere, Category = "Tracing")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, Category = "Tracing")
	float TraceDistance = 5000.0f;

	/** 平面世界尺寸（影响 UV 映射） */
	UPROPERTY(EditAnywhere, Category = "Tracing",
		meta=(ClampMin=1.0f))
	float PlaneWorldSize = 2000.0f;

	/** 最大编码速度 */
	UPROPERTY(EditAnywhere, Category = "Tracing",
		meta=(ClampMin=1.0f))
	float MaxVelocity = 500.0f;

	// ---- 材质 ----
	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* CollisionPainterDotMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* CollisionPainterLineMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* AdvectionMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* CompositeGradientMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* DivergenceMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* PressureSolverMat = nullptr;

	UPROPERTY(EditAnywhere, Category = "Materials")
	class UMaterialInstance* DisplayMat = nullptr;

	// ---- Debug ----
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebugMessages = false;

protected:
	/** 将 Actor 属性同步到组件（在 BeginPlay / OnConstruction 时调用） */
	void SyncPropertiesToComponent();
};
