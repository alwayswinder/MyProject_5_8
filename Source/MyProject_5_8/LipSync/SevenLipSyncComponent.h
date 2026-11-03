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
class UAnimSequence;

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

	/**
	 * Extra multiplier applied to jawOpen only, on top of the mapping table.
	 * jawOpen is what makes the mouth look "too wide open", and the global Gain
	 * carries the lips (smile / frown / close) with it, so it gets its own knob.
	 * 1.0 = the tuned default; 0.5 = half the jaw travel; 1.5 = more shouty.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "0", ClampMax = "3"))
	float JawOpenScale = 1.0f;

	/**
	 * Hard ceiling for jawOpen (0..1). The ARKit 1.0 gape is an extreme shout;
	 * this head already reads as shouting around 0.6, so cap it there. Only clips
	 * the loudest vowels, the dynamics below stay linear.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "0.05", ClampMax = "1"))
	float JawOpenMax = 0.60f;

	/** Multiplier for the rounding channels (mouthFunnel / mouthPucker) that stack on the jaw drop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "0", ClampMax = "2"))
	float RoundingScale = 1.0f;

	/**
	 * Deadzone on jawOpen, applied after scaling: everything at or below this
	 * value (0..1) is treated as a fully closed mouth, and the range above it is
	 * stretched back over 0..1.
	 *
	 * The model never really returns the jaw to zero while somebody is talking -
	 * it sits around 0.07-0.12 of its own output, which on this head is already a
	 * visible gap. Without the deadzone the mouth therefore looks permanently
	 * ajar, "open, open, open" instead of open-close-open-close.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "0", ClampMax = "0.6"))
	float JawOpenFloor = 0.10f;

	/**
	 * FInterpTo speed used when a morph target is rising. 30 is snappy enough for
	 * speech; lower it for a lazier, more stylised mouth.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "1", ClampMax = "120"))
	float OpenSpeed = 30.0f;

	/**
	 * FInterpTo speed used when a morph target is falling (the mouth shutting).
	 * Deliberately faster than OpenSpeed so every syllable actually closes
	 * instead of smearing into the next one.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "1", ClampMax = "120"))
	float CloseSpeed = 40.0f;

	/**
	 * How much the other "mouth opening" channels (mouthFunnel, mouthLowerDown)
	 * follow the jaw. The model keeps 0.1-0.35 on those almost all the time and on
	 * this mesh that alone holds the lips apart - jawOpen can be 0 and the mouth
	 * still shows a hole. At 1 those shapes fade out as the jaw closes, so a shut
	 * jaw means a shut mouth. At 0 they keep their own value (raw model behaviour).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape", meta = (ClampMin = "0", ClampMax = "1"))
	float LipFollowJaw = 1.0f;

	/**
	 * Per-morph multipliers applied on top of the mapping table, e.g.
	 * ("mouthStretchLeft", 0.0) to switch off a morph that creases this mesh.
	 * Runtime editable, so a suspect channel can be zeroed live during PIE and
	 * the artifact watched as it goes away.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Mouth Shape")
	TMap<FName, float> MorphScaleOverrides;

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

	/**
	 * Body animation restarted from frame 0 every time a playback pass starts,
	 * so each loop is in sync with the audio from its first frame. Leave empty to
	 * skip. Note: the clip must not carry morph-target curves, otherwise it will
	 * fight the lip sync for the mouth channels.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Animation")
	TObjectPtr<UAnimSequence> BodyAnimation;

	/** Play rate for BodyAnimation. 1 = as authored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Animation", meta = (ClampMin = "0.1", ClampMax = "4"))
	float BodyAnimationRate = 1.0f;

	/** Restart BodyAnimation on every pass (off = let it run on its own). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Animation")
	bool bRestartAnimationOnPlayback = true;

	/** Log a few morph values every LogInterval seconds (verbose proof while tuning). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync")
	bool bLogValues = true;

	/** Seconds between debug log lines. Drop it to ~0.03 to trace the open/close envelope. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seven LipSync|Debug", meta = (ClampMin = "0.01", ClampMax = "5"))
	float LogInterval = 0.5f;

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
