#include "SevenLipSyncComponent.h"

#include "RealisticMetaHumanLipSyncGenerator.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Sound/SoundWaveProcedural.h"

namespace SevenLipSyncPrivate
{
	constexpr int32 TargetSampleRate = 16000;
	constexpr int32 ModelChunkSize = 320; // 20 ms @ 16 kHz

	/** One control -> one ARKit morph target. Scale lets several controls feed one morph. */
	struct FMorphMapping
	{
		const TCHAR* Control;
		const TCHAR* Morph;
		float Scale;
	};

	// Control names are the keys returned by
	// URealisticMetaHumanLipSyncGenerator::GetControlValues().
	const FMorphMapping GMorphMappings[] =
	{
		// ---- jaw ----
		// The model's jaw output is fairly conservative (~0.1-0.25 during speech),
		// the ARKit jawOpen morph expects 0..1 for a full open, hence the boost.
		{ TEXT("CTRL_C_jaw.ty"),                    TEXT("jawOpen"),                2.20f },
		{ TEXT("CTRL_C_jaw_fwdBack.ty"),            TEXT("jawForward"),             1.00f },
		{ TEXT("CTRL_C_jaw.tx"),                    TEXT("jawRight"),               1.00f },

		// ---- lips / corners ----
		{ TEXT("CTRL_L_mouth_cornerPull.ty"),       TEXT("mouthSmileLeft"),         1.00f },
		{ TEXT("CTRL_R_mouth_cornerPull.ty"),       TEXT("mouthSmileRight"),        1.00f },
		{ TEXT("CTRL_L_mouth_cornerDepress.ty"),    TEXT("mouthFrownLeft"),         1.00f },
		{ TEXT("CTRL_R_mouth_cornerDepress.ty"),    TEXT("mouthFrownRight"),        1.00f },
		{ TEXT("CTRL_L_mouth_stretch.ty"),          TEXT("mouthStretchLeft"),       1.00f },
		{ TEXT("CTRL_R_mouth_stretch.ty"),          TEXT("mouthStretchRight"),      1.00f },
		{ TEXT("CTRL_L_mouth_upperLipRaise.ty"),    TEXT("mouthUpperUpLeft"),       1.00f },
		{ TEXT("CTRL_R_mouth_upperLipRaise.ty"),    TEXT("mouthUpperUpRight"),      1.00f },
		{ TEXT("CTRL_L_mouth_lowerLipDepress.ty"),  TEXT("mouthLowerDownLeft"),     1.00f },
		{ TEXT("CTRL_R_mouth_lowerLipDepress.ty"),  TEXT("mouthLowerDownRight"),    1.00f },
		{ TEXT("CTRL_L_mouth_dimple.ty"),           TEXT("mouthDimpleLeft"),        1.00f },
		{ TEXT("CTRL_R_mouth_dimple.ty"),           TEXT("mouthDimpleRight"),       1.00f },
		{ TEXT("CTRL_L_mouth_sharpCornerPull.ty"),  TEXT("mouthSmileLeft"),         0.35f },
		{ TEXT("CTRL_R_mouth_sharpCornerPull.ty"),  TEXT("mouthSmileRight"),        0.35f },

		// funnel: upper + lower corners
		{ TEXT("CTRL_L_mouth_funnelU.ty"),          TEXT("mouthFunnel"),            0.50f },
		{ TEXT("CTRL_L_mouth_funnelD.ty"),          TEXT("mouthFunnel"),            0.50f },

		// pucker
		{ TEXT("CTRL_L_mouth_purseU.ty"),           TEXT("mouthPucker"),            0.50f },
		{ TEXT("CTRL_L_mouth_purseD.ty"),           TEXT("mouthPucker"),            0.50f },
		{ TEXT("CTRL_L_mouth_towardsU.ty"),         TEXT("mouthPucker"),            0.25f },
		{ TEXT("CTRL_L_mouth_towardsD.ty"),         TEXT("mouthPucker"),            0.25f },

		// lips together -> mouth close / press
		{ TEXT("CTRL_L_mouth_lipsTogetherU.ty"),    TEXT("mouthClose"),             0.50f },
		{ TEXT("CTRL_L_mouth_lipsTogetherD.ty"),    TEXT("mouthClose"),             0.50f },
		{ TEXT("CTRL_L_mouth_lipsTogetherU.ty"),    TEXT("mouthPressLeft"),         0.30f },
		{ TEXT("CTRL_R_mouth_lipsTogetherU.ty"),    TEXT("mouthPressRight"),        0.30f },
		{ TEXT("CTRL_L_mouth_cornerSharpnessU.ty"), TEXT("mouthPressLeft"),         0.15f },
		{ TEXT("CTRL_R_mouth_cornerSharpnessU.ty"), TEXT("mouthPressRight"),        0.15f },

		// cheek / nose / tongue
		{ TEXT("CTRL_L_nose_nasolabialDeepen.ty"),  TEXT("noseSneerLeft"),          0.40f },
		{ TEXT("CTRL_R_nose_nasolabialDeepen.ty"),  TEXT("noseSneerRight"),         0.40f },
		{ TEXT("CTRL_C_tongue_inOut.ty"),           TEXT("tongueOut"),              1.00f },

		// ---- brows (only when bMouthOnly == false) ----
		{ TEXT("CTRL_L_brow_down.ty"),              TEXT("browDownLeft"),           1.00f },
		{ TEXT("CTRL_R_brow_down.ty"),              TEXT("browDownRight"),          1.00f },
		{ TEXT("CTRL_L_brow_raiseIn.ty"),           TEXT("browInnerUp"),            0.50f },
		{ TEXT("CTRL_R_brow_raiseIn.ty"),           TEXT("browInnerUp"),            0.50f },
		{ TEXT("CTRL_L_brow_raiseOut.ty"),          TEXT("browOuterUpLeft"),        1.00f },
		{ TEXT("CTRL_R_brow_raiseOut.ty"),          TEXT("browOuterUpRight"),       1.00f },

		// ---- eyes (only when bMouthOnly == false) ----
		{ TEXT("CTRL_L_eye_blink.ty"),              TEXT("eyeBlinkLeft"),           1.00f },
		{ TEXT("CTRL_R_eye_blink.ty"),              TEXT("eyeBlinkRight"),          1.00f },
		{ TEXT("CTRL_L_eye_squintInner.ty"),        TEXT("eyeSquintLeft"),          1.00f },
		{ TEXT("CTRL_R_eye_squintInner.ty"),        TEXT("eyeSquintRight"),         1.00f },
		{ TEXT("CTRL_L_eye_cheekRaise.ty"),         TEXT("cheekSquintLeft"),        1.00f },
		{ TEXT("CTRL_R_eye_cheekRaise.ty"),         TEXT("cheekSquintRight"),       1.00f },
	};

