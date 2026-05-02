#include "PlanetSurfaceActor.h"

#include "PlanetActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

APlanetSurfaceActor::APlanetSurfaceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	// Mesh content is left empty; subclasses (BP_*) supply the mesh asset.
}

void APlanetSurfaceActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	AlignToSurface();
}

void APlanetSurfaceActor::BeginPlay()
{
	Super::BeginPlay();
	AlignToSurface();
}

void APlanetSurfaceActor::AlignToSurface()
{
	APlanetActor* P = ResolvePlanet();
	if (!P)
	{
		return;
	}

	const FVector Center  = P->GetCenter();
	const FVector ActorLoc = GetActorLocation();
	const FVector ToActor  = ActorLoc - Center;

	const FVector Up = ToActor.GetSafeNormal();
	if (Up.IsNearlyZero())
	{
		// Actor sits exactly at planet center; no defined surface direction.
		return;
	}

	// ── Position ───────────────────────────────────────────────────────────────
	FVector NewLoc = ActorLoc;
	if (bSnapToSurface)
	{
		const float TargetRadius = P->GetRadius() + SurfaceOffset;
		NewLoc = Center + Up * TargetRadius;
	}

	// ── Rotation ───────────────────────────────────────────────────────────────
	// Preserve the actor's current facing as much as possible by projecting its
	// existing forward onto the surface tangent plane.
	FVector Fwd = FVector::VectorPlaneProject(GetActorForwardVector(), Up).GetSafeNormal();
	if (Fwd.IsNearlyZero())
	{
		// Polar fallback: forward was parallel to Up. Use world X, then world Y.
		Fwd = FVector::VectorPlaneProject(FVector::ForwardVector, Up).GetSafeNormal();
		if (Fwd.IsNearlyZero())
		{
			Fwd = FVector::VectorPlaneProject(FVector::RightVector, Up).GetSafeNormal();
		}
	}

	if (!FMath::IsNearlyZero(SurfaceYaw))
	{
		const FQuat YawQuat(Up, FMath::DegreesToRadians(SurfaceYaw));
		Fwd = YawQuat.RotateVector(Fwd);
	}

	// MakeFromZX: Z = Up (exact), X = Fwd (orthogonalised against Z).
	const FQuat NewQuat = FRotationMatrix::MakeFromZX(Up, Fwd).ToQuat();

	SetActorLocationAndRotation(NewLoc, NewQuat);
}

APlanetActor* APlanetSurfaceActor::ResolvePlanet() const
{
	if (Planet)
	{
		return Planet;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APlanetActor* Nearest = nullptr;
	float MinDistSq = TNumericLimits<float>::Max();
	const FVector MyLoc = GetActorLocation();

	for (TActorIterator<APlanetActor> It(const_cast<UWorld*>(World)); It; ++It)
	{
		APlanetActor* Candidate = *It;
		if (!Candidate)
		{
			continue;
		}
		const float DistSq = FVector::DistSquared(MyLoc, Candidate->GetCenter());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			Nearest   = Candidate;
		}
	}

	return Nearest;
}
