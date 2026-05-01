#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlanetCharacterMovementComponent.generated.h"

class APlanetActor;

UCLASS()
class PRINCE1_API UPlanetCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPlanetCharacterMovementComponent();

	void SetPlanet(APlanetActor* InPlanet);

	APlanetActor* GetPlanet() const { return Planet.Get(); }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Override floor walkability to use the planet surface normal at the hit point.
	 * This keeps sphere surface contacts stable while gravity direction changes every frame.
	 */
	virtual bool IsWalkable(const FHitResult& Hit) const override;

	/** Accepts planet-surface landings that the capsule edge checks can reject while moving in air. */
	virtual bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const override;

	/** Requests a floor retry for planet hits so falling movement can recover from edge-style contacts. */
	virtual bool ShouldCheckForValidLandingSpot(float DeltaTime, const FVector& Delta, const FHitResult& Hit) const override;

	/** Adds a radial planet-floor fallback when the standard CMC floor sweep fails on the curved surface. */
	virtual void FindFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult,
		bool bCanUseCachedLocation, const FHitResult* DownwardSweepResult = nullptr) const override;

	/** Ensures landing velocity is projected onto the planet surface plane. */
	virtual void ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) override;

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<APlanetActor> Planet;

private:
	bool GetPlanetUpAtLocation(const FVector& Location, FVector& OutUp) const;
	bool IsPlanetHit(const FHitResult& Hit) const;
	bool FindPlanetFloor(const FVector& CapsuleLocation, FFindFloorResult& OutFloorResult) const;
	void ProjectVelocityToPlanetSurface();
};
