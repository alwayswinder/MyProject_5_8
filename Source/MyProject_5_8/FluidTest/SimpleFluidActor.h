// SimpleFluidActor.h — 全量复刻 /Game/FluidNinjaLive/NinjaLive.NinjaLive
//
// 组件结构对齐原版：
//   Root → DisplayPlane(TraceMesh) + ActivationVolume + InteractionVolume + NinjaLiveComponent

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleFluidActor.generated.h"

class UMyNinjaLiveComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UMaterialBillboardComponent;

/** TraceMesh 非激活行为 (对齐原版) */
UENUM(BlueprintType)
enum class ETraceMeshInactiveBehaviour : uint8
{
	KeepVisible		UMETA(DisplayName = "Keep Visible"),
	Hide			UMETA(DisplayName = "Hide"),
	CollisionOnly	UMETA(DisplayName = "Collision Only")
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

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	bool ShowTraceMeshInEditor = false;  // BP CDO (Pool_0: false)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	ETraceMeshInactiveBehaviour TraceMeshInactiveBehaviour = ETraceMeshInactiveBehaviour::KeepVisible;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	FVector TraceMeshSize = FVector(20.5f, 20.5f, 1.0f);  // BP CDO (Pool_0: 20.5,20.5,1.0)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	bool SimActivatedByPawnProximity = true;  // BP CDO (Pool_0: true)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	FVector ActivationVolumeSize = FVector(80.0f, 80.0f, 50.0f);  // BP CDO (Pool_0: 80,80,50)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	FVector InteractionVolumeSize = FVector(20.5f, 20.5f, 0.1f);  // BP CDO (Pool_0: 20.5,20.5,0.1)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	FName InteractionVolumeTemplate = "InteractionVolume";

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	TArray<TEnumAsByte<EObjectTypeQuery>> OverlapFilterInclusiveObjType;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Actor")
	TArray<FName> OverlapFilterInclusiveBoneNameExact;

	// =============================================================
	// 组件转发属性 (直接在细节面板配置)
	// =============================================================

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Simulation",
		meta=(ClampMin=64, ClampMax=4096))
	int32 ResolutionX = 1600;  // BP CDO (Pool_0: 1600)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Simulation",
		meta=(ClampMin=64, ClampMax=4096))
	int32 ResolutionY = 1600;  // BP CDO (Pool_0: 1600)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Brush")
	float GlobalBrushScale = 4.0f;  // BP CDO (Pool_0: 4.0)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Brush")
	float UserInputBrushScale = 1.2f;  // BP CDO (Pool_0: 1.2)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Brush")
	bool BrushScaledByInteractingObjSize = true;  // BP CDO (Pool_0: true)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Brush")
	float BrushVelocityNoiseFreq = 0.1f;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Brush")
	float DampenBrushBelowThisVelocity = 0.01f;  // BP CDO (Pool_0: 0.01)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Trace")
	bool UseCustomTraceSource = true;  // BP CDO (Pool_0: true)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Trace")
	FVector CustomTraceSourcePosition = FVector(200.0f, 200.0f, 5000.0f);  // BP CDO (Pool_0: 200,200,5000)

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Trace")
	float TraceDistance = 5000.0f;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Simulation",
		meta=(ClampMin=1, ClampMax=32))
	int32 PressureIterations = 8;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Simulation",
		meta=(ClampMin=0.0f, ClampMax=1.0f))
	float Dissipation = 0.99f;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Simulation")
	float PlaneWorldSize = 8200.0f;  // SM_plane_400x400 * scale 20.5

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Simulation")
	float MaxVelocity = 500.0f;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Preset")
	TObjectPtr<class UDataTable> DefaultPreset;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Preset")
	FString PresetNameFilterCriteria = TEXT("Usecase");

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TArray<TObjectPtr<class UMaterialInstance>> OutputMaterials;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	int32 OutputMaterialSelected = 0;  // BP CDO (Pool_0: 0)

	// ---- 材质 ----
	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> CollisionPainterDotMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> CollisionPainterLineMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> AdvectionMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> CompositeGradientMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> DivergenceMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> PressureSolverMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> PressureSolverIterMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> DisplayMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Materials")
	TObjectPtr<class UMaterialInstance> PressureCorrectionMat;

	UPROPERTY(EditAnywhere, Category = "NinjaLive|Debug")
	bool bShowDebugMessages = false;

protected:
	/** Overlap handlers — 对齐原版 OverlapFilter 逻辑 */
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

	/** 判断重叠对象是否符合过滤条件 */
	bool DoesOverlapPassFilter(AActor* Other, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex = INDEX_NONE) const;

	/** 从重叠位置计算仿真 UV */
	FVector2D OverlapToSimUV(const FVector& WorldPos) const;

	/** 同步所有属性到组件 */
	void SyncPropertiesToComponent();
};
