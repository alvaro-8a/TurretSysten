// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CppTurret.generated.h"

UCLASS()
class TURRET_API ACppTurret : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACppTurret();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* TurretMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Beam;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Target1;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Target2;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BeamTarget;

	UPROPERTY(EditDefaultsOnly, Category = Shooting)
	UParticleSystemComponent* P_MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = Shooting)
	USoundBase* ShootSound;

	UPROPERTY()
	FTimerHandle ChangeTargetTimerHandle;

	UPROPERTY()
	FTimerHandle TraceTimerHandle;

	UPROPERTY()
	FTimerHandle ShootTimerHandle;

	// Rotation
	int TimerCount = 0;
	FRotator CurrentLookRotation;
	FRotator TargetLookRotation;
	FRotator LookRotationDelta;

	UPROPERTY(EditAnywhere)
	float ChangeTargetDelay = 5.f;

	UPROPERTY(EditAnywhere)
	float RotationRateMultiplier = 1.f;

	UPROPERTY()
	AActor* Enemy = nullptr;

	UFUNCTION()
	void UpdateLookAtTarget(float DeltaTime);

	UFUNCTION()
	void ChangeBeamTarget();

	UFUNCTION(BlueprintCallable)
	void SetBeamLength(float Length);

	UFUNCTION()
	void TraceBeam();

	UFUNCTION()
	void CheckEnemy(AActor* HitActor);

	UFUNCTION()
	void FollowEnemy(float DeltaTime);

	UFUNCTION()
	void Shoot();

protected:
	// Beam length
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BeamLength = 1000.f;


};