	float ReadControl(const TMap<FString, float>& Values, const TCHAR* Name)
	{
		if (const float* Found = Values.Find(FString(Name)))
		{
			return *Found;
		}
		return 0.0f;
	}

	bool IsMouthMorph(const FString& MorphName)
	{
		return MorphName.StartsWith(TEXT("mouth"))
			|| MorphName.StartsWith(TEXT("jaw"))
			|| MorphName.StartsWith(TEXT("tongue"))
			|| MorphName.StartsWith(TEXT("cheek"));
	}

	/** Minimal RIFF/WAVE reader: PCM 8/16/24/32 bit and IEEE float 32, mono or stereo, resampled to 16 kHz mono. */
	bool LoadWavAsMono16k(const FString& AbsolutePath, TArray<float>& OutSamples, FString& OutError)
	{
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *AbsolutePath))
		{
			OutError = FString::Printf(TEXT("cannot read '%s'"), *AbsolutePath);
			return false;
		}

		auto ReadU16 = [&Bytes](int32 Offset) -> uint16
		{
			if (Offset < 0 || Offset + 2 > Bytes.Num()) { return 0; }
			return static_cast<uint16>(Bytes[Offset]) | (static_cast<uint16>(Bytes[Offset + 1]) << 8);
		};
		auto ReadU32 = [&Bytes](int32 Offset) -> uint32
		{
			if (Offset < 0 || Offset + 4 > Bytes.Num()) { return 0; }
			return static_cast<uint32>(Bytes[Offset])
				| (static_cast<uint32>(Bytes[Offset + 1]) << 8)
				| (static_cast<uint32>(Bytes[Offset + 2]) << 16)
				| (static_cast<uint32>(Bytes[Offset + 3]) << 24);
		};
		auto ReadTag = [&Bytes](int32 Offset) -> FString
		{
			if (Offset < 0 || Offset + 4 > Bytes.Num()) { return FString(); }
			return FString(4, reinterpret_cast<const ANSICHAR*>(Bytes.GetData() + Offset));
		};

		if (Bytes.Num() < 44 || ReadTag(0) != TEXT("RIFF") || ReadTag(8) != TEXT("WAVE"))
		{
			OutError = TEXT("not a RIFF/WAVE file");
			return false;
		}

		uint16 FormatTag = 0;
		uint16 Channels = 0;
		uint16 BitsPerSample = 0;
		uint32 SourceRate = 0;
		int32 DataOffset = INDEX_NONE;
		int32 DataSize = 0;

		int32 Pos = 12;
		while (Pos + 8 <= Bytes.Num())
		{
			const FString ChunkId = ReadTag(Pos);
			const uint32 ChunkSize = ReadU32(Pos + 4);
			const int32 Payload = Pos + 8;

			if (ChunkId == TEXT("fmt "))
			{
				FormatTag = ReadU16(Payload);
				Channels = ReadU16(Payload + 2);
				SourceRate = ReadU32(Payload + 4);
				BitsPerSample = ReadU16(Payload + 14);
				if (FormatTag == 0xFFFE && Payload + 26 <= Bytes.Num())
				{
					// WAVE_FORMAT_EXTENSIBLE: real format tag sits in the sub-format GUID.
					FormatTag = ReadU16(Payload + 24);
				}
			}
			else if (ChunkId == TEXT("data"))
			{
				DataOffset = Payload;
				DataSize = static_cast<int32>(FMath::Min<uint32>(ChunkSize, static_cast<uint32>(Bytes.Num() - Payload)));
			}

			Pos = Payload + static_cast<int32>(ChunkSize) + static_cast<int32>(ChunkSize & 1u);
		}

		if (DataOffset == INDEX_NONE || Channels == 0 || SourceRate == 0 || BitsPerSample == 0)
		{
			OutError = TEXT("missing fmt/data chunk");
			return false;
		}
		if (FormatTag != 1 && FormatTag != 3)
		{
			OutError = FString::Printf(TEXT("unsupported wav format tag %d"), FormatTag);
			return false;
		}

		const int32 BytesPerSample = BitsPerSample / 8;
		const int32 FrameSize = BytesPerSample * Channels;
		if (FrameSize <= 0)
		{
			OutError = TEXT("invalid frame size");
			return false;
		}

		const int32 NumFrames = DataSize / FrameSize;
		if (NumFrames <= 0)
		{
			OutError = TEXT("empty audio data");
			return false;
		}

		// Decode + downmix to mono float.
		TArray<float> Mono;
		Mono.SetNumUninitialized(NumFrames);
		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const int32 FrameOffset = DataOffset + Frame * FrameSize;
			float Sum = 0.0f;
			for (int32 Channel = 0; Channel < Channels; ++Channel)
			{
				const int32 Offset = FrameOffset + Channel * BytesPerSample;
				float Value = 0.0f;
				if (FormatTag == 3)
				{
					float Raw = 0.0f;
					FMemory::Memcpy(&Raw, Bytes.GetData() + Offset, sizeof(float));
					Value = Raw;
				}
				else if (BitsPerSample == 8)
				{
					Value = (static_cast<float>(Bytes[Offset]) - 128.0f) / 128.0f;
				}
				else if (BitsPerSample == 16)
				{
					Value = static_cast<float>(static_cast<int16>(ReadU16(Offset))) / 32768.0f;
				}
				else if (BitsPerSample == 24)
				{
					int32 Raw = static_cast<int32>(Bytes[Offset])
						| (static_cast<int32>(Bytes[Offset + 1]) << 8)
						| (static_cast<int32>(Bytes[Offset + 2]) << 16);
					if (Raw & 0x800000) { Raw |= ~0xFFFFFF; }
					Value = static_cast<float>(Raw) / 8388608.0f;
				}
				else if (BitsPerSample == 32)
				{
					Value = static_cast<float>(static_cast<int32>(ReadU32(Offset))) / 2147483648.0f;
				}
				Sum += Value;
			}
			Mono[Frame] = FMath::Clamp(Sum / static_cast<float>(Channels), -1.0f, 1.0f);
		}

		if (static_cast<int32>(SourceRate) == TargetSampleRate)
		{
			OutSamples = MoveTemp(Mono);
			return true;
		}

		// Linear resample to 16 kHz.
		const double Ratio = static_cast<double>(SourceRate) / static_cast<double>(TargetSampleRate);
		const int32 OutNum = FMath::Max(1, FMath::FloorToInt(NumFrames / Ratio));
		OutSamples.SetNumUninitialized(OutNum);
		for (int32 Index = 0; Index < OutNum; ++Index)
		{
			const double SrcPos = Index * Ratio;
			const int32 SrcIndex = FMath::Clamp(FMath::FloorToInt(SrcPos), 0, NumFrames - 1);
			const int32 NextIndex = FMath::Min(SrcIndex + 1, NumFrames - 1);
			const float Alpha = static_cast<float>(SrcPos - SrcIndex);
			OutSamples[Index] = FMath::Lerp(Mono[SrcIndex], Mono[NextIndex], Alpha);
		}
		return true;
	}
}

