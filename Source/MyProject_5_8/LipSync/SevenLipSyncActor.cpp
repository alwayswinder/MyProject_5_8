#include "SevenLipSyncActor.h"

#include "SevenLipSyncComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ASevenLipSyncActor::ASevenLipSyncActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	if (UCameraComponent* Camera = GetCameraComponent())
	{
		Camera->SetFieldOfView(CameraFOV);
		Camera->SetConstraintAspectRatio(false);
	}

	FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
	FaceMesh->SetupAttachment(RootComponent);
	FaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FaceMesh->SetCastShadow(false);
	FaceMesh->SetGenerateOverlapEvents(false);
	FaceMesh->SetRelativeLocation(MeshLocation);
	FaceMesh->SetRelativeRotation(MeshRotation);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SevenMeshFinder(TEXT("/Game/_MyTest/Mouth/Seven.Seven"));
	if (SevenMeshFinder.Succeeded())
	{
		FaceMesh->SetSkeletalMeshAsset(SevenMeshFinder.Object);
	}

	KeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(RootComponent);
	KeyLight->SetMobility(EComponentMobility::Movable);
	KeyLight->SetRelativeLocation(FVector(30.0f, 55.0f, 35.0f));
	KeyLight->SetIntensity(KeyLightIntensity);
	KeyLight->SetAttenuationRadius(900.0f);
	KeyLight->SetCastShadows(false);

	LipSync = CreateDefaultSubobject<USevenLipSyncComponent>(TEXT("LipSync"));
	LipSync->TargetMesh = FaceMesh;
}

void ASevenLipSyncActor::BeginPlay()
{
	// Apply the editable settings before component BeginPlay runs (the Super call
	// below dispatches it), otherwise the component starts with its own defaults.
	if (UCameraComponent* Camera = GetCameraComponent())
	{
		Camera->SetFieldOfView(CameraFOV);
	}
	if (FaceMesh)
	{
		FaceMesh->SetRelativeLocation(MeshLocation);
		FaceMesh->SetRelativeRotation(MeshRotation);
		FaceMesh->SetRelativeScale3D(FVector(MeshScale));
	}
	if (KeyLight)
	{
		KeyLight->SetIntensity(KeyLightIntensity);
	}
	if (LipSync)
	{
		LipSync->TargetMesh = FaceMesh;
		LipSync->WavFile = WavFile;
		LipSync->LipSyncLookaheadMs = LipSyncLookaheadMs;
		LipSync->Gain = LipSyncGain;
		LipSync->JawOpenScale = LipSyncJawOpenScale;
		LipSync->JawOpenMax = LipSyncJawOpenMax;
		LipSync->RoundingScale = LipSyncRoundingScale;
		LipSync->JawOpenFloor = LipSyncJawOpenFloor;
		LipSync->OpenSpeed = LipSyncOpenSpeed;
		LipSync->CloseSpeed = LipSyncCloseSpeed;
		LipSync->LipFollowJaw = LipSyncLipFollowJaw;
		LipSync->LogInterval = LipSyncLogInterval;
	}

	Super::BeginPlay();
}

void ASevenLipSyncActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Keep the sync knobs live: they can be dragged in the Details panel while PIE
	// is running, which is the practical way to dial out audio-device latency.
	if (LipSync)
	{
		LipSync->LipSyncLookaheadMs = LipSyncLookaheadMs;
		LipSync->Gain = LipSyncGain;
		LipSync->JawOpenScale = LipSyncJawOpenScale;
		LipSync->JawOpenMax = LipSyncJawOpenMax;
		LipSync->RoundingScale = LipSyncRoundingScale;
		LipSync->JawOpenFloor = LipSyncJawOpenFloor;
		LipSync->OpenSpeed = LipSyncOpenSpeed;
		LipSync->CloseSpeed = LipSyncCloseSpeed;
		LipSync->LipFollowJaw = LipSyncLipFollowJaw;
		LipSync->LogInterval = LipSyncLogInterval;
		LipSync->GateLowRms = LipSyncGateLowRms;
		LipSync->GateHighRms = LipSyncGateHighRms;
	}

	if (ViewTargetLockSeconds > 0.0f)
	{
		ViewTargetLockSeconds -= DeltaSeconds;

		// Keep re-asserting for a moment: the player controller may still be
		// assigning its own view target right after possession.
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				if (PlayerController->GetViewTarget() != this)
				{
					PlayerController->SetViewTarget(this);
				}
			}
		}
	}
}
