#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetSurfaceActor.generated.h"

class APlanetActor;
class UStaticMeshComponent;

/**
 * Base actor for props that should sit on a spherical planet surface,
 * automatically oriented so their local +Z points along the surface normal.
 *
 * Subclass via Blueprint (BP_Tree_Surface, BP_Rock_Surface, ...) and assign
 * a static mesh whose pivot is at the base.  Drag into the level next to a
 * planet — the actor snaps and aligns on placement and on every move.
 */
UCLASS()
class PRINCE1_API APlanetSurfaceActor : public AActor
{
	GENERATED_BODY()

public:
	APlanetSurfaceActor();

	/** Re-runs the alignment logic. Exposed to the Details panel and Blueprints. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Planet Alignment")
	void AlignToSurface();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** Target planet. Leave null to auto-pick the nearest APlanetActor in the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Alignment")
	TObjectPtr<APlanetActor> Planet;

	/** If true, snap the actor location onto the planet surface (radius + offset). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Alignment")
	bool bSnapToSurface = true;

	/** Radial offset above the surface. Positive = floats above, negative = sunk in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Alignment",
		meta = (EditCondition = "bSnapToSurface"))
	float SurfaceOffset = 0.f;

	/** Extra yaw around the surface normal (degrees). Lets each instance vary its facing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Alignment")
	float SurfaceYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	/** Returns the explicit Planet if set, otherwise the nearest APlanetActor in the world. */
	APlanetActor* ResolvePlanet() const;
};
