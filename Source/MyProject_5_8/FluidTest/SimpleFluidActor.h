// SimpleFluidActor.h — 全量复刻 /Game/FluidNinjaLive/NinjaLive.NinjaLive
//
// 组件结构对齐原版：
//   Root → TraceMesh(DisplayPlane) + EditorIcon
//        + ActivationVolume + InteractionVolume + NinjaLiveComponent
//
// 使用方式：在关卡中放置 BP_SimpleFluidActor 或 C++ ASimpleFluidActor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleFluidActor.generated.h"

class UMyNinjaLiveComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UMaterialBillboardComponent;

UENUM(BlueprintType)
enum class ETraceMeshInactiveBehaviour : uint8
{
	KeepVisible      UMETA(DisplayName = "Keep Visible"),
	Hide             UMETA(DisplayName = "Hide"),
	CollisionOnly    UMETA(DisplayName = "Collision Only")
};

UCLASS(Blueprintable)
class MYPROJECT_5_8_API ASimpleFluidActor : public AActor
{
	GENERATED_BODY()

public:
	ASimpleFluidActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// =============================================================
	// 组件 (对齐 NinjaLive BP)
	// =============================================================

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** 对齐 BP TraceMesh */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DisplayPlane;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UMaterialBillboardComponent> EditorIcon;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UMyNinjaLiveComponent> NinjaLiveComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> ActivationVolume;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionVolume;

	// =============================================================
	// Actor 属性 (对齐 NinjaLive BP 的 Actor 级配置)
	// =============================================================

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Interaction")
	bool ShowTraceMeshInEditor = true;  // BP CDO: true

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Activation")
	ETraceMeshInactiveBehaviour TraceMeshInactiveBehaviour = ETraceMeshInactiveBehaviour::KeepVisible;

	/** BP TraceMeshSize = (1,1,1) — DisplayPlane scale */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Interaction")
	FVector TraceMeshSize = FVector(1.0f, 1.0f, 1.0f);

	/** BP SimActivatedByPawnProximity = false */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Activation")
	bool SimActivatedByPawnProximity = false;

	/** BP ActivationVolumeSize = (50,50,50) */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Activation")
	FVector ActivationVolumeSize = FVector(50.0f, 50.0f, 50.0f);

	/** BP InteractionVolumeSize = (1,1,1) */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Interaction")
	FVector InteractionVolumeSize = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Interaction")
	FName InteractionVolumeTemplate = "InteractionVolume";

	/** BP OverlapFilterInclusiveObjType = all */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Interaction")
	TArray<TEnumAsByte<EObjectTypeQuery>> OverlapFilterInclusiveObjType;

	/** BP OverlapFilterInclusiveBoneNameExact = empty (no bone filter) */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Interaction")
	TArray<FName> OverlapFilterInclusiveBoneNameExact;

	/** BP 不活跃时灰化材质 */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Activation")
	TObjectPtr<class UMaterialInstance> InactiveGrayMaterial;

	// ---- 组件覆盖属性 (传递到 NinjaLiveComponent) ----
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(InlineEditConditionToggle))
	bool OverrideComponentVariables = false;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	int32 Override_ResolutionX = 256;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	int32 Override_ResolutionY = 256;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	float Override_GlobalBrushScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	int32 Override_OutputMaterialSelected = 1;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	bool Override_bLOD1_ReduceSimQuality = false;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	bool Override_bLOD2_ReduceSamplingFPS = false;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	float Override_LOD_NearBound = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	float Override_LOD_FarBound = 5000.0f;

	/** BP DownscaleCollisionPainterResolution = 1 */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	int32 Override_DownscaleCollisionPainterResolution = 1;

	/** BP DownscalePressureResolution = 1 */
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Component Overrides",
		meta=(EditCondition="OverrideComponentVariables"))
	int32 Override_DownscalePressureResolution = 1;

protected:
	UFUNCTION()
	void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnActivationVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnActivationVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool DoesOverlapPassFilter(AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex = INDEX_NONE) const;
	FVector2D OverlapToSimUV(const FVector& WorldPos) const;
	void SyncPropertiesToComponent();
};