USevenLipSyncComponent::USevenLipSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USevenLipSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!TargetMesh)
	{
		if (AActor* Owner = GetOwner())
		{
			TargetMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		}
	}

	EnsureGenerator();

	if (bAutoStart && !WavFile.IsEmpty())
	{
		StartPlayback(WavFile);
	}
}

void USevenLipSyncComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bPlaying = false;
	StopAudio();
	Generator = nullptr;
	Super::EndPlay(EndPlayReason);
}

void USevenLipSyncComponent::EnsureGenerator()
{
	if (Generator)
	{
		return;
	}

	FRealisticMetaHumanLipSyncConfig Config;
	Config.ModelType = ERealisticMetaHumanLipSyncModelType::SemiOptimized;
	Generator = URealisticMetaHumanLipSyncGenerator::CreateRealisticMetaHumanLipSyncGenerator(Config);

	UE_LOG(LogTemp, Log, TEXT("SevenLipSync: generator created = %s"),
		Generator ? TEXT("true") : TEXT("false"));
}

void USevenLipSyncComponent::StartPlayback(const FString& InWavPath)
{
	EnsureGenerator();
	if (!Generator)
	{
		UE_LOG(LogTemp, Error, TEXT("SevenLipSync: no generator, aborting playback"));
		return;
	}

	FString AbsolutePath = InWavPath;
	if (FPaths::IsRelative(AbsolutePath))
	{
		AbsolutePath = FPaths::Combine(FPaths::ProjectDir(), AbsolutePath);
	}
	AbsolutePath = FPaths::ConvertRelativePathToFull(AbsolutePath);

	FString Error;
	AudioSamples.Reset();
	if (!SevenLipSyncPrivate::LoadWavAsMono16k(AbsolutePath, AudioSamples, Error))
	{
		UE_LOG(LogTemp, Error, TEXT("SevenLipSync: failed to load '%s': %s"), *AbsolutePath, *Error);
		return;
	}

	ReadCursor = 0;
	SampleAccumulator = 0.0;

	// First playback of the session: the ONNX model is still loading for ~1 s and
	// drops frames, which would desync the very first pass. Wait it out once.
	if (!bWarmedUp)
	{
		bWarmedUp = true;
		WarmupRemaining = 1.2f;
		bPlaying = false;
		UE_LOG(LogTemp, Log, TEXT("SevenLipSync: warming up the model before playback"));
		return;
	}

	StartAudio(AudioSamples);
	bPlaying = true;

	UE_LOG(LogTemp, Log, TEXT("SevenLipSync: playing '%s' (%.2f s @16k mono)"),
		*AbsolutePath, AudioSamples.Num() / 16000.0f);
}

