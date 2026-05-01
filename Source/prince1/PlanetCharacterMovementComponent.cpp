#include "PlanetCharacterMovementComponent.h"

#include "PlanetActor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

UPlanetCharacterMovementComponent::UPlanetCharacterMovementComponent()
{
	bAlwaysCheckFloor                 = true;
	bUseFlatBaseForFloorChecks        = false;
	bMaintainHorizontalGroundVelocity = true;
	SetWalkableFloorAngle(50.f);
}

void UPlanetCharacterMovementComponent::SetPlanet(APlanetActor* InPlanet)
{
	Planet = InPlanet;
}

void UPlanetCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	// ── Pre-physics ────────────────────────────────────────────────────────────
	// Set gravity direction so that the base class floor detection, jumping,
	// and braking all use the planet-relative "down". Do NOT rotate the capsule
	// here; moving the capsule before physics can shift it into / away from the
	// floor mid-tick, causing spurious Falling mode and killing ground friction.
	if (Planet.IsValid() && UpdatedComponent)
	{
		const FVector PreUp = (UpdatedComponent->GetComponentLocation() - Planet->GetCenter()).GetSafeNormal();
		if (!PreUp.IsNearlyZero())
		{
			SetGravityDirection(-PreUp);
		}
	}

	// ── Standard CMC physics ───────────────────────────────────────────────────
	// Floor detection, velocity integration, braking, and mode transitions
	// (Walking ↔ Falling) all happen here, with the gravity direction we set above.
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ── Post-physics capsule alignment ─────────────────────────────────────────
	// Rotate AFTER physics so the capsule's position/floor-contact is not
	// disturbed during the physics step. The rotation is purely cosmetic this
	// frame; next frame's physics starts from the corrected orientation.
	if (Planet.IsValid() && UpdatedComponent)
	{
		// Recompute Up from the position after movement (character has moved).
		const FVector PostUp = (UpdatedComponent->GetComponentLocation() - Planet->GetCenter()).GetSafeNormal();
		if (PostUp.IsNearlyZero())
		{
			return;
		}

		// Update gravity for any system that reads it between ticks.
		SetGravityDirection(-PostUp);

		// Desired yaw: face velocity direction when moving, keep current when idle.
		const FVector FlatVelocity = FVector::VectorPlaneProject(Velocity, PostUp);
		const bool bMoving         = FlatVelocity.SizeSquared() > 25.f; // > 5 cm/s

		const FVector DesiredForward = bMoving
			? FlatVelocity.GetSafeNormal()
			: FVector::VectorPlaneProject(UpdatedComponent->GetForwardVector(), PostUp).GetSafeNormal();

		if (!DesiredForward.IsNearlyZero())
		{
			// Build a single target quaternion: Z = planet Up, X = desired forward.
			// MakeFromZX keeps Z exact and orthogonalises X, avoiding gimbal issues.
			const FQuat TargetQuat  = FRotationMatrix::MakeFromZX(PostUp, DesiredForward).ToQuat();
			const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();

			// Smooth turn while moving, instant Z-correction while idle.
			const float Alpha = bMoving ? FMath::Clamp(DeltaTime * 10.f, 0.f, 1.f) : 1.f;
			UpdatedComponent->SetWorldRotation(FQuat::Slerp(CurrentQuat, TargetQuat, Alpha).GetNormalized(),
				/*bSweep=*/false);
		}
	}
}

bool UPlanetCharacterMovementComponent::IsWalkable(const FHitResult& Hit) const
{
	if (!Hit.IsValidBlockingHit())
	{
		return false;
	}

	if (!IsPlanetHit(Hit))
	{
		return Super::IsWalkable(Hit);
	}

	FVector PlanetUp;
	if (!GetPlanetUpAtLocation(Hit.ImpactPoint, PlanetUp))
	{
		PlanetUp = -GetGravityDirection();
	}

	const float SurfaceDot = FVector::DotProduct(Hit.ImpactNormal, PlanetUp);
	return SurfaceDot >= GetWalkableFloorZ();
}

bool UPlanetCharacterMovementComponent::IsValidLandingSpot(const FVector& CapsuleLocation,
	const FHitResult& Hit) const
{
	if (Super::IsValidLandingSpot(CapsuleLocation, Hit))
	{
		return true;
	}

	if (!IsPlanetHit(Hit))
	{
		return false;
	}

	FFindFloorResult PlanetFloor;
	return FindPlanetFloor(CapsuleLocation, PlanetFloor) && PlanetFloor.IsWalkableFloor();
}

