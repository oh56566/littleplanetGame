#include "PlanetActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace PlanetActorPrivate
{
	// Engine's basic Sphere mesh has a radius of 50 cm at scale 1.
	constexpr float EngineSphereBaseRadius = 50.f;
}

APlanetActor::APlanetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(Radius);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAll"));

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		VisualMesh->SetStaticMesh(SphereMeshFinder.Object);
	}
}

void APlanetActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(Radius, /*bUpdateOverlaps=*/false);
	}
	if (VisualMesh)
	{
		const float MeshScale = Radius / PlanetActorPrivate::EngineSphereBaseRadius;
		VisualMesh->SetRelativeScale3D(FVector(MeshScale));
	}
}