void USevenLipSyncComponent::StopAudio()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
	if (ProceduralWave)
	{
		ProceduralWave->ResetAudio();
	}
}

void USevenLipSyncComponent::StartAudio(const TArray<float>& Samples)
{
	if (Samples.Num() == 0)
	{
		return;
	}

	if (!AudioComponent)
	{
		AudioComponent = NewObject<UAudioComponent>(this);
		AudioComponent->bAutoActivate = false;
		AudioComponent->bIsUISound = true;          // 2D, no attenuation
		AudioComponent->bAllowSpatialization = false;
		if (AActor* Owner = GetOwner())
		{
			if (USceneComponent* Root = Owner->GetRootComponent())
			{
				AudioComponent->SetupAttachment(Root);
			}
		}
		AudioComponent->RegisterComponent();
	}

	AudioComponent->Stop();

	// A fresh wave per playback: ResetAudio() is deferred to the audio thread and
	// races with QueueAudio(), which makes AvailableByteCount unreliable across loops.
	ProceduralWave = NewObject<USoundWaveProcedural>(this);
	ProceduralWave->SetSampleRate(SevenLipSyncPrivate::TargetSampleRate);
	ProceduralWave->NumChannels = 1;
	ProceduralWave->bLooping = false;

	const int32 NumSamples = Samples.Num();
	TArray<uint8> Pcm;
	Pcm.SetNumUninitialized(NumSamples * static_cast<int32>(sizeof(int16)));
	int16* Out = reinterpret_cast<int16*>(Pcm.GetData());
	for (int32 Index = 0; Index < NumSamples; ++Index)
	{
		Out[Index] = static_cast<int16>(FMath::Clamp(Samples[Index], -1.0f, 1.0f) * 32767.0f);
	}

	QueuedBytes = Pcm.Num();
	ProceduralWave->Duration = static_cast<float>(NumSamples) / static_cast<float>(SevenLipSyncPrivate::TargetSampleRate);
	ProceduralWave->QueueAudio(Pcm.GetData(), Pcm.Num());

	AudioComponent->SetSound(ProceduralWave);
	AudioComponent->Play();

	UE_LOG(LogTemp, Log, TEXT("SevenLipSync: audio queued %.2fs (%d bytes), playing=%s"),
		ProceduralWave->Duration, QueuedBytes, AudioComponent->IsPlaying() ? TEXT("true") : TEXT("false"));
}