bool UPlanetCharacterMovementComponent::ShouldCheckForValidLandingSpot(float DeltaTime,
	const FVector& Delta, const FHitResult& Hit) const
{
	if (Super::ShouldCheckForValidLandingSpot(DeltaTime, Delta, Hit))
	{
		return true;
	}

	return IsPlanetHit(Hit);
}

void UPlanetCharacterMovementComponent::FindFloor(const FVector& CapsuleLocation,
	FFindFloorResult& OutFloorResult, bool bCanUseCachedLocation,
	const FHitResult* DownwardSweepResult) const
{
	Super::FindFloor(CapsuleLocation, OutFloorResult, bCanUseCachedLocation, DownwardSweepResult);

	if (OutFloorResult.IsWalkableFloor())
	{
		return;
	}

	FFindFloorResult PlanetFloor;
	if (FindPlanetFloor(CapsuleLocation, PlanetFloor))
	{
		OutFloorResult = PlanetFloor;
	}
}

void UPlanetCharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float RemainingTime,
	int32 Iterations)
{
	ProjectVelocityToPlanetSurface();

	// Let the base class handle mode transitions and its own velocity projection.
	Super::ProcessLanded(Hit, RemainingTime, Iterations);

	// Safety pass: ensure velocity is truly parallel to the planet surface.
	// The base class uses GetGravityDirection() for this in UE 5.4+, but on a
	// curved surface a small mismatch can leave a residual into-floor component
	// that prevents proper ground friction from engaging.
	ProjectVelocityToPlanetSurface();
}

bool UPlanetCharacterMovementComponent::GetPlanetUpAtLocation(const FVector& Location,
	FVector& OutUp) const
{
	if (!Planet.IsValid())
	{
		return false;
	}

	OutUp = (Location - Planet->GetCenter()).GetSafeNormal();
	return !OutUp.IsNearlyZero();
}

bool UPlanetCharacterMovementComponent::IsPlanetHit(const FHitResult& Hit) const
{
	return Planet.IsValid() && Hit.IsValidBlockingHit() && Hit.GetActor() == Planet.Get();
}

bool UPlanetCharacterMovementComponent::FindPlanetFloor(const FVector& CapsuleLocation,
	FFindFloorResult& OutFloorResult) const
{
	OutFloorResult.Clear();

	if (!HasValidData() || !UpdatedComponent || !CharacterOwner || !Planet.IsValid())
	{
		return false;
	}

	FVector PlanetUp;
	if (!GetPlanetUpAtLocation(CapsuleLocation, PlanetUp))
	{
		return false;
	}

	float PawnRadius = 0.f;
	float PawnHalfHeight = 0.f;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlanetFindFloor), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(QueryParams, ResponseParam);

	const float TraceDist = MaxStepHeight + MAX_FLOOR_DIST + 5.f;
	const FVector TraceStart = CapsuleLocation;
	const FVector TraceEnd = CapsuleLocation - PlanetUp * TraceDist;

	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(PawnRadius, PawnHalfHeight);
	FHitResult Hit(1.f);
	const bool bBlockingHit = FloorSweepTest(Hit, TraceStart, TraceEnd,
		UpdatedComponent->GetCollisionObjectType(), CapsuleShape, QueryParams, ResponseParam);

	if (!bBlockingHit || !IsPlanetHit(Hit) || !IsWalkable(Hit))
	{
		return false;
	}

	const float FloorDist = FVector::DotProduct(CapsuleLocation - Hit.Location, PlanetUp);
	const float MaxAcceptedFloorDist = MaxStepHeight + MAX_FLOOR_DIST;

	if (FloorDist > MaxAcceptedFloorDist)
	{
		return false;
	}

	OutFloorResult.SetFromSweep(Hit, FloorDist, true);
	return true;
}

void UPlanetCharacterMovementComponent::ProjectVelocityToPlanetSurface()
{
	if (Planet.IsValid() && CharacterOwner)
	{
		const FVector Up = (CharacterOwner->GetActorLocation() - Planet->GetCenter()).GetSafeNormal();
		if (!Up.IsNearlyZero())
		{
			// Remove any velocity component pointing into or out of the floor.
			Velocity = FVector::VectorPlaneProject(Velocity, Up);
		}
	}
}
