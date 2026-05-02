#include "PortalActor.h"

#include "PlanetActor.h"
#include "PrinceCharacter.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "Portal"

APortalActor::APortalActor()
{
	// The portal mesh is decorative; the player walks up to it, not through it.
	// Use Block so the capsule doesn't clip into the visual.
	if (Mesh)
	{
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	}

	PromptText = LOCTEXT("PortalPromptDefault", "Board");
}

// ─── IInteractable ──────────────────────────────────────────────────────────────

bool APortalActor::CanInteract_Implementation(const AActor* Interactor) const
{
	// Need a destination and an active traveller; reject if a teleport is already pending.
	return TargetPlanet != nullptr
		&& Interactor != nullptr
		&& !PendingTraveller.IsValid();
}

FText APortalActor::GetInteractPrompt_Implementation() const
{
	return PromptText;
}

bool APortalActor::OnInteract_Implementation(AActor* Interactor)
{
	APawn* Pawn = Cast<APawn>(Interactor);
	if (!Pawn || !TargetPlanet)
	{
		return false;
	}

	BeginTeleport(Pawn);
	return true;
}

// ─── Teleport pipeline ─────────────────────────────────────────────────────────

void APortalActor::BeginTeleport(APawn* Traveller)
{
	if (!Traveller)
	{
		return;
	}

	PendingTraveller = Traveller;

	// Optional fade-out via the player camera manager. hold=true so the screen
	// stays black after the fade reaches alpha=1, until the inbound fade starts.
	if (bUseFadeTransition)
	{
		if (APlayerController* PC = Cast<APlayerController>(Traveller->GetController()))
		{
			if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
			{
				PCM->StartCameraFade(/*FromAlpha=*/0.f, /*ToAlpha=*/1.f,
					FadeDuration, FLinearColor::Black,
					/*bShouldFadeAudio=*/false, /*bHoldWhenFinished=*/true);
			}
		}
	}

	// Use a tiny non-zero delay even when fading is disabled, so the timer
	// callback runs cleanly outside the input dispatch frame.
	const float Delay = bUseFadeTransition ? FMath::Max(FadeDuration, KINDA_SMALL_NUMBER) : 0.01f;

	GetWorldTimerManager().SetTimer(
		TeleportTimerHandle, this, &APortalActor::FinishTeleport, Delay, /*bLoop=*/false);
}

void APortalActor::FinishTeleport()
{
	APawn* Traveller = PendingTraveller.Get();
	PendingTraveller = nullptr;

	if (!Traveller || !TargetPlanet)
	{
		return;
	}

	const FVector ArrivalLoc = ComputeArrivalLocation();
	const FQuat   ArrivalRot = ComputeArrivalRotation();

	// TeleportPhysics so the capsule's transform is moved without sweeping —
	// the destination is far away, and a sweep would collide with the planet.
	Traveller->SetActorLocationAndRotation(
		ArrivalLoc, ArrivalRot,
		/*bSweep=*/false, /*OutSweepHitResult=*/nullptr,
		ETeleportType::TeleportPhysics);

	// Stale velocity from before the jump would launch the character along the
	// new planet's tangent (or worse, into space). Wipe it.
	if (UCharacterMovementComponent* MoveComp = Traveller->FindComponentByClass<UCharacterMovementComponent>())
	{
		MoveComp->Velocity = FVector::ZeroVector;
	}

	// Swap planets — this updates the CMC's gravity direction and the
	// character's CameraForwardFlat so the next tick sees the new orientation.
	if (APrinceCharacter* PrinceChar = Cast<APrinceCharacter>(Traveller))
	{
		PrinceChar->SetCurrentPlanet(TargetPlanet);
	}

	// Fade back in. fromAlpha=1 starts at fully black (matching the held state
	// from the outbound fade) and fades to 0.
	if (bUseFadeTransition)
	{
		if (APlayerController* PC = Cast<APlayerController>(Traveller->GetController()))
		{
			if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
			{
				PCM->StartCameraFade(/*FromAlpha=*/1.f, /*ToAlpha=*/0.f,
					FadeDuration, FLinearColor::Black,
					/*bShouldFadeAudio=*/false, /*bHoldWhenFinished=*/false);
			}
		}
	}
}

// ─── Arrival math ──────────────────────────────────────────────────────────────

FVector APortalActor::ComputeArrivalLocation() const
{
	if (!TargetPlanet)
	{
		return FVector::ZeroVector;
	}

	if (ArrivalAnchor)
	{
		const FVector AnchorLoc = ArrivalAnchor->GetActorLocation();
		const FVector PlanetUp  = (AnchorLoc - TargetPlanet->GetCenter()).GetSafeNormal();
		// Lift by ArrivalHeightOffset so the capsule center clears the surface.
		return AnchorLoc + PlanetUp * ArrivalHeightOffset;
	}

	// No anchor — arrive at the world-+Z "north pole" of the destination planet.
	return TargetPlanet->GetCenter()
		+ FVector::UpVector * (TargetPlanet->GetRadius() + ArrivalHeightOffset);
}

FQuat APortalActor::ComputeArrivalRotation() const
{
	if (!TargetPlanet || !ArrivalAnchor)
	{
		return FQuat::Identity;
	}

	const FVector AnchorLoc = ArrivalAnchor->GetActorLocation();
	const FVector PlanetUp  = (AnchorLoc - TargetPlanet->GetCenter()).GetSafeNormal();

	if (PlanetUp.IsNearlyZero())
	{
		return FQuat::Identity;
	}

	// Project the anchor's forward onto the surface tangent plane so the capsule
	// faces "outward" along the surface, never tilted off the normal.
	FVector Fwd = FVector::VectorPlaneProject(ArrivalAnchor->GetActorForwardVector(), PlanetUp).GetSafeNormal();
	if (Fwd.IsNearlyZero())
	{
		// Polar fallback: anchor's forward was parallel to the surface normal.
		Fwd = FVector::VectorPlaneProject(FVector::ForwardVector, PlanetUp).GetSafeNormal();
		if (Fwd.IsNearlyZero())
		{
			Fwd = FVector::VectorPlaneProject(FVector::RightVector, PlanetUp).GetSafeNormal();
		}
	}

	// Z = surface normal (exact), X = forward (orthogonalised against Z).
	return FRotationMatrix::MakeFromZX(PlanetUp, Fwd).ToQuat();
}

#undef LOCTEXT_NAMESPACE