void USevenLipSyncComponent::StopPlayback()
{
	bPlaying = false;
	StopAudio();

	if (Generator)
	{
		// Push everything to silence so the face relaxes instead of freezing.
		TMap<FString, float> Zeroed;
		for (const FString& Name : URealisticMetaHumanLipSyncGenerator::GetControlNames())
		{
			Zeroed.Add(Name, 0.0f);
		}
		Generator->SetControlValues(Zeroed);
	}
}

void USevenLipSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Generator || !TargetMesh)
	{
		return;
	}

	if (WarmupRemaining > 0.0f)
	{
		WarmupRemaining -= DeltaTime;
		if (WarmupRemaining <= 0.0f)
		{
			StartPlayback(WavFile);
		}
		return;
	}

	if (LoopGapRemaining > 0.0f)
	{
		LoopGapRemaining -= DeltaTime;
		if (LoopGapRemaining <= 0.0f)
		{
			StartPlayback(WavFile);
		}
		// No early return: UpdateMorphTargets still runs below so the mouth can
		// actually relax to zero during the silence between passes.
	}

	if (bPlaying)
	{
		FeedAudio(DeltaTime);
	}

	UpdateMorphTargets(DeltaTime);
}

void USevenLipSyncComponent::FeedAudio(float DeltaTime)
{
	// Master clock = the audio device, not the frame delta. Odd sizes and frame
	// hitches make a wall-clock cursor drift away from what is actually audible,
	// which shows up as lips moving after the sound has stopped.
	int32 PlayedSamples = -1;
	if (ProceduralWave && QueuedBytes > 0)
	{
		// Bytes still sitting in the wave's queue; what is left of QueuedBytes has
		// already been handed to the mixer, i.e. is what you are hearing now.
		const int32 AvailableBytes = ProceduralWave->GetAvailableAudioByteCount();
		if (AvailableBytes >= 0 && AvailableBytes <= QueuedBytes)
		{
			PlayedSamples = (QueuedBytes - AvailableBytes) / static_cast<int32>(sizeof(int16));
		}
	}

	if (PlayedSamples < 0)
	{
		// No usable audio clock (no device / not playing yet): fall back to frame time.
		SampleAccumulator += static_cast<double>(DeltaTime) * SevenLipSyncPrivate::TargetSampleRate;
		PlayedSamples = static_cast<int32>(SampleAccumulator);
	}

	const int32 LookaheadSamples = FMath::RoundToInt(
		LipSyncLookaheadMs * SevenLipSyncPrivate::TargetSampleRate / 1000.0f);

	const int32 FeedTarget = FMath::Clamp(PlayedSamples + LookaheadSamples, 0, AudioSamples.Num());

	DebugPlayedSeconds = static_cast<float>(PlayedSamples) / SevenLipSyncPrivate::TargetSampleRate;

	// Feed whole 20 ms chunks plus the final short tail.
	while (ReadCursor < FeedTarget)
	{
		const int32 NumToFeed = FMath::Min(SevenLipSyncPrivate::ModelChunkSize, FeedTarget - ReadCursor);
		TArray<float> Chunk;
		Chunk.Append(AudioSamples.GetData() + ReadCursor, NumToFeed);
		ReadCursor += NumToFeed;

		Generator->ProcessAudioData(MoveTemp(Chunk), SevenLipSyncPrivate::TargetSampleRate, 1);
	}

	// Loop/stop on the audio clock, not on the model cursor: with a negative
	// lookahead the cursor deliberately lags behind and would never reach the end.
	if (PlayedSamples >= AudioSamples.Num())
	{
		if (bLoopAudio && LoopGapSeconds > 0.0f)
		{
			// Close the mouth and wait a beat before the next pass.
			StopPlayback();
			LoopGapRemaining = LoopGapSeconds;
		}
		else if (bLoopAudio && AudioSamples.Num() > 0)
		{
			ReadCursor = 0;
			SampleAccumulator = 0.0;
			StartAudio(AudioSamples);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("SevenLipSync: playback finished"));
			StopPlayback();
		}
	}

	DebugCursorSeconds = static_cast<float>(ReadCursor) / SevenLipSyncPrivate::TargetSampleRate;
}

