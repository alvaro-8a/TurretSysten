// Fill out your copyright notice in the Description page of Project Settings.


#include "CppTurret.h"

#include "Kismet/KismetMathLibrary.h"
#include "TurretAnimInterface.h"
#include "CharacterInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
//#include "Engine/World.h"

#define OUT

// Sets default values
ACppTurret::ACppTurret()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TurretMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(Root);

	Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
	Beam->SetupAttachment(TurretMesh, TEXT("BeamSocket"));

	Target1 = CreateDefaultSubobject<USceneComponent>(TEXT("Target1"));
	Target1->SetupAttachment(Root);

	Target2 = CreateDefaultSubobject<USceneComponent>(TEXT("Target2"));
	Target2->SetupAttachment(Root);

	BeamTarget = CreateDefaultSubobject<USceneComponent>(TEXT("BeamTarget"));
	BeamTarget->SetupAttachment(Root);

	P_MuzzleFlash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MuzzleFlash"));
	P_MuzzleFlash->SetupAttachment(TurretMesh, TEXT("BeamSocket"));
	P_MuzzleFlash->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void ACppTurret::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(ChangeTargetTimerHandle, this, &ACppTurret::ChangeBeamTarget, ChangeTargetDelay, true, 1.0f);
	GetWorldTimerManager().SetTimer(TraceTimerHandle, this, &ACppTurret::TraceBeam, .1f, true, .1f);
}

// Called every frame
void ACppTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Enemy)
	{
		FollowEnemy(DeltaTime);
	}
	else 
	{
		UpdateLookAtTarget(DeltaTime);
	}
	//TraceBeam();
}

void ACppTurret::UpdateLookAtTarget(float DeltaTime)
{
	if (CurrentLookRotation.Equals(TargetLookRotation, 1.f))
	{
		return;
	}

	CurrentLookRotation += LookRotationDelta * RotationRateMultiplier * DeltaTime;
	
	if (TurretMesh->GetAnimInstance()->Implements<UTurretAnimInterface>())
	{
		ITurretAnimInterface::Execute_UpdateLookAtRotation(TurretMesh->GetAnimInstance(), CurrentLookRotation);
	}
}

void ACppTurret::ChangeBeamTarget()
{
	TimerCount++;

	if (TimerCount % 2 == 0)
	{
		BeamTarget->SetWorldLocation(Target1->GetComponentLocation());
	}
	else
	{
		BeamTarget->SetWorldLocation(Target2->GetComponentLocation());
	}

	FVector Start = TurretMesh->GetSocketLocation("BeamSocket");
	FVector End = BeamTarget->GetComponentLocation();

	TargetLookRotation = UKismetMathLibrary::FindLookAtRotation(Start, End);

	LookRotationDelta = TargetLookRotation - CurrentLookRotation;
	LookRotationDelta.Normalize();
}

void ACppTurret::SetBeamLength(float Length)
{
	// Set scale relative to the Length and adjust offset
	// Scale = L / 200; Offset = (L / 400) * -50
	// Length = 1000; Scale = 5; Offset = -250
	Beam->SetRelativeScale3D(FVector(Length / 200.f, Beam->GetRelativeScale3D().Y, Beam->GetRelativeScale3D().Z));
	Beam->SetRelativeLocation(FVector((Length / 200.f) * -50.f, Beam->GetRelativeLocation().Y, Beam->GetRelativeLocation().Z));
}

void ACppTurret::TraceBeam()
{
	FHitResult HitResult;
	FVector Start = TurretMesh->GetSocketLocation("BeamSocket");
	FVector End = Start + Beam->GetForwardVector() * BeamLength;

	FCollisionQueryParams CollQueryParams;
	CollQueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel
	(
		OUT HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Camera,
		CollQueryParams
	);

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Hit: %s"), (bHit ? TEXT("true") : TEXT("false"))));
	//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), (bHit ? TEXT("true") : TEXT("false")));

	if (bHit)
	{
		SetBeamLength(HitResult.Distance);
		CheckEnemy(HitResult.GetActor());
	}
	else 
	{
		SetBeamLength(BeamLength);
	}
}

void ACppTurret::CheckEnemy(AActor* HitActor)
{
	if (HitActor->Implements<UCharacterInterface>())
	{
		bool bEnemy = ICharacterInterface::Execute_IsEnemy(HitActor);

		if (bEnemy)
		{
			if (!Enemy)
			{
				Enemy = HitActor;
				// Start shooting
				GetWorldTimerManager().SetTimer(ShootTimerHandle, this, &ACppTurret::Shoot, .5f, true, .5f);
				UE_LOG(LogTemp, Warning, TEXT("Enemy Detected!"));
			}
		}
	}
	else
	{
		Enemy = nullptr;
		// Stop shooting
		GetWorldTimerManager().ClearTimer(ShootTimerHandle);
	}
}

void ACppTurret::FollowEnemy(float DeltaTime)
{

	FVector Start = TurretMesh->GetSocketLocation("BeamSocket");
	FVector End = Enemy->GetActorLocation();

	FRotator RotationToEnemy = UKismetMathLibrary::FindLookAtRotation(Start, End);

	CurrentLookRotation = FMath::RInterpTo(CurrentLookRotation, RotationToEnemy, DeltaTime, 100);

	if (TurretMesh->GetAnimInstance()->Implements<UTurretAnimInterface>())
	{
		ITurretAnimInterface::Execute_UpdateLookAtRotation(TurretMesh->GetAnimInstance(), CurrentLookRotation);
	}
}

void ACppTurret::Shoot()
{
	UGameplayStatics::PlaySoundAtLocation(this, ShootSound, P_MuzzleFlash->GetComponentLocation());

	P_MuzzleFlash->Activate(true);

	FHitResult HitResult;
	FVector Start = TurretMesh->GetSocketLocation("BeamSocket");
	FVector End = Start + Beam->GetForwardVector() * BeamLength;

	FCollisionQueryParams CollQueryParams;
	CollQueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel
	(
		OUT HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Camera,
		CollQueryParams
	);

	if (bHit)
	{
		FPointDamageEvent DamageEvent(10.f, HitResult, Beam->GetForwardVector(), nullptr);
		HitResult.GetActor()->TakeDamage(10.f, DamageEvent, GetInstigatorController(), this);
	}
}
