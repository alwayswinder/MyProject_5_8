// Seven lip sync: drives ARKit 52 morph targets from the Runtime MetaHuman Lip Sync
// realistic (MetaHuman control) model.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SevenLipSyncComponent.generated.h"

class USkeletalMeshComponent;
class URealisticMetaHumanLipSyncGenerator;
class USoundWaveProcedural;
class UAudioComponent;

/**
 * Reads MetaHuman GUI control values from URealisticMetaHumanLipSyncGenerator and
 * maps them onto the ARKit blend shape names that the Seven mesh actually has.
 * Audio is streamed from a .wav file at 16 kHz.
 */
UCLASS(ClassGroup = (LipSync), meta = (BlueprintSpawnableComponent))
class MYPROJECT_5_8_API USevenLipSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USevenLipSyncComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Start streaming a wav file through the lip sync model. */
	UFUNCTION(BlueprintCallable, Category = "Seven LipSync")
	void StartPlayback(const FString& InWavPath);

	UFUNCTION(BlueprintCallable, Category = "Seven LipSync")
	void StopPlayback();

	UFUNCTION(BlueprintCallable, Category = "Seven LipSync")
	bool IsPlaying() const { return bPlaying; }

	/** Mesh whose morph targets get driven. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

	/** Wav file to play. Relative paths resolve against the project directory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	FString WavFile = TEXT("TestAudio/SevenSpeech.wav");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	float Gain = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	float SmoothingSpeed = 30.0f;

	/**
	 * Silence gate, in RMS of the 16 kHz signal. Below GateLowRms the mouth is
	 * forced shut, above GateHighRms it is fully driven, linear in between.
	 * Kills the model's "hallucinated" mouth movement on leading/trailing
	 * silence, breath and low-level noise.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync", meta = (ClampMin = "0", ClampMax = "0.5"))
	float GateLowRms = 0.015f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync", meta = (ClampMin = "0", ClampMax = "0.5"))
	float GateHighRms = 0.035f;

	/** Don't touch brow/eye/cheek channels (mouth only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	bool bMouthOnly = true;

	/**
	 * How far ahead of the audio clock the model is fed, in milliseconds.
	 * Positive = lips earlier than the sound, negative = lips later.
	 * Use it to cancel device/model latency: dial it until the lips line up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync", meta = (ClampMin = "-1500", ClampMax = "400"))
	float LipSyncLookaheadMs = 0.0f;

	/** Restart the wav when it reaches the end (handy for a demo loop). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	bool bLoopAudio = true;

	/** Silence between loops, so you can actually see the mouth stop with the sound. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync", meta = (ClampMin = "0", ClampMax = "10"))
	float LoopGapSeconds = 1.0f;

	/** Log a few morph values every half second (verbose proof while tuning). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	bool bLogValues = true;

private:
	void EnsureGenerator();
	void LoadWavFile(const FString& InPath);
	void FeedAudio(float DeltaTime);
	void UpdateMorphTargets(float DeltaTime);
	void StartAudio(const TArray<float>& Samples);
	void StopAudio();

	UPROPERTY(Transient)
	TObjectPtr<URealisticMetaHumanLipSyncGenerator> Generator;

	/** Procedural sound so the clip is actually audible while it drives the face. */
	UPROPERTY(Transient)
	TObjectPtr<USoundWaveProcedural> ProceduralWave;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AudioComponent;

	TArray<float> AudioSamples;
	int32 ReadCursor = 0;
	int32 QueuedBytes = 0;
	bool bPlaying = false;
	double SampleAccumulator = 0.0;
	TMap<FName, float> MorphValues;
	bool bLoggedControlNames = false;
	float DebugLogTimer = 0.0f;
	float DebugPlayedSeconds = 0.0f;
	float DebugCursorSeconds = 0.0f;
	float DebugGate = 1.0f;
	float SmoothedGate = 1.0f;

	/** The ONNX model needs ~1 s to load; playing audio before that desyncs the first pass. */
	bool bWarmedUp = false;
	float WarmupRemaining = 0.0f;
	float LoopGapRemaining = 0.0f;
};