void USevenLipSyncComponent::UpdateMorphTargets(float DeltaTime)
{
	EnsureGenerator();
	if (!Generator || !TargetMesh)
	{
		return;
	}

	const TMap<FString, float> Controls = Generator->GetControlValues();

	if (!bLoggedControlNames && Controls.Num() > 0)
	{
		bLoggedControlNames = true;
		FString Keys;
		for (const TPair<FString, float>& Pair : Controls)
		{
			Keys += Pair.Key + TEXT(" ");
		}
		UE_LOG(LogTemp, Log, TEXT("SevenLipSync: %d control keys: %s"), Controls.Num(), *Keys);
	}

	// Accumulate every mapping into a target value per morph target.
	TMap<FName, float> Targets;
	for (const SevenLipSyncPrivate::FMorphMapping& Mapping : SevenLipSyncPrivate::GMorphMappings)
	{
		const FString MorphName(Mapping.Morph);
		if (bMouthOnly && !SevenLipSyncPrivate::IsMouthMorph(MorphName))
		{
			continue;
		}

		const float Value = SevenLipSyncPrivate::ReadControl(Controls, Mapping.Control) * Mapping.Scale * Gain;
		float& Accumulated = Targets.FindOrAdd(FName(*MorphName));
		Accumulated += Value;
	}

	// Between passes (loop gap, warm-up, stopped) nothing should move: the generator
	// can still hold the last values it computed, so force the pose closed here.
	if (!bPlaying)
	{
		Targets.Reset();
	}

	// Silence gate. The model happily invents mouth shapes for silence, breath and
	// low level noise, which shows up as lips moving before a line starts and after
	// it ends. Measure the ~100 ms of audio the model is looking at, and fade the
	// whole pose out when that window is essentially silent.
	float GateGain = bPlaying ? 1.0f : 0.0f;
	if (bPlaying && AudioSamples.Num() > 0 && GateHighRms > GateLowRms)
	{
		const int32 Center = FMath::Clamp(ReadCursor, 0, AudioSamples.Num() - 1);
		const int32 HalfWindow = 1920; // 120 ms at 16 kHz - wide enough to bridge short gaps
		const int32 StartIndex = FMath::Max(0, Center - HalfWindow);
		const int32 EndIndex = FMath::Min(AudioSamples.Num(), Center + HalfWindow);
		if (EndIndex > StartIndex)
		{
			double SumOfSquares = 0.0;
			for (int32 Index = StartIndex; Index < EndIndex; ++Index)
			{
				const double Sample = AudioSamples[Index];
				SumOfSquares += Sample * Sample;
			}
			const float Rms = static_cast<float>(FMath::Sqrt(SumOfSquares / static_cast<double>(EndIndex - StartIndex)));
			GateGain = FMath::Clamp((Rms - GateLowRms) / (GateHighRms - GateLowRms), 0.0f, 1.0f);
		}
	}

	// Smooth the gate so it never pops mid-word.
	SmoothedGate = FMath::FInterpTo(SmoothedGate, GateGain, DeltaTime, 12.0f);
	DebugGate = SmoothedGate;

	for (TPair<FName, float>& Pair : Targets)
	{
		Pair.Value = FMath::Clamp(Pair.Value, 0.0f, 1.0f) * SmoothedGate;
	}

	// Interpolate towards the target and write the morph target.
	for (const TPair<FName, float>& Pair : Targets)
	{
		float& Current = MorphValues.FindOrAdd(Pair.Key);
		Current = FMath::FInterpTo(Current, Pair.Value, DeltaTime, FMath::Max(1.0f, SmoothingSpeed));
		TargetMesh->SetMorphTarget(Pair.Key, Current);
	}

	// Relax morphs that we drove last frame but are no longer in the target set.
	for (auto It = MorphValues.CreateIterator(); It; ++It)
	{
		if (Targets.Contains(It.Key()))
		{
			continue;
		}
		It.Value() = FMath::FInterpTo(It.Value(), 0.0f, DeltaTime, FMath::Max(1.0f, SmoothingSpeed));
		TargetMesh->SetMorphTarget(It.Key(), It.Value());
		if (It.Value() < KINDA_SMALL_NUMBER)
		{
			It.RemoveCurrent();
		}
	}

	if (bLogValues)
	{
		DebugLogTimer += DeltaTime;
		if (DebugLogTimer >= 0.5f)
		{
			DebugLogTimer = 0.0f;
			const bool bAudioPlaying = AudioComponent && AudioComponent->IsPlaying();
			const int32 AvailableBytesNow = ProceduralWave ? ProceduralWave->GetAvailableAudioByteCount() : -1;
			UE_LOG(LogTemp, Log, TEXT("SevenLipSync: jawOpen=%.3f funnel=%.3f pucker=%.3f smileL=%.3f lowerDownL=%.3f | audio=%s availKB=%d played=%.2fs cursor=%.2fs gate=%.2f"),
				MorphValues.FindRef(FName(TEXT("jawOpen"))),
				MorphValues.FindRef(FName(TEXT("mouthFunnel"))),
				MorphValues.FindRef(FName(TEXT("mouthPucker"))),
				MorphValues.FindRef(FName(TEXT("mouthSmileLeft"))),
				MorphValues.FindRef(FName(TEXT("mouthLowerDownLeft"))),
				bAudioPlaying ? TEXT("on") : TEXT("off"),
				AvailableBytesNow / 1024,
				DebugPlayedSeconds,
				DebugCursorSeconds,
				DebugGate);
		}
	}
}
