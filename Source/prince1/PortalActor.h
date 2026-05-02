#pragma once

#include "CoreMinimal.h"
#include "PlanetSurfaceActor.h"
#include "Interactable.h"
#include "PortalActor.generated.h"

class APlanetActor;

/**
 * Surface-aligned portal that teleports the interacting pawn to another planet.
 *
 * Designer workflow:
 *  - Drop two BP_Portal instances, one on each planet (auto-aligns to surface).
 *  - Set TargetPlanet to the destination planet.
 *  - Optionally set ArrivalAnchor to the partner portal on the other planet so
 *    the player arrives standing next to it; otherwise the player materialises
 *    at the world-up "north pole" of TargetPlanet.
 *
 * Runtime flow on E-key press:
 *   FadeOut -> (timer FadeDuration) -> Teleport + SwapPlanet -> FadeIn
 */
UCLASS()
class PRINCE1_API APortalActor : public APlanetSurfaceActor, public IInteractable
{
	GENERATED_BODY()

public:
	APortalActor();

	// IInteractable
	virtual bool  OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractPrompt_Implementation() const override;
	virtual bool  CanInteract_Implementation(const AActor* Interactor) const override;

protected:
	/** Destination planet. Required — interaction is gated on this being valid. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	TObjectPtr<APlanetActor> TargetPlanet;

	/**
	 * Optional landing reference on the destination planet (typically the partner
	 * APortalActor on the other side). When set, the player arrives just above
	 * this anchor with capsule Z aligned to that planet's surface normal and
	 * facing the anchor's projected forward.
	 *
	 * When null, the player arrives at the world-up "north pole" of TargetPlanet
	 * with identity rotation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	TObjectPtr<AActor> ArrivalAnchor;

	/** Vertical offset above the anchor / surface (cm). Compensates for capsule half-height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal", meta = (ClampMin = "0.0"))
	float ArrivalHeightOffset = 100.f;

	/** When true, fades the screen to black before/after the teleport. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	bool bUseFadeTransition = true;

	/** Duration (seconds) of each fade segment (out, then in). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal", meta = (ClampMin = "0.0"))
	float FadeDuration = 0.5f;

	/** Localised prompt shown above the portal when the player is in range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
	FText PromptText;

private:
	/** Starts the fade-out and schedules FinishTeleport(). */
	void BeginTeleport(APawn* Traveller);

	/** Performs the actual SetActorLocationAndRotation, planet swap, and fade-in. */
	void FinishTeleport();

	FVector ComputeArrivalLocation() const;
	FQuat   ComputeArrivalRotation() const;

	/** Pending traveller for the timer callback (avoids capturing in lambdas). */
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> PendingTraveller;

	FTimerHandle TeleportTimerHandle;
};
