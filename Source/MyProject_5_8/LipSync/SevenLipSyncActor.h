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

	/** Extra multiplier on jawOpen only (the "mouth opens too wide" knob). 1 = tuned default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "3"))
	float LipSyncJawOpenScale = 1.0f;

	/** Hard ceiling for jawOpen. Lower it further if the mouth still gapes on loud vowels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0.05", ClampMax = "1"))
	float LipSyncJawOpenMax = 0.60f;

	/** Multiplier for funnel / pucker (the round "O" that stacks on top of the jaw drop). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "2"))
	float LipSyncRoundingScale = 1.0f;

	/**
	 * jawOpen deadzone. Everything at or below this counts as a closed mouth, so
	 * raise it (0.12~0.18) if the mouth still looks permanently ajar between
	 * syllables; set it to 0 to get the raw model curve back.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "0.6"))
	float LipSyncJawOpenFloor = 0.10f;

	/** How fast a morph opens (FInterpTo speed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "1", ClampMax = "120"))
	float LipSyncOpenSpeed = 30.0f;

	/** How fast it shuts - keep this above OpenSpeed or the syllables smear together. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "1", ClampMax = "120"))
	float LipSyncCloseSpeed = 40.0f;

	/**
	 * 1 = funnel / lower-lip shapes fade out as the jaw closes, so a shut jaw
	 * really seals the mouth. Drop to 0 to keep the raw model's lip shapes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "1"))
	float LipSyncLipFollowJaw = 1.0f;

	/** Seconds between debug log lines; ~0.03 traces the open/close envelope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio|Debug", meta = (ClampMin = "0.01", ClampMax = "5"))
	float LipSyncLogInterval = 0.5f;

	/** Silence gate RMS thresholds (16 kHz domain). Raise GateLowRms if the mouth still twitches in silence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "0.5"))
	float LipSyncGateLowRms = 0.015f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Audio", meta = (ClampMin = "0", ClampMax = "0.5"))
	float LipSyncGateHighRms = 0.035f;

	/**
	 * Seconds after BeginPlay during which the local player's view is forced
	 * through this camera. 1.5 s is enough to see the face at startup; raise it
	 * (e.g. 300) to keep PIE framed on the face while tuning.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven|Framing", meta = (ClampMin = "0", ClampMax = "3600"))
	float ViewTargetLockSeconds = 1.5f;

private:
};
