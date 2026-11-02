#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "SevenLipSyncActor.generated.h"

class USkeletalMeshComponent;
class UPointLightComponent;
class USevenLipSyncComponent;

/**
 * Self contained test rig: a camera framed on Seven's face, a key light and the
 * lip sync component. Grabs the local player's view target for a second after
 * BeginPlay so PIE always looks at the face.
 */
UCLASS()
class MYPROJECT_5_8_API ASevenLipSyncActor : public ACameraActor
{
	GENERATED_BODY()

public:
	ASevenLipSyncActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seven")
	TObjectPtr<USkeletalMeshComponent> FaceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seven")
	TObjectPtr<UPointLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seven")
	TObjectPtr<USevenLipSyncComponent> LipSync;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Framing")
	FVector MeshLocation = FVector(105.0f, 0.0f, -158.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Framing")
	FRotator MeshRotation = FRotator(0.0f, 90.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Framing")
	float MeshScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Framing")
	float CameraFOV = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Lighting")
	float KeyLightIntensity = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio")
	FString WavFile = TEXT("TestAudio/SevenSpeech.wav");

	/** Model feed offset vs. the audio clock (ms). Negative = lips later than sound. Live-editable during PIE. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "-1500", ClampMax = "400"))
	float LipSyncLookaheadMs = 0.0f;

	/** Overall mouth amplitude. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "3"))
	float LipSyncGain = 1.0f;

	/** Silence gate RMS thresholds (16 kHz domain). Raise GateLowRms if the mouth still twitches in silence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "0.5"))
	float LipSyncGateLowRms = 0.015f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "0.5"))
	float LipSyncGateHighRms = 0.035f;

private:
	/** Seconds left during which we keep forcing the local player to look through this camera. */
	float ViewTargetLockTime = 1.5f;
};
